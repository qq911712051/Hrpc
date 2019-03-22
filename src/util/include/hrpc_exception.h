/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 
 * @Date: 2019-03-22 18:37:55
 */
#ifndef HRPC_EX_H_
#define HRPC_EX_H_

#include <exception>
#include <string>
namespace Hrpc
{
/**
 * @description:  这是一个简单的封装的异常的类
 */
class Hrpc_Exception : public std::exception
{
public:

    /**
     * @description: 传入错误信息
     * @param: str 错误信息
     * @return: 
     */
    Hrpc_Exception(const std::string& str) : std::exception(), _errStr(str), _errCode(0)
    {}

    /**
     * @description: 传入错误信息以及错误代码进行构造
     * @param  str 错误信息 
     * @param  code 错误代码
     * @return: 
     */
    Hrpc_Exception(const std::string& str, int code) : std::exception(), _errStr(str), _errCode(code)
    {
    }

    /**
     * @description: 实现虚函数what，返回错误信息 
     * @param 
     * @return: 返回错误信息 
     */
    virtual const char* what() const throw();
    
    /**
     * @description: 获取错误码 
     * @param 
     * @return: 返回错误代码， 0为无效代码 
     */
    int getErrCode() const;

    /**
     * @description: 析构函数
     * @param 
     * @return: 
     */
    virtual ~Hrpc_Exception()
    {}
private:
    std::string _errStr;    // 错误信息
    int _errCode;       // 错误代码
};
}
#endif