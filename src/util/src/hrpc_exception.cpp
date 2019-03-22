#include <hrpc_exception.h>


namespace Hrpc
{

const char* Hrpc_Exception::what() const throw()
{
    return _errStr.c_str();
}

int Hrpc_Exception::getErrCode() const 
{
    return _errCode;
}

}