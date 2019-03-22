/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 封装了socket相关操作
 * @Date: 2019-03-22 18:25:57
 */

#ifndef HRPC_SOCKET_H_
#define HRPC_SOCKET_H_

/**
 * @description: 这是一个封装socket操作的一个类， 包括socket设置  
 */
class Hrpc_Socket
{
public:

private:
    int _fd;    // 所拥有的描述符
    bool _bOwner;   // 是否拥有此描述符
    int _domain;    // socket所属于的域
};
#endif