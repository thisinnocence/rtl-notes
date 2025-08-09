#include <iostream>
#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace tlm_utils;

// 行为级：是事件驱动和时钟驱动的，每个时钟沿都可能触发状态机的变化。
// 事务级：是函数调用驱动的，它把整个操作过程看作一个原子性的、不可分割的事件，在宏观层面上进行时间建模。

// 定义一个 XOR 加密事务类
// 这个类包含了加密所需的所有信息：输入数据、密钥和输出结果
class xor_transaction : public tlm_generic_payload {
public:
    sc_uint<32> data;
    sc_uint<32> key;
    sc_uint<32> result;

    xor_transaction() : tlm_generic_payload() {}
};

// 目标模块 (Target): 执行实际的 XOR 加密操作
// 相当于xor-sc行为级里的的 xor_encryptor DUT
SC_MODULE(xor_encryptor_target) {
    // 声明一个 TLM 目标 socket
    simple_target_socket<xor_encryptor_target> socket;

    SC_CTOR(xor_encryptor_target) : socket("socket") {
        // 将 b_transport 方法注册到 socket，用于处理阻塞式传输
        //  描述一个同步或串行的通信过程。调用者发送请求后必须等待结果，才能进行下一步操作。
        //  而xor IP对应的硬件行为级，如果调用对接这个模块，也有对应同步通信协议，输入数据，开始，完成等。
        socket.register_b_transport(this, &xor_encryptor_target::b_transport);
    }

private:
    // b_transport 是 TLM 2.0 的核心方法，用于接收事务
    //   这个函数调用一次性替代了行为级中的 start 信号、clk 时钟、data_in/key_in 传输以及 done 信号
    //   大大简化了 *行为级* 仿真中每个具体的时钟驱动，复位，状态信号等；
    //   内部会推进一个 *事务完成需要的所有时钟周期*，并且在时期内完成完整的事务处理
    void b_transport(tlm_generic_payload& trans, sc_time& delay) {
        // 将通用的 tlm_generic_payload 转换成我们自定义的 xor_transaction
        xor_transaction* my_trans = dynamic_cast<xor_transaction*>(&trans);
        
        if (my_trans) {
            // 模拟加密操作的时延，这里我们等待 10 ns
            // 这个时间是完整的事务所需要的时钟周期，可以推算出来
            delay += sc_time(10, SC_NS); 
            wait(delay); // 模拟等待时间
            
            // 执行 XOR 加密
            my_trans->result = my_trans->data ^ my_trans->key;

            cout << sc_time_stamp() << ": Target received data=0x" << hex << my_trans->data 
                 << ", key=0x" << my_trans->key << ", computed result=0x" << my_trans->result << endl;
        }

        // 设置响应状态，表示操作成功完成
        trans.set_response_status(TLM_OK_RESPONSE);
    }
};

// 发起者模块 (Initiator): 发起加密请求
// 它相当于你原有的 xor_encryptor_tb
SC_MODULE(xor_encryptor_initiator) {
    // 声明一个 TLM 发起者 socket
    simple_initiator_socket<xor_encryptor_initiator> socket;
    
    // 我们仍然使用一个时钟来驱动测试线程，但它不用于 DUT 的逻辑
    sc_in_clk clk;

    SC_CTOR(xor_encryptor_initiator) : socket("socket") {
        // 注册一个线程，用于执行测试逻辑
        SC_THREAD(test_process);
        sensitive << clk.pos();
    }

    void test_process() {
        cout << "----------------------------------------" << endl;
        cout << "TLM Testbench started." << endl;
        wait(1, SC_NS); // 等待仿真启动

        // --- 第一个测试用例 ---
        sc_uint<32> data1 = 0xdeadbeef;
        sc_uint<32> key1 = 0x12345678;
        sc_uint<32> expected1 = data1 ^ key1;

        // 创建第一个事务
        xor_transaction trans1;
        trans1.data = data1;
        trans1.key = key1;
        sc_time delay1 = SC_ZERO_TIME;

        cout << sc_time_stamp() << ": Initiator sending first transaction..." << endl;
        // 通过 socket 发送事务，这是一个阻塞式调用，会等待目标模块处理完毕
        socket->b_transport(trans1, delay1);
        
        cout << sc_time_stamp() << ": Initiator received first result." << endl;
        cout << "Expected: 0x" << hex << expected1 << ", Actual: 0x" << trans1.result << endl;
        if (trans1.result == expected1) {
            cout << "First test PASSED!" << endl;
        } else {
            cout << "First test FAILED: Incorrect output!" << endl;
        }
        cout << "----------------------------------------" << endl;

        // --- 第二个测试用例 ---
        wait(10, SC_NS); // 等待一段时间再发送第二个事务
        sc_uint<32> data2 = 0x01234567;
        sc_uint<32> key2 = 0xabcdef01;
        sc_uint<32> expected2 = data2 ^ key2;

        // 创建第二个事务
        xor_transaction trans2;
        trans2.data = data2;
        trans2.key = key2;
        sc_time delay2 = SC_ZERO_TIME;

        cout << sc_time_stamp() << ": Initiator sending second transaction..." << endl;
        socket->b_transport(trans2, delay2);

        cout << sc_time_stamp() << ": Initiator received second result." << endl;
        cout << "Expected: 0x" << hex << expected2 << ", Actual: 0x" << trans2.result << endl;
        if (trans2.result == expected2) {
            cout << "Second test PASSED!" << endl;
        } else {
            cout << "Second test FAILED: Incorrect output!" << endl;
        }
        cout << "----------------------------------------" << endl;
        
        // 结束仿真
        wait(1, SC_NS);
        sc_stop();
    }
};

// 主函数
int sc_main(int argc, char* argv[]) {
    // 实例化模块
    xor_encryptor_initiator initiator("initiator");
    xor_encryptor_target target("target");
    sc_clock clk("clk", 10, SC_NS);

    // 连接 socket
    initiator.socket.bind(target.socket);
    initiator.clk(clk);

    // 运行仿真
    sc_start();

    return 0;
}
