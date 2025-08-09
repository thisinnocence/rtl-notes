#include "systemc.h"

SC_MODULE(Producer) {
    sc_event& e;
    SC_CTOR(Producer, sc_event& ev) : e(ev) {
        SC_THREAD(trigger);
    }

    void trigger() {
        while (true) {
            wait(2, SC_SEC);
            std::cout << "Producer: Event triggered at " << sc_time_stamp() << std::endl;
            e.notify();  // 触发事件
        }
    }
};

SC_MODULE(Consumer) {
    sc_event& e;
    SC_CTOR(Consumer, sc_event& ev) : e(ev) {
        SC_THREAD(catcher);
    }

    void catcher() {
        while (true) {
            wait(e);  // 等待事件触发
            std::cout << "Consumer: Event caught at " << sc_time_stamp() << std::endl;
        }
    }
};

int sc_main(int argc, char* argv[]) {
    sc_event shared_event;  // 跨模块共享的事件

    Producer producer("Producer", shared_event);
    Consumer consumer("Consumer", shared_event);

    sc_start(10, SC_SEC);  // 运行仿真
    return 0;
}
