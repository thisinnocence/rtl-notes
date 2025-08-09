/*
 * OS线程可以和SC_THREAD交替运行，对于 event 只有SC调度的协程中触发使用，跨OS的thread需要使用这个实现。
 * also see:  https://github.com/Xilinx/libsystemctlm-soc/blob/master/utils/async_event.h
 *
 */

#include <pthread.h>
#include <unistd.h>

#include "systemc.h"
#include "async_event.h" // see Makefile: it's from $(SYSTEMC_HOME)/examples/sysc/async_suspend

async_event async_event;

SC_MODULE(AsyncExample) {
    SC_CTOR(AsyncExample) {
        SC_THREAD(process_event);
    }

    void process_event() {
        int cnt = 0;
        for(;;) {
            wait(async_event);  // 等待异步事件触发
            printf("Async event caught at %f cnt[%d]\n", sc_time_stamp().to_seconds(), cnt);
            if (cnt++ > 1) {
                exit(0);
            }
        }
    }
};

void* os_thread(void* arg) {
    while (true) {
        sleep(1);  // 模拟一些外部事件
        printf("OS thread: Triggering async event at %f\n", sc_time_stamp().to_seconds());
        async_event.notify();  // 异步通知 SystemC 事件
    }
    return nullptr;
}

int sc_main(int argc, char* argv[]) {
    AsyncExample example("AsyncExample");

    pthread_t tid;
    pthread_create(&tid, nullptr, os_thread, nullptr);

    sc_start();  // 启动仿真
    pthread_join(tid, nullptr);
    return 0;
}
