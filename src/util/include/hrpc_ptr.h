/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 智能指针实现
 * @Date: 2019-03-27 23:29:49
 */
#ifndef HRPC_PTR_H_
#define HRPC_PTR_H_

#include <hrpc_atomic.h>
#include <hrpc_exception.h>
#include <hrpc_lock.h>
namespace Hrpc
{


/**
 *  智能指针异常类
 */
class Hrpc_SmartPointerException : public Hrpc_Exception
{
public:
    Hrpc_SmartPointerException(const std::string& str) : Hrpc_Exception(str) {}
    Hrpc_SmartPointerException(const std::string& str, int err) : Hrpc_Exception(str, err) {}
    ~Hrpc_SmartPointerException() {}

};

/**
 *  一个计数节点类 
 *  保存指向真实对象的指针， 作为一个代理类存在
 */
template <typename ObjectType, class CountType = Hrpc_Atomic>
class CountNode
{
public:
    /**
     * @description: 初始化参数
     * @param {type} 
     * @return: 
     */
    CountNode(ObjectType* ptr) : _pEle(ptr), _sharedCount(1), _weakCount(0) {}

    /**
     * @description: 析构函数， 理论上什么也不会做 
     * @param {type} 
     * @return: 
     */
    ~CountNode();

    /**
     * @description: 获取原始指针
     * @param {type} 
     * @return: 
     */
    ObjectType* getPointer() const
    {
        return _pEle;
    }

    /**
     * @description: 重载指针
     * @param {type} 
     * @return: 
     */
    ObjectType* operator->();

    /**
     * @description: 重载引用
     * @param {type} 
     * @return: 
     */
    ObjectType& operator*();

    /**
     * @description: 获取原始指针被引用的个数
     * @param {type} 
     * @return: 
     */
    int get_count();
    
    /**
     * @description: 减少1个强指针引用
     * @param {type} 
     * @return: 
     */
    bool decRefShared();

    /**
     * @description: 减少一个弱指针引用
     * @param {type} 
     * @return: 
     */
    bool decRefWeak();

    /**
     * @description: 增加
     * @param {type} 
     * @return: 
     */
    void incRefShared();

    /**
     * @description: 增加
     * @param {type} 
     * @return: 
     */
    void incRefWeak();
private:

    CountType _sharedCount; // 强指针的个数
    CountType _weakCount;   // 弱指针的个数
    ObjectType* _pEle;      // 原始对象地址
};

template <class> class Hrpc_WeakPtr;

template <typename Type>
class Hrpc_SharedPtr
{
    typedef CountNode<Type> node_type;
public:
    Hrpc_SharedPtr();
    ~Hrpc_SharedPtr();
    Hrpc_SharedPtr(Type*);

    Hrpc_SharedPtr& reset(Type*);

    Type* get();
    
    Hrpc_SharedPtr(const Hrpc_SharedPtr&);
    Hrpc_SharedPtr& operator=(const Hrpc_SharedPtr&);
    Hrpc_SharedPtr& operator=(const Hrpc_WeakPtr<Type>&);
    bool operator==(const Hrpc_SharedPtr&);

    Type* operator->()
    {
        if (!_node)
            throw Hrpc_SmartPointerException("[Hrpc_SharedPtr::operator->]: pointer is null");
        _node->getPointer();
    }

    Type& operator*()
    {
        if (!_node)
            throw Hrpc_SmartPointerException("[Hrpc_SharedPtr::operator*]: pointer is null");
        return *(_node->getPointer());
    }

    int count()
    {
        return _node->get_count();
    }
    operator bool()
    {
        return _node != 0;
    }
private:
    node_type* _node;
};

template <typename Type>
Hrpc_SharedPtr<Type>::Hrpc_SharedPtr()
{
    _node = 0;
}

template <typename Type>
Hrpc_SharedPtr<Type>::Hrpc_SharedPtr(Type* ptr)
{
    _node = new node_type(ptr);
}

template <typename Type>
Hrpc_SharedPtr<Type>::~Hrpc_SharedPtr()
{
    if (_node)
    {
        bool res = _node->decRefShared();
        if (res)
        {
            delete _node;
            _node = 0;
        }
    }
}

template <typename Type>
Hrpc_SharedPtr<Type>& Hrpc_SharedPtr<Type>::reset(Type* ptr)
{
    bool res = _node->decRefShared();
    if (res)
    {
        delete _node;
        _node = 0;
    }

    if (ptr)
    {
        _node = new node_type(ptr);
    }
    return *this;
    
}

template <typename Type>
Type* Hrpc_SharedPtr<Type>::get()
{
    return _node->getPointer();
}

template <typename Type>
Hrpc_SharedPtr<Type>::Hrpc_SharedPtr(const Hrpc_SharedPtr<Type>& sp)
{
    _node = sp._node;
    _node->incRefShared();
}



template <typename Type>
Hrpc_SharedPtr<Type>& Hrpc_SharedPtr<Type>::operator=(const Hrpc_SharedPtr<Type>& sp)
{
    if (_node)
    {
        bool res = _node->decRefShared();
        if (res)
        {
            delete _node;
            _node = 0;
        }
    }

    _node = sp._node;
    _node->incRefShared();
    return *this;
}


template <typename Type>
Hrpc_SharedPtr<Type>& Hrpc_SharedPtr<Type>::operator=(const Hrpc_WeakPtr<Type>& sp)
{
    if (_node)
    {
        bool res = _node->decRefShared();
        if (res)
        {
            delete _node;
            _node = 0;
        }
    }

    _node = sp._node;
    _node->incRefShared();
    return *this;
}

template <typename Type>
bool Hrpc_SharedPtr<Type>::operator==(const Hrpc_SharedPtr<Type>& sp)
{
    return _node == sp._node;
}





template <typename ObjectType, class CountType>
CountNode<ObjectType, CountType>::~CountNode()
{
    if (_pEle)
        delete _pEle;
    
}


template <typename ObjectType, class CountType>
ObjectType* CountNode<ObjectType, CountType>::operator->()
{
    if (!_pEle)
        throw Hrpc_SmartPointerException("[CountNode<ObjectType, CountType>::operator->]: object-pointer is null");
    return _pEle;
}

template <typename ObjectType, class CountType>
ObjectType& CountNode<ObjectType, CountType>::operator*()
{
    if (!_pEle)
        throw Hrpc_SmartPointerException("[CountNode<ObjectType, CountType>::operator*]: object-pointer is null");
    return (*_pEle);
}

template <typename ObjectType, class CountType>
int CountNode<ObjectType, CountType>::get_count()
{
    return _sharedCount;
}
    
template <typename ObjectType, class CountType>
void CountNode<ObjectType, CountType>::incRefShared() 
{
    _sharedCount++;
}

template <typename ObjectType, class CountType>
void CountNode<ObjectType, CountType>::incRefWeak() 
{
    _weakCount++;
}

template <typename ObjectType, class CountType>
bool CountNode<ObjectType, CountType>::decRefShared() 
{
    if (_sharedCount <= 0)
    {
        throw Hrpc_SmartPointerException("[CountNode<ObjectType, CountType>::decRefShared]: refCount is error");
    }
    if (_sharedCount == 1)
    {
        _sharedCount = 0;
        delete _pEle;
        _pEle = 0;
        if (_weakCount == 0)
            return true;
    }
    else 
        _sharedCount--;
    return false;
}

template <typename ObjectType, class CountType>
bool CountNode<ObjectType, CountType>::decRefWeak() 
{
    if (_weakCount == 1)
    {
        _weakCount = 0;
        
        // 判断是否已经彻底无效
        if (_sharedCount == 0)
        {
            return true;
        }
    }
    else
    {
        --_weakCount;
    }
    
    return false;
}

}
#endif
