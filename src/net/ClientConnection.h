#ifndef CLIENT_CONNECTION_H_
#define CLIENT_CONNECTION_H_
#include <memory>

#include <ConnectionBase.h>
namespace Hrpc
{

/**
 * 主要用作客户端的主动链接
 */
class ClientConnection
{
public:

private:
    
    ConnectionBase  _base;      // 基础链接
};
using ClientConnectionPtr = std::shared_ptr<ClientConnection>;
}
#endif