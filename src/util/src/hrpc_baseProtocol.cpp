#include <hrpc_baseProtocol.h>

namespace Hrpc
{
Hrpc_Buffer Hrpc_BaseProtocol::parse(Hrpc_Buffer&& buf)
{
    throw Hrpc_ProtocolException("[Hrpc_BaseProtocol::parse]: not define the parse-protocol");
}
}