/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 进行序列化的工具 
 * @Date: 2019-05-04 13:12:18
 */
#ifndef HRPC_SERIALIZE_STREAM_H_
#define HRPC_SERIALIZE_STREAM_H_

#include <vector>
#include <map>

#include <hrpc_buffer.h>
#include <hrpc_common.h>

namespace Hrpc
{
// 定义序列化的基本类型, 兼容不同位数的机器

using Int8 = std::int8_t;
using Int16 = std::int16_t;
using Int32 = std::int32_t;
using Int64 = std::int64_t;
using Bool = bool;
using Float = float;

/**
 * 序列化流， 进行参数的序列化
 *  序列化的结构为
 *     |----4bit---|----4bit---|  + 2byte数据长度 + data
 *        类型         变量的标号
 * 
 *  如果变量的标号大于等于4bit所能表示的最大值15， 则用下一个字节来存储标号
 */
class Hrpc_SerializeStream
{
    enum
    {
        INT8_TYPE = 1,
        INT16_TYPE = 2,
        INT32_TYPE = 3,
        INT64_TYPE = 4,
        FLOAT_TYPE = 5,
        BOOL_TYPE = 6,  
        /**
         *  type + seq + length + context 
         */
        STRING_TYPE = 7,       
        /**
         * type + seq + subType + length + context
         */
        VECTOR_TYPE = 8,        

        /**
         * type + seq + subType1 + subType2 + length
         */
        MAP_TYPE = 9
    };
public:
    /**
     * @description: 序列化数据到缓冲区
     * @param {type} 
     * @return: 
     */
    template<typename T>
    void write(int seq, const T& data);

    /**
     * @description: 读取序列化的数据
     * @param {type} 
     * @return: 返回一个bool值， 是否找到数据
     */
    template<typename T>
    bool read(int seq, T& data);

    
    /**
     * @description: 构造函数
     * @param {type} 
     * @return: 
     */
    Hrpc_SerializeStream();

    /**
     * @description: 设置序列化流的buffer
     * @param {type} 
     * @return: 
     */
    void setBuffer(Hrpc_Buffer&& buf);

    /**
     * @description: 获取序列化的buffer
     * @param {type} 
     * @return: 
     */
    Hrpc_Buffer getBuffer();

    /**
     * @description: 返回缓冲区大小
     * @param {type} 
     * @return: 
     */
    size_t getBufferSize() const;
private:
    /**
     * @description: 从pos出读取数据
     * @param {type} 
     * @return: 
     */
    template<typename T>
    int readData(int pos, Int8 type, T& data);

    /**
     * @description: 从特定位置读取特定类型数据
     * @param {type} 
     * @return: 返回数据结束的位置
     */
    int readData(int pos, Int8 type, Int8& data);
    int readData(int pos, Int8 type, Int16& data);
    int readData(int pos, Int8 type, Int32& data);
    int readData(int pos, Int8 type, Int64& data);
    int readData(int pos, Int8 type, Bool& data);
    int readData(int pos, Int8 type, Float& data);
    int readData(int pos, Int8 type, std::string& data);
    
    
    template<typename T>
    int readData(int pos, Int8 type, std::vector<T>& data);

    template<typename K, typename V>
    int readData(int pos, Int8 type, std::map<K, V>& data);
private:
    
    /**
     * @description: 在buffer中寻找seq标号的位置
     * @param {type} 
     * @return: 
     */
    int find(int seq, Int8& type);
    /**
     * @description: 封装seq和type为一个整体
     * @param {type} 
     * @return:
     */
    void setDataHead(int seq, int type);

    /**
     * @description: 设置元素数据体
     * @param {type} 
     * @return: 
     */
    template<typename T>
    void setDataBody(const T& obj);

    /**
     * @description: 根据参数T获取对应的序列化类型
     * @param {type} 
     * @return: 
     */
    template<typename T>
    Int8 getDataType(const T& a);
    
    Int8 getDataType(const Int8& a);
    Int8 getDataType(const Int16& a);
    Int8 getDataType(const Int32& a);
    Int8 getDataType(const Int64& a);
    Int8 getDataType(const Bool& a);
    Int8 getDataType(const Float& a);
    Int8 getDataType(const std::string& a);

    template<typename T>
    Int8 getDataType(const std::vector<T>& a);

    template<typename K, typename V>
    Int8 getDataType(const std::map<K, V>& a);
private:
    
    /**
     * @description: 异常情况， 未知类型 
     * @param {type} 
     * @return: 
     */
    template<typename T>
    void writeTo(int seq, const T& data);
    void writeTo(int seq, const Int8& data);
    void writeTo(int seq, const Int16& data);
    void writeTo(int seq, const Int32& data);
    void writeTo(int seq, const Int64& data);
    void writeTo(int seq, const Bool& data);
    void writeTo(int seq, const Float& data);
    void writeTo(int seq, const std::string& data);

    template<typename T>
    void writeTo(int seq, const std::vector<T>& data);

    template<typename K, typename V>
    void writeTo(int seq, const std::map<K, V>& data);
private:
    /**
     * @description:异常情况, 位置类型 
     * @param {type} 
     * @return: 
     */
    template<typename T>
    void writeData(const T& data);

    /**
     * @description: 正常情况下 
     * @param {type} 
     * @return: 
     */
    void writeData(const Int8& data);
    void writeData(const Int16& data);
    void writeData(const Int32& data);
    void writeData(const Int64& data);
    void writeData(const Bool& data);
    void writeData(const Float& data);
    void writeData(const std::string& data);

    template<typename T>
    void writeData(const std::vector<T>& data);

    template<typename K, typename V>
    void writeData(const std::map<K, V>& data);

private:
    Hrpc_Buffer _buffer;    // buffer    
    bool        _valid = {false};     // 是否已经设置了buffer
};


template<typename T>
Int8 Hrpc_SerializeStream::getDataType(const T& a)
{
    throw Hrpc_Exception("[Hrpc_SerializeStream::getDataType]: unknown type");
}
template<typename T>
Int8 Hrpc_SerializeStream::getDataType(const std::vector<T>& a)
{
    return VECTOR_TYPE;
}
template<typename K, typename V>
Int8 Hrpc_SerializeStream::getDataType(const std::map<K, V>& a)
{
    return MAP_TYPE;
}

template<typename T>
void Hrpc_SerializeStream::write(int seq, const T& data)
{
    if (!_valid || seq < 0 || seq > 127)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream：：write]: write error, buffer is null or seq error");
    }
    
    // 传入到data
    writeTo(seq, data);
}
    
template<typename T>
void Hrpc_SerializeStream::writeTo(int seq, const T& data)
{
    throw Hrpc_Exception("[Hrpc_SerializeStream::writeTo]: write error, unknown type");
}

template<typename T>
void Hrpc_SerializeStream::setDataBody(const T& data)
{
    // 记录 元素size的位置
    auto pos = _buffer.size() - 2;
    // 写入vector中数据类型
    writeData(data);
    
    // 计算空间大小
    Int16 eleSpaceSize = _buffer.size() - pos - 2;
    // 覆盖写入
    _buffer.overWriteInt16(pos, eleSpaceSize);
}

template<typename T>
void Hrpc_SerializeStream::writeTo(int seq, const std::vector<T>& data)
{
    // 写入头部
    setDataHead(seq, VECTOR_TYPE);
    
    // 写如主体
    setDataBody(data);
    
}
template<typename K, typename V>
void Hrpc_SerializeStream::writeTo(int seq, const std::map<K, V>& data)
{
    // 写入头部
    setDataHead(seq, MAP_TYPE);
    // 写如主体
    setDataBody(data);
}

template<typename T>
void Hrpc_SerializeStream::writeData(const T& data)
{
    throw Hrpc_Exception("[Hrpc_SerializeStream::writeData]: unknown type");  
}


template<typename T>
void Hrpc_SerializeStream::writeData(const std::vector<T>& data)
{
    // 写入元素的类型,1字节
    _buffer.appendInt8(getDataType(T()));

    // 写入元素数量
    Int16 size = data.size();
    _buffer.appendInt16(size);

    // 写入具体的元素
    for (auto& t : data)
    {
        writeData(t);
    }

}

template<typename K, typename V>
void Hrpc_SerializeStream::writeData(const std::map<K, V>& data)
{
    // 写入元素类型
    Int8 type = getDataType(K()) << 4;
    type |= getDataType(V());

    _buffer.appendInt8(type);
    // 写入元素数量
    Int16 size = data.size();
    _buffer.appendInt16(size);
    // 写入具体数据
    for (auto& t : data)
    {
        // 写入第一个元素， 写入第二个元素
        writeData(t.first);
        writeData(t.second);
    }

}



template<typename T>
bool Hrpc_SerializeStream::read(int seq, T& data)
{
    Int8 type = -1;
    int pos = find(seq, type);
    if (pos < 0)
    {
        return false;
    }
    // 读取数据， 如果类型不匹配， 直接抛异常
    readData(pos, type, data);
    return true;
}

template<typename T>
int Hrpc_SerializeStream::readData(int pos, Int8 type, T& data)
{
    throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type");
}

template<typename T>
int Hrpc_SerializeStream::readData(int pos, Int8 type, std::vector<T>& data)
{
    if (type != VECTOR_TYPE)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type, element type is " + Hrpc_Common::tostr(int(type)) + ", recv-type is std::vector");
    }

    if (_buffer.size() - pos >= 3)
    {
        Int8 eleType = _buffer.peekInt8From(pos);
        Int16 eleCount = _buffer.peekInt16From(pos + 1);

        // 判断数据时候出错
        if (getDataType(T()) != eleType || eleCount < 0)
        {
            throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: read Vector error");    
        }
        
        // 读取数据
        int lastPos = pos + 3;
        data.clear();
        for (int i = 0; i < eleCount; i++)
        {
            T tmp;
            lastPos = readData(lastPos, eleType, tmp);
            // 将读取的数据压入vector
            data.push_back(std::move(tmp));
        }

        // 返回结束位置
        return lastPos;
    }
    else
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: pos[" + Hrpc_Common::tostr(pos) + "] doesn't has data");
    }
    return -1;
}

template<typename K, typename V>
int Hrpc_SerializeStream::readData(int pos, Int8 type, std::map<K, V>& data)
{
    if (type != MAP_TYPE)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type, element type is " + Hrpc_Common::tostr(int(type)) + ", recv-type is std::map");
    }

    if (_buffer.size() - pos >= 3)
    {
        Int8 eleType = _buffer.peekInt8From(pos);
        // 获取K和V的类型
        Int8 typeK = (std::uint8_t)eleType >> 4;
        Int8 typeV = eleType & 0xf;

        Int16 eleCount = _buffer.peekInt16From(pos + 1);

        // 判断数据时候出错
        if (getDataType(K()) != typeK || getDataType(V()) != typeV || eleCount < 0)
        {
            throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: read map error");    
        }
        
        // 读取数据
        int lastPos = pos + 3;
        data.clear();
        for (int i = 0; i < eleCount; i++)
        {
            K tmpK;
            V tmpV;
            // 读取第一个值
            lastPos = readData(lastPos, typeK, tmpK);

            Hrpc_Common::compiler_barrier();
            // 读取第二个值
            lastPos = readData(lastPos, typeV, tmpV);
            // 将数据压入 data中
            data[tmpK] = std::move(tmpV);
        }
        
        // 返回结束位置
        return lastPos;
    }
    else
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: pos[" + Hrpc_Common::tostr(pos) + "] doesn't has data");
    }
    return -1;
}
}
#endif