#include <systemc.h>
#include "xor_encryptor.cc"

SC_MODULE(xor_encryptor_tb) {
    // 时钟和信号声明
    sc_clock clk;
    sc_signal<bool> rst_n, start, done;
    sc_signal<sc_uint<32>> data_in, key_in, data_out;

    // DUT实例
    xor_encryptor* dut;

    SC_CTOR(xor_encryptor_tb) : clk("clk", 10, SC_NS) {
        // 创建DUT实例
        dut = new xor_encryptor("dut");
        
        // 连接信号
        dut->clk(clk);
        dut->rst_n(rst_n);
        dut->start(start);
        dut->data_in(data_in);
        dut->key_in(key_in);
        dut->data_out(data_out);
        dut->done(done);

        // 注册测试进程
        SC_THREAD(test_process);
    }

    void test_process() {
        // 初始化信号
        rst_n.write(false);
        start.write(false);
        data_in.write(0xdeadbeef);
        key_in.write(0x12345678);

        // 1. 复位并等待几个时钟周期确保稳定
        wait(20, SC_NS);
        rst_n.write(true);
        wait(clk.posedge_event());
        cout << "----------------------------------------" << endl;
        cout << "Test started." << endl;

        // 2. 启动第一次加密操作
        wait(clk.posedge_event());
        start.write(true);
        cout << "Time=" << sc_time_stamp() << ", Starting encryption with:" << endl;
        cout << "Data_in  = 0x" << hex << data_in.read() << endl;
        cout << "Key_in   = 0x" << hex << key_in.read() << endl;
        
        wait(clk.posedge_event());
        start.write(false); // 禁用启动信号

        // 3. 等待加密完成 - 确保捕获done信号
        wait(clk.posedge_event());
        wait(clk.posedge_event());
        
        // 4. 验证结果 - 在时钟边沿后立即检查
        if (done.read()) {
            cout << "Time=" << sc_time_stamp() << ", Encryption finished." << endl;
            sc_uint<32> expected = data_in.read() ^ key_in.read();
            cout << "Expected output = 0x" << hex << expected << endl;
            cout << "Actual output   = 0x" << hex << data_out.read() << endl;
            
            if (data_out.read() == expected) {
                cout << "----------------------------------------" << endl;
                cout << "Test PASSED!" << endl;
                cout << "----------------------------------------" << endl;
            } else {
                cout << "----------------------------------------" << endl;
                cout << "Test FAILED: Incorrect output!" << endl;
                cout << "----------------------------------------" << endl;
            }
        } else {
            cout << "Test FAILED: Done signal not received." << endl;
        }

        wait(clk.posedge_event());

        // 5. 第二次测试 - 确保前一个操作完全结束
        wait(clk.posedge_event());
        data_in.write(0x01234567);
        key_in.write(0xabcdef01);
        start.write(true);
        cout << "----------------------------------------" << endl;
        cout << "Test another case:" << endl;
        cout << "Time=" << sc_time_stamp() << ", Starting encryption with:" << endl;
        cout << "Data_in  = 0x" << hex << data_in.read() << endl;
        cout << "Key_in   = 0x" << hex << key_in.read() << endl;
        
        wait(clk.posedge_event());
        start.write(false);

        // 6. 等待第二次加密完成
        wait(clk.posedge_event());
        wait(clk.posedge_event());
        
        if (done.read()) {
            cout << "Time=" << sc_time_stamp() << ", Encryption finished." << endl;
            cout << "Expected output = 0x" << hex << (data_in.read() ^ key_in.read()) << endl;
            cout << "Actual output   = 0x" << hex << data_out.read() << endl;
            if (data_out.read() == (data_in.read() ^ key_in.read())) {
                cout << "Test PASSED!" << endl;
            } else {
                cout << "Test FAILED: Incorrect output!" << endl;
            }
        } else {
            cout << "Test FAILED: Done signal not received!" << endl;
        }

        // 7. 结束仿真
        wait(10, SC_NS);
        sc_stop();
    }

    ~xor_encryptor_tb() {
        delete dut;
    }
};

int sc_main(int argc, char* argv[]) {
    xor_encryptor_tb tb("testbench");
    sc_start();
    return 0;
}
