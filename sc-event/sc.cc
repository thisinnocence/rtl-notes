/*
 *  1. SystemC默认用的是协程，在OS视角看是其实是单thread，所以遇到OS函数调用带来的OS线程的阻塞，
 *  会阻塞真个协程调度器，即所有的协程。
 *
 *  2. 同样，SystemC是非抢占式的调度，如果一个SC_THREAD陷入了 deadloop，那么所有的SC_THREAD都会
 *  得不到调度了。
 */

#include <unistd.h>
#include <stdio.h>

#include "systemc.h"

SC_MODULE(SimpleModule) {
    SC_CTOR(SimpleModule) { // 表示 SC_CONSTRUCTOR 
        SC_THREAD(thread1); // 注册第一个 SC_THREAD 进程
        SC_THREAD(thread2); // 注册第二个 SC_THREAD 进程
    }

    void thread1() {
        for (int i = 0; i < 5; ++i) { // 运行 5 秒后退出
            printf("thread1: Current simulation time: %s\n", sc_time_stamp().to_string().c_str());
            wait(1, SC_SEC); // 等待 1 秒, wait会触发仿真时间推进 + 协程调度器调度
            if (i == 3) {
                printf("thread1 begin: unistd sleep 2 seconds\n");
                sleep(2);
                printf("thread1 end: unistd sleep 2 seconds");
            }
        }
    }

    void thread2() {
        for (int i = 0; i < 5; ++i) { // 运行 5 秒后退出
            printf("thread2: Current simulation time: %s\n", sc_time_stamp().to_string().c_str());
            wait(1, SC_SEC); // 等待 1 秒
        }
    }
};

int sc_main(int argc, char* argv[]) {
    SimpleModule simple_module("simple_module");
    sc_start();
    return 0;
}
