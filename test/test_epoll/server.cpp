#include <iostream>
#include <hrpc_epoller.h>
#include <hrpc_socket.h>
#include <cstring>

using namespace Hrpc;

Hrpc_Socket sockets[1024];
size_t cur = 1;
char buffer[1024];
std::string sendBuffer[1024];
int main(int argc, char* argv[])
{


    Hrpc_Socket& sock = sockets[0];
    Hrpc_Epoller ep;    

    try
    {
        sock.CreateSocket(SOCK_STREAM, AF_INET);
        ep.createEpoll(1024);

        sock.setNonBlock(true);
        sock.setReuseAddr();
        sock.bind("127.0.0.1", 8888);
        sock.listen(1024);

        ep.add(sock.getFd(), 0, EPOLLIN);

        while (true)
        {
            sockaddr sa;
            socklen_t sl;
            int count = ep.wait(10);
            for (size_t pos = 0; pos < count; pos++)
            {
                try
                {
                    epoll_event ev = ep.get(pos);

                    if (ev.data.u32 == 0)
                    {
                        if (ev.events & EPOLLIN)
                        {
                            Hrpc_Socket& now = sockets[cur];
                            while (sock.accept(now, &sa, sl))         
                            {
                                now.setNonBlock();
                                // 注册时间
                                ep.add(now.getFd(), cur, EPOLLIN);
                                std::cout << "new connection" << std::endl;
                                cur++;
                            }
                        }
                    }
                    else 
                    {
                        Hrpc_Socket& now = sockets[ev.data.u64];
                        if ((ev.events & EPOLLERR) || (ev.events & EPOLLHUP))
                        {
                            std::cout << "epoll get EPOLLERR or EPOLLHUP, fd = " << now.getFd() << std::endl;
                            continue;
                        }

                        // 输入
                        if (ev.events & EPOLLIN)
                        {
                            memset(buffer, 0, 1024);
                            int cur = 0,tmp = 0;
                            bool flag = false;
                            while (true)
                            {
                                tmp = now.recv(buffer + cur, 1024);

                                if (tmp > 0)
                                {
                                    cur += tmp;
                                    continue;
                                }
                                else if (tmp == 0)
                                {
                                    std::cout << "connection close" << std::endl;
                                    ep.del(now.getFd());
                                    now.close();
                                    flag = true;
                                    break;
                                }
                                else 
                                {
                                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                                    {
                                        break;
                                    }
                                    else
                                    {
                                        std::cout << "a connection error" << std::endl;
                                        ep.del(now.getFd());
                                        now.close();
                                        flag = true;
                                        break;
                                    }
                                }
                            }

                            if (flag)
                            {
                                continue;
                            }
                            // 得到输入， 进行输出
                            memcpy(buffer + cur, "123456", 6);
                            cur += 6;
                            
                            sendBuffer[ev.data.u64] = std::string(buffer, cur);
                            std::cout << "recv message: [" << sendBuffer[ev.data.u64] << "]" << std::endl;
                            // 输出
                            ep.mod(now.getFd(), ev.data.u64, EPOLLOUT);
                        }
                        else if (ev.events & EPOLLOUT)
                        {
                            std::string& msg = sendBuffer[ev.data.u64];
                            if (!msg.empty())
                            {
                                size_t total = msg.size();
                                const char* buf = msg.c_str();
                                bool flag = false;
                                size_t cur = 0;
                                while (cur < total)
                                {
                                    int tmp = now.send(buf + cur, total - cur);

                                    if (tmp > 0)
                                    {
                                        cur += tmp;
                                        continue;
                                    }
                                    else if (tmp == 0)
                                    {
                                        std::cout << "connection close" << std::endl;
                                        ep.del(now.getFd());
                                        now.close();
                                        flag = true;
                                        break;
                                    }
                                    else 
                                    {
                                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            std::cout << "a connection error" << std::endl;
                                            ep.del(now.getFd());
                                            now.close();
                                            flag = true;
                                            break;
                                        }
                                    }        
                                }
                                if (flag)
                                    continue;
                                if (cur == total)
                                {
                                    // 数据发送完成
                                    ep.mod(now.getFd(), ev.data.u64, EPOLLIN);
                                }
                                std::cout << "msg has sent" << std::endl;
                            }
                            else
                            {
                                std::cout << "send msg is null" << std::endl;
                            }
                            
                        }

                    }


                }
                catch (const Hrpc_Exception& e)
                {
                    std::cerr << e.what() << ", errno = " << e.getErrCode() << std::endl;            
                }
            }
        }
    }
    catch(const Hrpc_Exception& e)
    {
        std::cerr << e.what() << ", errno = " << e.getErrCode() << std::endl;            
    }
    
    
    return 0;
}