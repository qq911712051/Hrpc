#include <iostream>
#include <hrpc_socket.h>
#include <cstring>
#include <unistd.h>
#include <time.h>
using namespace Hrpc;
char buffer[1024 * 1024] = "hello\n";

void initBuffer()
{
    size_t last = 1024*1024 -1;
    for (size_t i = 0; i < last; i++)
    {
        buffer[i] = 'a';
    }
    buffer[last] = 0;
}
int main(int argc, char* argv[])
{
    initBuffer();
    Hrpc_Socket sock;
    try
    {
        sock.CreateSocket(SOCK_STREAM, AF_INET, 0);
        sock.setNonBlock(true);
        

        std::cout << "sendBuffer = " << sock.getSendBufferSize() << std::endl;
        std::cout << "recvBuffer = " << sock.getRecvBufferSize() << std::endl;

        sock.setSendBufferSize(1024);
        std::cout << "now sendBuffer = " << sock.getSendBufferSize() << std::endl;

        sock.connect("localhost", 8888);
        while (1)
        {
            std::cout << "start send" << std::endl;
            size_t cur = 0, total = strlen(buffer);
            int tmp = 0;
            while (cur < total)
            {
                
                tmp = sock.send(buffer + cur, total - cur, 0);
                if (tmp > 0)
                {
                    cur += tmp;
                    std::cout << "send " << tmp << "bytes " << std::endl;
                    continue;
                }
                else if (tmp == 0)
                {
                    std::cout << "connection close" << std::endl;    
                    return 0;
                }
                else
                {
                    if (errno == EWOULDBLOCK || errno == EAGAIN)
                    {
                        break;
                    }
                    else
                    {
                        std::cout << "occur error" << std::endl;
                        return 0;
                    }
                    
                }
            }   
            std::cout << "send msg complete" << std::endl;
            ::usleep(2000000);
        }
    }
    catch(const Hrpc_Exception& e)
    {
        std::cerr << e.what() << ", errno = " << e.getErrCode() << std::endl;
    }
    

    return 0;
}