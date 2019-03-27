/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description:  日志系统
 * @Date: 2019-03-27 14:19:04
 */
#ifndef HRPC_LOGGER_H_
#define HRPC_LOGGER_H_

#include <iostream> // test

#define MSG_ERROR 
namespace Hrpc
{
/**
 * @description:  仅仅作为测试， 用std::cout 代替输出
 * @param {type} 
 * @return: 
 */
class Hrpc_Logger
{
public:

    enum
    {
        Logger_ERROR,
        Logger_WARNING,
        Logger_DEBUG
    };
public:    
    Hrpc_Logger() : _out(std::cout) {}
    
    static std::ostream& getInstance(int level = Logger_DEBUG)
    {
        return _out;
    }
private:
    std::ostream& _out;    // 输出流
};


}
#endif