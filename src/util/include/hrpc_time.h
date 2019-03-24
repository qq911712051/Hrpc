/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 和时间获取相关的类
 * @Date: 2019-03-24 15:07:49
 */

#ifndef HRPC_TIME_H_
#define HRPC_TIME_H_
#include <sys/time.h>
#include <unistd.h>


namespace Hrpc
{
class Hrpc_Time
{
public:
    /**
     * @description: 获取当前时间，秒数
     * @param {type} 
     * @return: 
     */
    static size_t getNowTimeMs();

    /**
     * @description: 获取当前时间，毫秒
     * @param {type} 
     * @return: 
     */
    static size_t getNowTime();
};
}
#endif
       