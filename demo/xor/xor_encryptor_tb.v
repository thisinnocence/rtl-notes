// testbench
// 波形： gtkwave waveform.vcd

`timescale 1ns / 1ps

module xor_encryptor_tb;

    // 声明测试激励的信号
    // 时钟信号 - 测试台输入
    // 定义为 reg 因为它由 initial/always 块驱动
    reg  clk;  
    reg  rst_n;
    reg  start;
    reg  [31:0] data_in;
    reg  [31:0] key_in;
    wire [31:0] data_out;
    wire done;

    // 实例化 DUT
    xor_encryptor dut (
        .clk(clk),  // 命名端口连接: 小括号里表示入参是外部的, .clk是module的端口
        .rst_n(rst_n),
        .start(start),
        .data_in(data_in),
        .key_in(key_in),
        .data_out(data_out),
        .done(done)
    );

    // --- 在这里添加波形生成代码 ---
    initial begin
        // 1. 设置波形文件的名称和格式
        //    'waveform.vcd' 是输出文件名，VCD 是常见的波形文件格式
        $dumpfile("waveform.vcd"); 
        
        // 2. 指定要转储的信号
        //    '0' 表示转储所有信号
        //    'xor_encryptor_tb' 表示从这个模块开始递归转储
        $dumpvars(0, xor_encryptor_tb); 
        
        $display("Dumping waveform to waveform.vcd...");
    end
    // --- 波形生成代码结束 ---


    // 时钟生成
    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end

    // 测试序列
    initial begin
        // 初始化信号
        rst_n = 1'b0;
        start = 1'b0;
        data_in = 32'hdeadbeef;
        key_in = 32'h12345678;

        // 1. 复位
        #10 rst_n = 1'b1;
        $display("----------------------------------------");
        $display("Test started.");

        // 2. 启动加密操作 (1个时钟周期)
        @(posedge clk);  // 等待时钟上升沿, 确保时序正确
        start = 1'b1;
        $display("Time=%0t, Starting encryption with:", $time);
        $display("Data_in  = 0x%h", data_in);
        $display("Key_in   = 0x%h", key_in);
        @(posedge clk);
        start = 1'b0;

        // 3. 等待状态机从 ENCRYPTING 到 DONE
        @(posedge clk);
        
        // 4. 验证结果
        if (done) begin
            $display("Time=%0t, Encryption finished.", $time);
            $display("Expected output = 0x%h", data_in ^ key_in);
            $display("Actual output   = 0x%h", data_out);
            
            if (data_out == (data_in ^ key_in)) begin
                $display("----------------------------------------");
                $display("Test PASSED!");
                $display("----------------------------------------");
            end else begin
                $display("----------------------------------------");
                $display("Test FAILED: Incorrect output!");
                $display("----------------------------------------");
            end
        end else begin
            $display("Test FAILED: Done signal not received.");
        end

        // 5. 等待状态机从 DONE 回到 IDLE
        @(posedge clk);
        
        // 6. 再次运行
        data_in = 32'h01234567;
        key_in = 32'habcdef01;
        start = 1'b1;
        $display("----------------------------------------");
        $display("Test another case:");
        $display("Time=%0t, Starting encryption with:", $time);
        $display("Data_in  = 0x%h", data_in);
        $display("Key_in   = 0x%h", key_in);
        @(posedge clk);
        start = 1'b0;

        @(posedge clk);

        if (done && (data_out == (data_in ^ key_in))) begin
            $display("Time=%0t, Encryption finished.", $time);
            $display("Expected output = 0x%h", data_in ^ key_in);
            $display("Actual output   = 0x%h", data_out);
            $display("Test PASSED!");
        end else begin
            $display("Test FAILED!");
        end

        // 7. 结束仿真
        #10 $finish;
    end

endmodule