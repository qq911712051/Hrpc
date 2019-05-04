#include <hrpc_common.h>

#include <hrpc_serializeStream.h>

namespace Hrpc
{

Hrpc_SerializeStream::Hrpc_SerializeStream() : _buffer(1, 0)
{
}

size_t Hrpc_SerializeStream::getBufferSize() const
{
    if (!_valid)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::getBufferSize]: the buffer is null");
    }
    return _buffer.size();
}

void Hrpc_SerializeStream::setBuffer(Hrpc_Buffer&& buf)
{
    _buffer = std::move(buf);
    _valid = true;
}

Hrpc_Buffer Hrpc_SerializeStream::getBuffer()
{
    _valid = false;
    return std::move(_buffer);
}

void Hrpc_SerializeStream::setDataHead(int seq, int type)
{
    Int8 head = 0;
    if (seq < 15)
    {
        head = seq;
        head |= (type << 4);
        _buffer.appendInt8(head);
    }
    else
    {
        head = (type << 4);
        head |= 0xf;
        _buffer.appendInt8(head);

        // 后一个字节单独用来存放seq
        _buffer.appendInt8(seq);
    }

    // 写入2字节的数据长度， 暂时写如空数据， 占位置
    _buffer.appendInt16(0);
}

void Hrpc_SerializeStream::writeTo(int seq, const Int8& data)
{
    setDataHead(seq, INT8_TYPE);

    // 写如主体
    setDataBody(data);
}

void Hrpc_SerializeStream::writeTo(int seq, const Int16& data)
{
    setDataHead(seq, INT16_TYPE);

    // 写如主体
    setDataBody(data);
}

void Hrpc_SerializeStream::writeTo(int seq, const Int32& data)
{
    setDataHead(seq, INT32_TYPE);

    // 写如主体
    setDataBody(data);
}

void Hrpc_SerializeStream::writeTo(int seq, const Int64& data)
{
    setDataHead(seq, INT64_TYPE);

    // 写如主体
    setDataBody(data);
}

void Hrpc_SerializeStream::writeTo(int seq, const Bool& data)
{
    setDataHead(seq, BOOL_TYPE);
    // 写如主体
    setDataBody(data);
}

void Hrpc_SerializeStream::writeTo(int seq, const Float& data)
{
    setDataHead(seq, FLOAT_TYPE);
    // 写如主体
    setDataBody(data);
}

void Hrpc_SerializeStream::writeTo(int seq, const std::string& data)
{
    setDataHead(seq, STRING_TYPE);
    // 写如主体
    setDataBody(data);
}

void Hrpc_SerializeStream::writeData(const Int8& data)
{
    _buffer.appendInt8(data);
}

void Hrpc_SerializeStream::writeData(const Int16& data)
{
    _buffer.appendInt16(data);
}
void Hrpc_SerializeStream::writeData(const Int32& data)
{
    _buffer.appendInt32(data);
}
void Hrpc_SerializeStream::writeData(const Int64& data)
{
    _buffer.appendInt64(data);
}
void Hrpc_SerializeStream::writeData(const Bool& data)
{
    if (data)
    {
        _buffer.appendInt8(1);
    }
    else
    {
        _buffer.appendInt8(0);
    }
    
}
void Hrpc_SerializeStream::writeData(const Float& data)
{
    _buffer.appendFloat(data);
}
void Hrpc_SerializeStream::writeData(const std::string& data)
{
    Int16 size = data.size();
    // 写入字符串长度
    _buffer.appendInt16(size);
    // 写入具体的数据
    _buffer.write(data);
}

int Hrpc_SerializeStream::find(int seq, Int8& type)
{
    size_t cur = 0;
    while (cur < _buffer.size())    
    {
        // 读取头部数据
        Int8 head = _buffer.peekInt8From(cur);
        if ((head & 0xf) == 0xf)
        {
            if (_buffer.size() - cur >= 2)
            {
                Int8 rs = _buffer.peekInt8From(cur + 1);
                if (rs == seq)
                {
                    type = (std::uint8_t)head >> 4;
                    return cur + 4;
                }
                else
                {
                    cur += 2;
                }
                
            }
            else
            {
                return -1;
            }
            
        }
        else if ((head & 0xf) == seq)
        {
            type = (std::uint8_t)head >> 4;
            return cur + 3;
        }
        else
        {
            cur += 1;
        }
        // 确定下一个参数的位置
        if (_buffer.size() - cur >= 2)
        {
            Int16 size = _buffer.peekInt16From(cur);
            if (size >= 0)
            {
                cur += size + 2;
            }
            else
            {
                throw Hrpc_Exception("[Hrpc_SerializeStream::find]: error element size");
            }
            
        }
        else
        {
            return -1;
        }
    }
    return -1;
}

int Hrpc_SerializeStream::readData(int pos, Int8 type, Int8& data)
{
    if (type != INT8_TYPE)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type, element type is " + Hrpc_Common::tostr(int(type)) + ", recv-type is Int8");
    }

    // 判断pos数据的有效性
    if (_buffer.size() - pos >= 1)
    {
        data = _buffer.peekInt8From(pos);
    }
    else
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: pos[" + Hrpc_Common::tostr(pos) + "] doesn't has data");
    }
    return pos + 1;
}
int Hrpc_SerializeStream::readData(int pos, Int8 type, Int16& data)
{
    if (type != INT16_TYPE)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type, element type is " + Hrpc_Common::tostr(int(type)) + ", recv-type is Int16");
    }

    // 判断pos数据的有效性
    if (_buffer.size() - pos >= 2)
    {
        data = _buffer.peekInt16From(pos);
    }
    else
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: pos[" + Hrpc_Common::tostr(pos) + "] doesn't has data");
    }
    return pos + 2;
}

int Hrpc_SerializeStream::readData(int pos, Int8 type, Int32& data)
{
    if (type != INT32_TYPE)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type, element type is " + Hrpc_Common::tostr(int(type)) + ", recv-type is Int32");
    }

    // 判断pos数据的有效性
    if (_buffer.size() - pos >= 4)
    {
        data = _buffer.peekInt32From(pos);
    }
    else
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: pos[" + Hrpc_Common::tostr(pos) + "] doesn't has data");
    }
    return pos + 4;
}

int Hrpc_SerializeStream::readData(int pos, Int8 type, Int64& data)
{
    if (type != INT64_TYPE)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type, element type is " + Hrpc_Common::tostr(int(type)) + ", recv-type is Int64");
    }

    // 判断pos数据的有效性
    if (_buffer.size() - pos >= 8)
    {
        data = _buffer.peekInt64From(pos);
    }
    else
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: pos[" + Hrpc_Common::tostr(pos) + "] doesn't has data");
    }
    return pos + 8;
}

int Hrpc_SerializeStream::readData(int pos, Int8 type, Bool& data)
{
    if (type != BOOL_TYPE)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type, element type is " + Hrpc_Common::tostr(int(type)) + ", recv-type is Bool");
    }
    // 判断pos数据的有效性
    if (_buffer.size() - pos >= 1)
    {
        Int8 res = _buffer.peekInt8From(pos);
        if (res == 1)
        {
            data = true;
        }
        else if (res == 0)
        {
            data = false;
        }
        else
        {
            throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: get bool value error =[" + Hrpc_Common::tostr(res) + "]");
        }
    }
    else
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: pos[" + Hrpc_Common::tostr(pos) + "] doesn't has data");
    }
    return pos + 1;
}

int Hrpc_SerializeStream::readData(int pos, Int8 type, Float& data)
{
    if (type != FLOAT_TYPE)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type, element type is " + Hrpc_Common::tostr(int(type)) + ", recv-type is Float");
    }
    // 判断pos数据的有效性
    if (_buffer.size() - pos >= 4)
    {
        Int32 res = _buffer.peekInt32From(pos);
        data = *(Float*)&res;
    }
    else
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: pos[" + Hrpc_Common::tostr(pos) + "] doesn't has data");
    }
    return pos + 4;
}

int Hrpc_SerializeStream::readData(int pos, Int8 type, std::string& data)
{
    if (type != STRING_TYPE)
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: error recv-type, element type is " + Hrpc_Common::tostr(int(type)) + ", recv-type is std::string");
    }
    // 判断pos数据的有效性
    if (_buffer.size() - pos >= 2)
    {
        // 获取字符串的长度
        Int16 length = _buffer.peekInt16From(pos);

        if (length > 0)
        {
            std::string res = _buffer.get(pos + 2, length);
            // 判断数据是否正确取出
            if (res != "")
            {
                data = res;
            }
            else
            {
                throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: get string type, length-para is error, can't get string");
            }
            
        }
        else if (length == 0)
        {
            data = "";
        }
        
        else
        {
            throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: get string type, length-para is error");
        }
        return pos + 2 + length;
        
    }
    else
    {
        throw Hrpc_Exception("[Hrpc_SerializeStream::readData]: pos[" + Hrpc_Common::tostr(pos) + "] doesn't has data");
    }
    return -1;
}

Int8 Hrpc_SerializeStream::getDataType(const Int8& a)
{
    return INT8_TYPE;
}
Int8 Hrpc_SerializeStream::getDataType(const Int16& a)
{
    return INT16_TYPE;
}
Int8 Hrpc_SerializeStream::getDataType(const Int32& a)
{
    return INT32_TYPE;
}
Int8 Hrpc_SerializeStream::getDataType(const Int64& a)
{
    return INT64_TYPE;
}
Int8 Hrpc_SerializeStream::getDataType(const Bool& a)
{
    return BOOL_TYPE;
}
Int8 Hrpc_SerializeStream::getDataType(const Float& a)
{
    return FLOAT_TYPE;
}
Int8 Hrpc_SerializeStream::getDataType(const std::string& a)
{
    return STRING_TYPE;
}

}