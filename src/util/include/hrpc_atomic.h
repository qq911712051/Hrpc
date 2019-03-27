/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 原子计数 
 * @Date: 2019-03-24 23:45:41
 */
#ifndef HRPC_ATOMIC_H_
#define HRPC_ATOMIC_H_

#include <cstdint>

namespace Hrpc
{

/**
 * @description: 原子计数类， 仅支持gcc编译器 
 */
class Hrpc_Atomic
{
    typedef int32_t atomic_t;  // 原子计数类型
public:
    /**
     * @description: 构造函数，初始化计数值
     * @param: val 初始值
     * @return: 
     */
    Hrpc_Atomic(int val) : _count(val) {}

    /**
     * @description: 默认构造函数，初始为
     * @param {type} 
     * @return: 
     */
    Hrpc_Atomic() : _count(0) {}
    
    /**
     * @description: 拷贝构造
     * @param {type} 
     * @return: 
     */
    Hrpc_Atomic(const Hrpc_Atomic& a) : _count(a._count) {} 

    /**
     * @description: 赋值
     * @param {type} 
     * @return: 
     */
    Hrpc_Atomic& operator=(const Hrpc_Atomic& a)
    {
        _count = a.get();
    } 

    /**
     * @description: 赋值
     * @param {type} 
     * @return: 
     */
    Hrpc_Atomic& operator=(int a)
    {
        _count = a;
    } 
    

    /**
     * @description: 获取原子计数值
     * @param {type} 
     * @return: 
     */
    atomic_t get() const {return _count;}

    /**
     * @description: 设置原子计数值
     * @param: val 计数值
     * @return: 
     */
    atomic_t set(int val)
    {
        return _count = val;
    }

    /**
     * @description: 将原子值增加
     * @param: val 增加值
     * @return: 返回增加之前的原子值
     */
    atomic_t add(int val)
    {
        return atomic_add(val);
    }

    /**
     * @description: 将原子值减少
     * @param: val 
     * @return: 返回减少之前的原子值
     */
    atomic_t sub(int val)
    {
        val = -val;
        return atomic_add(val);
    }

    /**
     * @description: 重载+=
     * @param {type} 
     * @return: 
     */
    atomic_t operator+=(int val)
    {
        return add(val);
    }

    /**
     * @description: 重载-=
     * @param {type} 
     * @return: 
     */
    atomic_t operator-=(int val)
    {
        return sub(val);
    }

    /**
     * @description: 前置++
     * @param {type} 
     * @return: 返回变更后的值
     */
    atomic_t operator++()
    {

        atomic_t tmp = atomic_add(1); 
        return tmp + 1;
    }

    /**
     * @description: 后置++
     * @param {type} 
     * @return: 返回变更前的值
     */
    atomic_t operator++(int)
    {
        return atomic_add(1); 
    }

    /**
     * @description: 前置--
     * @param {type} 
     * @return: 返回变更后的值
     */
    atomic_t operator--()
    {
        atomic_t tmp = atomic_add(-1);
        return tmp - 1;
    }

    /**
     * @description: 后置--
     * @param {type} 
     * @return: 返回变更前的值
     */
    atomic_t operator--(int)
    {
        return atomic_add(-1);
    }

    /**
     * @description: 自动转化为int 
     * @param {type} 
     * @return: 
     */
    operator int()
    {
        return _count;
    }

private:
    /**
     * @description: 原子性的增加计数值，使用gcc编译器提供的API
     * @param {type} 
     * @return: 返回变更前的值
     */
    atomic_t atomic_add(int val)
    {
        return __sync_fetch_and_add(&_count, val);
    }    
    
private:
    atomic_t _count;  // 计数值
};
}
#endif