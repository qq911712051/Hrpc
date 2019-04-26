#include <iostream>
#include <thread>
#include <cassert>
#include <memory>
#include <chrono>
#include <atomic>

#include <hrpc_ptr.h>
#include <hrpc_timer.h>
#include <hrpc_time.h>
#include <hrpc_queue.h>
#include <hrpc_config.h>
using namespace Hrpc;

using namespace std::chrono;


void test_timer()
{
    Hrpc_Timer timer;

    auto f1 = [](){
        std::cout << "hehee now time is " << Hrpc_Time::getNowTimeMs() << std::endl;
    };
    auto f2 = []() {
        std::cout << "this is call once in " << Hrpc_Time::getNowTimeMs() << std::endl;
    };
    auto f3 = [](){
        std::cout << "f3 now time is " << Hrpc_Time::getNowTimeMs() << std::endl;
    };
    auto id = timer.addTaskRepeat(0, 1000, f1);
    auto id2 = timer.addTaskOnce(2000, f2);
    auto id3 = timer.addTaskRepeat(3000, 2000, f3);
    auto start = Hrpc_Time::getNowTime();
    while (true)
    {
        timer.process();   

        

        if (Hrpc_Time::getNowTime() - start > 10)
        {
            bool res = timer.stopTask(id);
            if (res)
            {
                std::cout << "stop f1 succ" << std::endl;
            }
            else
            {
                std::cout << "stop f1 error" << std::endl;
            }
            break;
        }
    }
    timer.process();
    std::cout << "will stop same task again" << std::endl;
    std::cout << timer.stopTask(id) << std::endl;
}


void test_queue()
{
    std::mutex ioLock;
    bool bStop = false;
    Hrpc_Queue<std::unique_ptr<int>> queue;
    std::atomic<int> count;
    auto producer = [&queue, &count, &bStop](){
        while (!bStop)
        {
            int now = count.fetch_add(1);
            queue.push(std::unique_ptr<int>(new int(now)));
            std::this_thread::sleep_for(milliseconds(100));
        }
    };

    auto consumer = [&queue, &bStop, &ioLock](){
        while (!bStop)
        {
            // int data = queue.pop();
            auto ptr = queue.pop(100);
            if (ptr)
            {
                std::lock_guard<std::mutex> sync(ioLock);
                std::cout << "get data is " << *ptr << std::endl;
            }
        }
    };

    std::thread pp[5];
    for (auto& t : pp)
    {
        t = std::thread(producer);
    }


    std::thread cc[3];
    for (auto& t : cc)
    {
        t = std::thread(consumer);
    }

    auto start = Hrpc_Time::getNowTime();
    while (Hrpc_Time::getNowTime() - start < 5)
    {
        std::this_thread::sleep_for(seconds(1));
    }
    bStop = true;
    std::cout << "wait other thread end" << std::endl;
    for (auto& t : pp)
    {
        t.join();
    }
    for (auto& t : cc)
    {
        t.join();
    }

}
void test_config()
{

    Hrpc_Config config;
    config.parse("/home/abel/study/coding/1.cfg");
    config.print();
    std::cout << "------------------------" << std::endl;
    auto& node = config.getConfigNode("/hrpc/server/BindAdapter/");
    
    std::cout << "the size = " << node->_map.size() << std::endl;
    for (auto& x : node->_map)
    {
        std::cout << "****************" << std::endl;
        for (auto& s : x.second->_map)
            std::cout << s.second->_option << ":" << s.second->_value << std::endl;
        std::cout << "****************" << std::endl;
    }

}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        assert(false);
    }
    std::string test = argv[1];

    if (test == "Hrpc_Timer")
    {
        // 定时器测试
        test_timer();
    }
    else if (test == "Hrpc_Queue")
    {
        // 并发队列测试
        test_queue();
    }
    else if (test == "Hrpc_Config")
    {
        // 测试config配置读取
        test_config();
    }
    else
    {
        std::cerr << "unknown test project" << std::endl;
    }
    
    return 0;    
}