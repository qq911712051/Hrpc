#include <iostream>
#include <hrpc/HrpcClient.h>
#include <HelloWorld.h>

#include <thread>
#include <mutex>

using namespace Hrpc;

std::mutex _ioMtx;
int test(HrpcClient& client)
{
    bool stop = false;
    auto task = [&client, &stop]() {

        auto proxy = client.stringToProxy<HelloWorldProxy>("HelloWorld");
        
        while (!stop)
        {
            std::string outName;
            int res = proxy->test_hello("hello World", outName);
            {
                std::lock_guard<std::mutex> sync(_ioMtx);
                std::cout << "thread :" << std::this_thread::get_id() << ", res = " << res << ", outname = " << outName << std::endl;
            }
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    int threadNum = 1;
    size_t testTime = 1000;
    std::vector<std::thread> threads;
    for (int i = 0; i < threadNum; i++)
        threads.push_back(std::thread(task));

    std::this_thread::sleep_for(std::chrono::seconds(testTime));
    stop = true;

    for (auto& t : threads)
        t.join();
    std::cout << "test end" << std::endl;
}

int main()
{
    HrpcClient client("/home/abel/study/coding/1.cfg");
    test(client);
    
    // try
    // {
    //     auto proxy = client.stringToProxy<HelloWorldProxy>("HelloWorld");
        
    //     std::string outName;
    //     int res = proxy->test_hello("hello World", outName);
    //     std::cout << "res = " << res << ", outname = " << outName << std::endl;
    // }
    // catch (const Hrpc_Exception& e)
    // {
    //     std::cerr << "catch a excaption:" << e.what() << ", errCOde = " << e.getErrCode() << std::endl;
    // }
    // catch(const std::exception& e)
    // {
    //     std::cerr << "catch a excaption:" << e.what() << std::endl;
    // }
    
}