#ifndef HRPCPROTOCOL_H_
#define HRPCPROTOCOL_H_

#include <hrpc_baseProtocol.h>
namespace Hrpc
{

class HrpcProtocol : public Hrpc_BaseProtocol
{
public:
    HrpcProtocol() = default;
    /**
     * @description: 对于Hrpc协议进行解析
     * @param:
     * @return: 
     */
    Hrpc_Buffer parse(Hrpc_Buffer&& buf) override;
    
    /**
     * @description: 获取协议名称
     * @param {type} 
     * @return: 
     */
    static std::string getName() {return _name;};
    /**
     * @description: 析构函数
     * @param {type} 
     * @return: 
     */
    ~HrpcProtocol() override;
private:
    static std::string _name;
};
// 定义此协议的名称
std::string HrpcProtocol::_name = "HRPC";
}
#endif