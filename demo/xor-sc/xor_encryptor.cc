#include <systemc.h>

SC_MODULE(xor_encryptor) {
    // 端口声明
    sc_in_clk clk;
    sc_in<bool> rst_n;
    sc_in<bool> start;
    sc_in<sc_uint<32>> data_in;
    sc_in<sc_uint<32>> key_in;
    
    sc_out<sc_uint<32>> data_out;
    sc_out<bool> done;

    // 内部状态和寄存器
    sc_uint<32> current_data;
    sc_uint<32> current_key;
    
    // 状态定义
    enum State {IDLE = 0, ENCRYPTING = 1, DONE = 2};
    State state;

    // 构造函数
    SC_CTOR(xor_encryptor) {
        // 注册时钟敏感进程
        SC_METHOD(encrypt_proc);
        sensitive << clk.pos() << rst_n.neg();
        
        // 初始化状态
        state = IDLE;
    }

    // 加密处理过程
    void encrypt_proc() {
        // 异步复位
        if (!rst_n.read()) {
            current_data = 0;
            current_key = 0;
            data_out.write(0);
            done.write(false);
            state = IDLE;
        } else {
            // 时钟上升沿处理
            switch (state) {
                case IDLE:
                    done.write(false);
                    if (start.read()) {
                        current_data = data_in.read();
                        current_key = key_in.read();
                        state = ENCRYPTING;
                    }
                    break;

                case ENCRYPTING:
                    // 执行XOR加密
                    data_out.write(current_data ^ current_key);
                    done.write(true);
                    state = DONE;
                    break;

                case DONE:
                    done.write(false);
                    state = IDLE;
                    break;

                default:
                    state = IDLE;
                    done.write(false);
                    break;
            }
        }
    }
};