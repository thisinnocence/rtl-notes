# 一个简易APB实现

这个项目由三个主要部分构成：**APB Master**、**APB Slave** 和连接它们的 **Interconnect**。我们将在此基础上，添加一个 Testbench，来模拟时钟、复位和运行场景，最终验证加法器的功能。

-----

## APB协议简介

APB（Advanced Peripheral Bus）是 ARM 体系结构中用于连接低速外设的总线协议。它的设计目标是简单、低功耗和易于实现。APB 协议通常用于连接诸如 UART、GPIO、定时器等外设。

ARM APB spec: <https://developer.arm.com/documentation/ihi0024/latest/>

### 传输的时序

APB 协议的精髓在于其简化的传输时序。一次完整的传输通常只需要两个时钟周期，分为两个阶段：

1. **SETUP 阶段**：主设备（Master）首先拉高 **`PSEL`** 信号来选中一个从设备（Slave），同时在总线上设置好地址（`PADDR`）、数据（`PWDATA`）和读写方向（`PWRITE`）。在这个阶段，**`PENABLE`** 信号保持为低电平。

2. **ACCESS 阶段**：紧接着在下一个时钟周期，主设备将 **`PENABLE`** 信号拉高，标志着当前周期是 **“数据有效”** 的传输周期。从设备在这一周期内完成数据的读取或写入，并在时序结束时给出响应。

这种两阶段时序极大地简化了控制逻辑，使得 Slave 模块的实现变得非常直接。同时，APB 协议还支持**可编程等待周期**（通过 `PREADY` 信号），允许慢速从设备有足够的时间来处理数据，从而确保了总线传输的可靠性。

### 传输状态机

好的！以下是 APB 接口操作状态的简洁总结（表格 + 核心逻辑）：

APB 状态机核心总结

| 状态    | 触发条件/转移条件               | 关键信号状态 (`PSELx`/`PENABLE`) | 核心行为描述                                                                 |
|---------|--------------------------------|----------------------------------|-----------------------------------------------------------------------------|
| **IDLE** | 无传输请求                     | `0` / `0`                        | 默认状态，等待传输触发；若需传输 → 进入 `SETUP`。                           |
| **SETUP**| 进入传输（`IDLE → SETUP`）     | `1` / `0`                        | 仅维持 **1 个时钟周期**，下一个时钟沿自动跳转至 `ACCESS`。                  |
| **ACCESS** | 开始数据传输（`SETUP → ACCESS`）<br>或 `PREADY=0` 保持 | `1` / `1`                        | - **信号稳定期**：地址/控制/数据信号必须冻结<br>- 退出条件由 `PREADY` 控制：<br>　▪ `0` → 继续停留 `ACCESS`（插入等待）<br>　▪ `1` → 退出 `ACCESS`：<br>　　　→ 无后续传输 → 回到 `IDLE`<br>　　　→ 有后续传输 → 直接进入 `SETUP` |

1. 状态转移路径：  
    - IDLE → SETUP → ACCESS → (PREADY=1 ? IDLE : SETUP)  
    - SETUP 是短暂的过渡状态，仅用于锁存地址和控制信号。  
    - ACCESS 是实际数据传输阶段，可因外设未准备好（PREADY=0）而延长。

2. 信号稳定性要求：  
    - 在 SETUP → ACCESS 转换及 ACCESS 状态内的连续周期 中，以下信号必须固定：  
    - PADDR, PWRITE, PWDATA, PSTRB, PPROT（写操作时）。  

3. 流控机制：  
    - 外设通过 PREADY 控制传输节奏：  
    - PREADY=0：插入等待周期（ACCESS 状态持续）。  
    - PREADY=1：完成当前传输，退出 ACCESS。

APB 通过 `IDLE（空闲）→ SETUP（选通）→ ACCESS（传输）` 三状态循环完成数据交互，PREADY 信号允许外设灵活控制传输时序，确保低功耗与兼容性。

-----

## Step 1: 实现 APB Slave 模块

这是一个简单的加法器，它包含两个寄存器 `opA` 和 `opB`，以及一个 `result` 寄存器。Master 通过 APB 协议向 `opA` 和 `opB` 写入数据，然后可以从 `result` 寄存器中读出结果。

```verilog
// apb_slave_adder.v
module apb_slave_adder (
    input  wire         pclk,
    input  wire         presetn,
    input  wire         psel,
    input  wire         penable,
    input  wire         pwrite,
    input  wire [31:0]  paddr,
    input  wire [31:0]  pwdata,
    output reg  [31:0]  prdata,
    output reg          pready,
    output reg          pslverr
);

    reg [7:0] opA;
    reg [7:0] opB;
    reg [7:0] result;

    localparam ADDR_OP_A   = 32'h00;
    localparam ADDR_OP_B   = 32'h04;
    localparam ADDR_RESULT = 32'h08;

    always @(posedge pclk or negedge presetn) begin
        if (!presetn) begin
            opA    <= 8'h0;
            opB    <= 8'h0;
            result <= 8'h0;
            prdata <= 32'h0;
            pready <= 1'b0;
            pslverr <= 1'b0;
        end else begin
            pready <= 1'b0;
            pslverr <= 1'b0;
            if (psel && penable) begin
                pready <= 1'b1;
                if (pwrite) begin
                    if (paddr[7:0] == ADDR_OP_A) begin
                        opA <= pwdata[7:0];
                    end else if (paddr[7:0] == ADDR_OP_B) begin
                        opB <= pwdata[7:0];
                        result <= opA + pwdata[7:0];
                    end
                end else begin // 读操作
                    if (paddr[7:0] == ADDR_RESULT) begin
                        prdata <= {24'h0, result};
                    end else begin
                        pslverr <= 1'b1;
                    end
                end
            end
        end
    end
endmodule
```

-----

## Step 2: 实现 APB Master 模块

这个 Master 模块包含一个简单的**状态机**，它负责生成 APB 协议的时序。我们还用**Verilog `task`** 封装了读写操作，这能让你的 Testbench 代码更简洁。

```verilog
// apb_master_controller.v
module apb_master_controller (
    input  wire         pclk,
    input  wire         presetn,
    output reg          psel,
    output reg          penable,
    output reg          pwrite,
    output reg  [31:0]  paddr,
    output reg  [31:0]  pwdata,
    input  wire [31:0]  prdata,
    input  wire         pready,
    input  wire         pslverr
);

    reg [2:0] state;
    reg [31:0] addr_reg;
    reg [31:0] data_reg;
    
    localparam IDLE    = 3'd0;
    localparam SETUP   = 3'd1;
    localparam ACCESS  = 3'd2;

    // 封装 APB 写操作，使得Testbench代码更简洁
    task apb_write;
        input [31:0] addr;
        input [31:0] data;
    begin
        @(posedge pclk);
        psel = 1'b1;
        pwrite = 1'b1;
        paddr = addr;
        pwdata = data;

        @(posedge pclk);
        penable = 1'b1;

        @(posedge pclk);
        penable = 1'b0;
        psel = 1'b0;
        pwrite = 1'b0;
    end
    endtask

    // 封装 APB 读操作
    task apb_read;
        input [31:0] addr;
    begin
        @(posedge pclk);
        psel = 1'b1;
        pwrite = 1'b0;
        paddr = addr;

        @(posedge pclk);
        penable = 1'b1;

        @(posedge pclk);
        penable = 1'b0;
        psel = 1'b0;
    end
    endtask

    always @(posedge pclk or negedge presetn) begin
        if (!presetn) begin
            psel <= 1'b0;
            penable <= 1'b0;
            pwrite <= 1'b0;
            paddr <= 32'h0;
            pwdata <= 32'h0;
        end
    end
endmodule
```

**注意**：这里的 Master 模块只包含时序逻辑，具体的读写操作将由 Testbench 调用其内部的 `task` 来完成。

-----

## Step 3: 实现 APB Interconnect

`Interconnect` 模块负责将 Master 和 Slave 连接起来，并包含地址译码和数据多路复用逻辑。

```verilog
// apb_top_bus.v
module apb_top_bus (
    input wire pclk,
    input wire presetn
);

    // Master 控制的 APB 总线信号
    wire psel;
    wire penable;
    wire pwrite;
    wire [31:0] paddr;
    wire [31:0] pwdata;
    
    // 从设备返回的 APB 总线信号
    wire [31:0] prdata_bus;
    wire pready_bus;
    wire pslverr_bus;

    // --- 地址译码器 ---
    // 假设 Slave Adder 的地址范围是 0x00001000 - 0x00001FFF
    wire psel_adder = psel & (paddr[31:12] == 20'h00001);

    // --- 实例化 Master 和 Slave ---
    apb_master_controller u_master (
        .pclk(pclk),
        .presetn(presetn),
        .psel(psel),
        .penable(penable),
        .pwrite(pwrite),
        .paddr(paddr),
        .pwdata(pwdata),
        .prdata(prdata_bus),
        .pready(pready_bus),
        .pslverr(pslverr_bus)
    );

    apb_slave_adder u_slave_adder (
        .pclk(pclk),
        .presetn(presetn),
        .psel(psel_adder), // 连接专属的 psel
        .penable(penable),
        .pwrite(pwrite),
        .paddr(paddr),
        .pwdata(pwdata),
        .prdata(prdata_bus), // 因为只有一个 Slave，直接连接
        .pready(pready_bus),
        .pslverr(pslverr_bus)
    );

endmodule
```

-----

## Step 4: 实现 Testbench

Testbench 是整个验证过程的关键。它会实例化 `apb_top_bus` 模块，生成时钟和复位，然后调用 Master 模块中的 `task` 来发送读写激励，最后检查返回的数据是否正确。

```verilog
// tb_apb.v
`timescale 1ns / 1ps

module tb_apb;
    // --- Testbench 信号 ---
    reg  pclk;
    reg  presetn;
    wire [31:0] prdata_bus; // 从 DUT 读取数据
    
    // --- 实例化 DUT (被测模块) ---
    apb_top_bus u_dut (
        .pclk(pclk),
        .presetn(presetn)
    );
    
    // 连接到 DUT 的 Master 端口，用于调用 task
    apb_master_controller u_master_inst (
        .pclk(pclk),
        .presetn(presetn),
        .psel(), // Master 的输出，Testbench 不需要关心
        .penable(),
        .pwrite(),
        .paddr(),
        .pwdata(),
        .prdata(u_dut.prdata_bus),
        .pready(u_dut.pready_bus),
        .pslverr(u_dut.pslverr_bus)
    );
    
    // --- 时钟生成 ---
    initial begin
        pclk = 0;
        forever #5 pclk = ~pclk; // 100MHz 时钟
    end
    
    // --- 激励序列 ---
    initial begin
        $monitor("Time=%0t | psel=%b, penable=%b, pwrite=%b, paddr=0x%h, pwdata=0x%h, prdata=0x%h",
                  $time, u_master_inst.psel, u_master_inst.penable, u_master_inst.pwrite,
                  u_master_inst.paddr, u_master_inst.pwdata, prdata_bus);

        // --- 1. 复位 ---
        presetn = 1'b0;
        #20;
        presetn = 1'b1;
        #10;
        
        // --- 2. 写入 opA = 10 ---
        u_master_inst.apb_write(32'h00001000, 32'd10);
        
        // --- 3. 写入 opB = 20 (加法器将计算 10+20) ---
        u_master_inst.apb_write(32'h00001004, 32'd20);
        
        // --- 4. 读取结果 ---
        u_master_inst.apb_read(32'h00001008);
        
        // --- 5. 检查结果 ---
        @(posedge pclk);
        if (u_dut.prdata_bus == 32'd30) begin
            $display("TEST PASSED: 10 + 20 = 30");
        end else begin
            $display("TEST FAILED: Expected 30, got %d", u_dut.prdata_bus);
        end
        
        #10;
        $finish;
    end
endmodule
```

**Testbench 实现的关键点：**

- **时钟生成**：使用 `initial` 块和 `forever` 循环来产生一个稳定的时钟信号。
- **复位**：在 `initial` 块的开头，先拉低复位信号一段时间，然后拉高，确保 DUT 正确复位。
- **调用任务**：Testbench 通过实例化 Master 模块并直接调用其 `apb_write` 和 `apb_read` 任务来发送激励，这比手动 `poke` 信号要高效得多。
- **验证**：在读操作完成后，Testbench 会通过 `if` 语句检查 `prdata_bus` 的值是否符合预期，并打印出测试结果。
