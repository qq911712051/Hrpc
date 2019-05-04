#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>

#include <hrpc_buffer.h>
#include <hrpc_common.h>

namespace Hrpc
{


Hrpc_Buffer::Hrpc_Buffer(size_t len, size_t before)
{
    _buffer = 0;
    _before = before;
    try
    {
        if (len <= 32)
            len = 32;
        
        // 这里buffer前面预留
        _cap = len + _before;
        _cur = _before;
        _end = _before;

        _buffer = new char[_cap];
    } 
    catch (const std::exception& e)
    {
        std::cerr << "[Hrpc_Buffer::Hrpc_Buffer]: " << e.what() << std::endl;
        _buffer = nullptr;
    }
}

Hrpc_Buffer::~Hrpc_Buffer()
{
    if (_buffer)
    {
        delete [] _buffer;
    }
}

bool Hrpc_Buffer::write(const std::string& data, bool flag)
{
    return write(data.c_str(), data.size(), flag);
}

bool Hrpc_Buffer::write(const char* begin, const char* end, bool flag)
{
    return write(begin, end - begin, flag);
}

bool Hrpc_Buffer::write(const char* data, int len, bool flag)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::write]: _buffer is null");
    if (len < 0)
        len = ::strlen(data);

    // 如果允许空间优化
    if (flag)
    {
        optimizeSpace();
    }
    size_t free = freeSize();
    if (len <= free)
    {
        // 直接复制数据到缓冲区
        ::memcpy(_buffer + _end, data, len);
        
        // 更新数据
        _end += len;
    }
    else
    {
        // 需要扩容
        size_t expand = (size() + len) * 2 + _before;
        bool res = expandBuffer(expand);
        if (!res)
            return false;

        // 将新插入数据复制到新的缓冲区
        ::memcpy(_buffer + _end, data, len);

        // 更新游标
        _end += len;
    }
        
    return true;
}


void Hrpc_Buffer::pushData(Hrpc_Buffer&& buf, bool flag)
{
    write(buf.begin(), buf.end(), flag);
}

bool Hrpc_Buffer::expandBuffer(size_t len)
{
    try
    {
        char* tmp = new char[len];
        Hrpc_Common::compiler_barrier();
        _cap = len;
        size_t buffer_size = _end - _cur;
        if (buffer_size > len)
            throw std::runtime_error("param error");
        // 复制数据到新缓冲区
        ::memcpy(tmp + _before, _buffer + _cur, buffer_size);
        // 更新游标
        _cur = _before;
        _end = _cur + buffer_size;

        // 释放旧缓冲区
        delete [] _buffer;
        _buffer = tmp;
    }
    catch (std::exception& e)
    {
        std::cerr << "[Hrpc_Buffer::expandBuffer]: " << e.what() << ", now need buffer size = " << len << std::endl;
        return false;
    }
    return true;
}

bool Hrpc_Buffer::skipBytes(size_t n)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::skipBytes]: _buffer is null");
    if (_end - _cur >= n)
        _cur += n;
    else
    {
        return false;
    }
    return true;
    
}


size_t Hrpc_Buffer::read(char* buffer, size_t len) const
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::read]: _buffer is null");
    if (len >= size())
    {
        ::memcpy(buffer, _buffer + _cur, size());
        return size();
    }
    else
    {
        ::memcpy(buffer, _buffer + _cur, len);
        return len;
    }
}

size_t Hrpc_Buffer::read(std::string& buffer) const
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::read]: _buffer is null");
    buffer.resize(size());

    size_t has = read(&buffer[0], size());
    if (has != size())
    {
        throw Hrpc_BufferException("[Hrpc_Buffer::read]: read size error");
    }
    else
        return size();
}

void Hrpc_Buffer::clear()
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::clear]: _buffer is null");
    _cur = _before;
    _end = _before;
}

Hrpc_Buffer::Hrpc_Buffer(Hrpc_Buffer&& buffer)
{
    _buffer = buffer._buffer;
    _cur = buffer._cur;
    _end = buffer._end;
    _cap = buffer._cap;
    _before = buffer._before;
    
    buffer._buffer = nullptr;
}

Hrpc_Buffer& Hrpc_Buffer::operator=(Hrpc_Buffer&& buffer)
{
    // 释放自己的空间
    if (_buffer)
        delete [] _buffer;

        
    _buffer = buffer._buffer;
    _cur = buffer._cur;
    _end = buffer._end;
    _cap = buffer._cap;
    _before = buffer._before;
    
    buffer._buffer = nullptr;
    
}

bool Hrpc_Buffer::appendFrontInt32(int data)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendFrontInt32]: _buffer is null");
    if (_cur < 4)
        return false;

    std::int32_t res = Hrpc_Common::htonInt32(data);
    
    // 移动游标
    _cur -= 4;

    ::memcpy(_buffer + _cur, (void*)&res, sizeof(res));

    return true;    
}
bool Hrpc_Buffer::appendFrontFloat(float data)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendFrontFloat]: _buffer is null");
    if (_cur < 4)
        return false;

    std::int32_t data1 = *((std::int32_t*)&data);
    std::int32_t res = Hrpc_Common::htonInt32(data1);
    
    // 移动游标
    _cur -= 4;

    ::memcpy(_buffer + _cur, (void*)&res, sizeof(res));

    return true;    
}

bool Hrpc_Buffer::appendInt32(int data, bool flag)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendInt32]: _buffer is null");

    std::int32_t res = Hrpc_Common::htonInt32(data);
    
    write((char*)&res, sizeof(res), flag);
    
    // ::memcpy(_buffer + _end, (void*)&res, sizeof(res));
    // // 移动游标
    // _end += 4;

    return true;
}

bool Hrpc_Buffer::appendFloat(float data, bool flag)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendFloat]: _buffer is null");

    std::int32_t data1 = *((std::int32_t*)&data);
    std::int32_t res = Hrpc_Common::htonInt32(data1);
    
    write((char*)&res, sizeof(res), flag);
    
    // ::memcpy(_buffer + _end, (void*)&res, sizeof(res));
    // // 移动游标
    // _end += 4;

    return true;
}

bool Hrpc_Buffer::appendFrontInt16(std::int16_t data)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendFrontInt16]: _buffer is null");
    if (_cur < 2)
        return false;

    std::int16_t res = Hrpc_Common::htonInt16(data);
    
    // 移动游标
    _cur -= 2;

    ::memcpy(_buffer + _cur, (void*)&res, sizeof(res));

    return true;    
}

bool Hrpc_Buffer::appendInt16(std::int16_t data, bool flag)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendInt16]: _buffer is null");

    std::int16_t res = Hrpc_Common::htonInt16(data);
    
    write((char*)&res, sizeof(res), flag);

    // ::memcpy(_buffer + _end, (void*)&res, sizeof(res));
    // // 移动游标
    // _end += 2;

    return true;
}

bool Hrpc_Buffer::appendFrontInt64(std::int64_t data)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendFrontInt64]: _buffer is null");
    if (_cur < 8)
        return false;

    std::int64_t res = Hrpc_Common::htonInt64(data);
    
    // 移动游标
    _cur -= 8;

    ::memcpy(_buffer + _cur, (void*)&res, sizeof(res));

    return true;    
}
bool Hrpc_Buffer::appendInt64(std::int64_t data, bool flag)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendInt64]: _buffer is null");

    std::int64_t res = Hrpc_Common::htonInt64(data);
    
    write((char*)&res, sizeof(res), flag);

    // ::memcpy(_buffer + _end, (void*)&res, sizeof(res));
    // // 移动游标
    // _end += 2;

    return true;
}


bool Hrpc_Buffer::appendFrontInt8(std::int8_t data)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendFrontInt8]: _buffer is null");
    if (_cur < 1)
        return false;    
    // 移动游标
    _cur -= 1;

    ::memcpy(_buffer + _cur, (void*)&data, sizeof(data));

    return true;    
}

bool Hrpc_Buffer::appendInt8(std::int8_t data, bool flag)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::appendInt8]: _buffer is null");
    
    write((char*)&data, sizeof(data), flag);

    // ::memcpy(_buffer + _end, (void*)&data, sizeof(data));
    // // 移动游标
    // _end += 1;

    return true;
}

std::int32_t Hrpc_Buffer::peekFrontInt32() const
{   
    return peekInt32From(0);
}

float Hrpc_Buffer::peekFrontFloat() const
{
    std::int32_t res = peekInt32From(0);
    return *(float*)&res;
}

std::int64_t Hrpc_Buffer::peekFrontInt64() const
{
    return peekInt64From(0);
}

std::int16_t Hrpc_Buffer::peekFrontInt16() const
{
    return peekInt16From(0);
}

std::int8_t Hrpc_Buffer::peekFrontInt8() const
{

    return peekInt8From(0);
}

std::string Hrpc_Buffer::toByteString() const
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::toByteString]: _buffer is null");
    std::string data;
    for (size_t i = _cur; i < _end - 1; i++)
    {
        data += toHexByteString(int((unsigned char)_buffer[i])) + " ";
    }
    data += toHexByteString(_buffer[_end - 1]);
    return data;
}

std::string Hrpc_Buffer::toHexByteString(int byte) const
{
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << byte;
    std::string res;
    ss >> res;
    return res;    
}

const char* Hrpc_Buffer::find(const std::string& data) const
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::find]: _buffer is null");

    std::string origin(_buffer + _cur, _buffer + _end);
    auto pos = origin.find(data);
    if (pos != std::string::npos)
    {
        return begin() + pos;
    }
    else
    {
        return end();
    }
    return end();
}

std::string Hrpc_Buffer::get(size_t pos, size_t len) const
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::get]: _buffer is null");

    if (pos + len > _end)
        return "";
    
    return std::string(_buffer + _cur + pos, _buffer + _cur + pos + len);
}


Hrpc_Buffer Hrpc_Buffer::getToBuffer(size_t pos, size_t len) const
{

    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::getToBuffer]: _buffer is null");

    if (pos + len > _end)
        return Hrpc_Buffer(32);
    
    Hrpc_Buffer res(len);

    res.write(_buffer + _cur + pos,  _buffer + _cur + pos + len);

    return res;
}

bool Hrpc_Buffer::skipTo(const char* pos)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::skipTo]: _buffer is null");

    auto diff = pos - (_buffer + _cur);

    if (diff > 0 && diff <= _end - _cur)
    {
        _cur += diff;
        return true;
    }
    return false;
    
}

void Hrpc_Buffer::optimizeSpace()
{
    // 前端空闲空间超过70%， 则进行移动操作
    if ((double)_cur / _cap > 0.7)
    {
        auto size = Hrpc_Buffer::size();
        for (size_t i = 0; i < size; i++)   
        {
            _buffer[_before + i] = _buffer[_cur + i];
        }
        
        // 重置游标
        _cur = _before;
        _end = _cur + size;
    }
}

bool Hrpc_Buffer::appendFront(const std::string& data)
{
    auto length = data.size();
    if (_cur >= length)
    {
        _cur -= length;
        // 复制数据
        ::memcpy(_buffer + _cur, data.data(), length);
    }
    else
    {
        return false;
    }
    return true;
}
bool Hrpc_Buffer::overWriteInt16(size_t pos, const std::int16_t& data)
{
    if (pos + 2 > size())
    {
        return false;
    }
    
    auto oldEnd = _end;
    _end = _cur + pos;
    // 写入数据
    appendInt16(data, false);

    // 回复游标
    _end = oldEnd;
    return true;
}
bool Hrpc_Buffer::overWriteInt32(size_t pos, const std::int32_t& data)
{
    if (pos + 4 > size())
    {
        return false;
    }
    
    auto oldEnd = _end;
    _end = _cur + pos;
    // 写入数据
    appendInt32(data, false);

    // 回复游标
    _end = oldEnd;
    return true;
}
bool Hrpc_Buffer::overWriteInt64(size_t pos, const std::int64_t& data)
{
    if (pos + 8 > size())
    {
        return false;
    }
    
    auto oldEnd = _end;
    _end = _cur + pos;
    // 写入数据
    appendInt64(data, false);

    // 回复游标
    _end = oldEnd;
    return true;
}
bool Hrpc_Buffer::overWriteInt8(size_t pos, const std::int8_t& data)
{
    if (pos + 1 > size())
    {
        return false;
    }
    
    auto oldEnd = _end;
    _end = _cur + pos;
    // 写入数据
    appendInt8(data, false);

    // 回复游标
    _end = oldEnd;
    return true;
}

std::int8_t Hrpc_Buffer::peekInt8From(size_t pos) const
{
    if (pos < 0 || _end - pos < 1)
    {
        throw Hrpc_Exception("[Hrpc_Buffer::peekInt8From]: peek error");
    }
    return *(std::int8_t*)(_buffer + _cur + pos);
}
std::int16_t Hrpc_Buffer::peekInt16From(size_t pos) const
{
    if (pos < 0 || _end - pos < 2)
    {
        throw Hrpc_Exception("[Hrpc_Buffer::peekInt16From]: peek error");
    }
    std::int16_t origin = *(std::int16_t*)(_buffer + _cur + pos);
    
    return Hrpc_Common::ntohInt16(origin);
}
std::int32_t Hrpc_Buffer::peekInt32From(size_t pos) const
{
    if (pos < 0 || _end - pos < 4)
    {
        throw Hrpc_Exception("[Hrpc_Buffer::peekInt32From]: peek error");
    }
    std::int32_t origin = *(std::int32_t*)(_buffer + _cur + pos);
    
    return Hrpc_Common::ntohInt32(origin);
}
std::int64_t Hrpc_Buffer::peekInt64From(size_t pos) const
{
    if (pos < 0 || _end - pos < 8)
    {
        throw Hrpc_Exception("[Hrpc_Buffer::peekInt64From]: peek error");
    }
    std::int64_t origin = *(std::int64_t*)(_buffer + _cur + pos);
    
    return Hrpc_Common::ntohInt64(origin);
}

}
