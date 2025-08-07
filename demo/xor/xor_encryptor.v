// 一个简单的 32 位 XOR 对称加密时序电路模块

module xor_encryptor (
    input clk,          // 时钟信号, input默认就是wire，省略了 wire 关键字
    input rst_n,        // 同步低电平复位信号 (通常用 rst_n 表示低电平有效)

    input start,        // 启动加密操作的信号 (脉冲信号)
    input [31:0] data_in, // 32 位明文数据输入
    input [31:0] key_in,   // 32 位密钥输入

    output reg [31:0] data_out, // 32 位密文数据输出
    output reg done           // 加密完成指示信号 (脉冲信号)
);

    // 内部寄存器，用于锁存输入数据和密钥，以及存储中间结果
    reg [31:0] current_data;
    reg [31:0] current_key;

    // 状态寄存器 (虽然这里逻辑简单，不需要复杂状态机，但这是时序电路的常见组成部分)
    // 0: 空闲 (IDLE)
    // 1: 加密中 (ENCRYPTING)
    // 2: 完成 (DONE)
    reg [1:0] state;
    localparam IDLE       = 2'b00;  // 局部常量，编译期值确定
    localparam ENCRYPTING = 2'b01;
    localparam DONE       = 2'b10;

    // 时序逻辑：在时钟上升沿或复位下降沿触发
    // 同时判断clk和rst_n的状态，为了异步复位
    // 注： 只使用时钟（同步复位）必须等待下一个时钟上升沿才能复位, 如果时钟出现故障，电路将无法复位
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin // 异步低电平复位
            current_data <= 32'h0;
            current_key  <= 32'h0;
            data_out     <= 32'h0;
            done         <= 1'b0;
            state        <= IDLE; // 复位到空闲状态
        end else begin
            // 状态机逻辑
            case (state)
                IDLE: begin
                    done <= 1'b0; // 在 IDLE 状态，done 信号为低
                    if (start) begin // 如果接收到启动信号, 信号为1就是true
                        current_data <= data_in; // 锁存输入数据
                        current_key  <= key_in;   // 锁存输入密钥
                        state <= ENCRYPTING;      // 进入加密状态
                    end
                end

                ENCRYPTING: begin
                    // 执行 XOR 加密操作，并将结果赋给输出寄存器
                    // 注意：这里 data_out 是在 ENCRYPTING 状态的下一个时钟周期输出结果
                    data_out <= current_data ^ current_key;
                    done     <= 1'b1; // 加密完成，拉高 done 信号
                    state    <= DONE;   // 进入完成状态
                end

                DONE: begin
                    done  <= 1'b0; // 仅在 ENCRYPTING 状态的下一个周期拉高 done，然后立即拉低
                    state <= IDLE; // 返回空闲状态，等待下一次启动
                end

                default: begin // 捕获未定义的状态，通常复位到 IDLE
                    state <= IDLE;
                    done  <= 1'b0;
                end
            endcase
        end
    end

endmodule