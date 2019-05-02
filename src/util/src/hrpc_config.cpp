#include <fstream>
#include <iostream>
#include <cstring>

#include <hrpc_config.h>
#include <hrpc_common.h>

namespace Hrpc
{

void Hrpc_Config::parse(const std::string& file)
{
    if (_config)
    {
        _config.release();
    }


    // 开始解析文件
    std::fstream fin(file);


    if (!fin.is_open())
    {
        throw Hrpc_Exception("[Hrpc_Config::parse]: file open error");
    }

    // buffer
    char buffer[1024] = {0};
    std::string data;
    while (fin.getline(buffer, 1024))
    {
        std::size_t length = ::strlen(buffer);
        if (length >= 1023)
        {
            // 出现错误
            throw Hrpc_ConfigException("[Hrpc_Config::parse]: the text line too large");
        }
        // 是否有注释符号#
        for (int i = 0; i < length; i++)
        {
            if (buffer[i] == '#')
            {
                buffer[i] = '\0';  // 忽略后面数据
                break;
            }
        }
        // 压入到有效数据中
        data.append(buffer);
    }
    std::cout << data << std::endl;
    // data.append(buffer);
    // 解析
    _config = parseString(data);
}

std::unique_ptr<Hrpc_Config::ConfigNode> Hrpc_Config::parseString(const std::string& data)
{
    auto root = std::unique_ptr<ConfigNode>(new ConfigNode());
    std::string var;
    // 检测是否是值
    bool res = isValue(data, var);

    if (res)
    {
        root->_isValue = true;
        root->_value = var;
        return root;
    }

    int state = 0;
    std::string firstName;
    std::string lastName;
    std::string others;

    for (int cur = 0; cur < data.size();)
    {
        char now = data[cur];
        if (state == 0)
        {
            if (isspace(now))
            {
                cur++;
                continue;
            }
            else if (now == '<')
            {
                cur++;
                state = 1;
                continue;
            }
            else
            {
                std::string msg(data.begin(), data.begin() + cur);
                throw Hrpc_ConfigException("[Hrpc_Config::parseString]: occur error = [" + msg + "], alldata = [" + data + "], pos = " + Hrpc_Common::tostr(cur));
            }
        }
        else if (state == 1)
        {
            if (isNameCharacter(now))
            {
                firstName += now;
                cur++;
                continue;
            }
            else if (now == '>')
            {
                auto cpos = cur;
                while (cpos < data.size())
                {
                    auto pos = data.find(firstName, cpos);
                    if (pos != std::string::npos)
                    {
                        if (data[pos-2] == '<' && data[pos-1] == '/'
                            && pos + firstName.size() < data.size() && data[pos+firstName.size()] == '>')
                        {
                            // 发现一个节点
                            // 搜索这个节点是否存在
                            if (root->_map.find(firstName) != root->_map.end())
                            {
                                throw Hrpc_ConfigException("[Hrpc_Config::parseString]: multiple same node, name = " + firstName);
                            }

                            std::string ot(data.begin() + cur + 1, data.begin() + pos - 2);
                            root->_map[firstName] = parseString(ot);
                            root->_map[firstName]->_option = firstName;
                            
                            // 重置游标
                            cur = pos + firstName.size() + 1;
                            firstName = "";
                            
                            // 重置状态
                            state = 0;
                            
                            break;
                        }
                        else
                        {
                            cpos = pos + firstName.size();
                            continue;
                        }
                    }
                    else
                    {
                        std::string msg(data.begin(), data.begin() + cur);
                        throw Hrpc_ConfigException("[Hrpc_Config::parseString]: occur error = [" + msg + "], alldata = [" + data + "], pos = " + Hrpc_Common::tostr(cur));
                    }
                }

                if (cpos >= data.size())
                {
                    throw Hrpc_ConfigException("[Hrpc_Config::parseString]: no option = " + firstName); 
                }
                continue;
            }
            else
            {
                std::string msg(data.begin(), data.begin() + cur);
                throw Hrpc_ConfigException("[Hrpc_Config::parseString]: occur error = [" + msg + "], alldata = [" + data + "], pos = " + Hrpc_Common::tostr(cur));
            }
        }
    }

    
    return root;
}

bool Hrpc_Config::isSpace(char ch)
{
    if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
        return true;
    else
    {
        return false;
    }
    
}

bool Hrpc_Config::isNameCharacter(char ch)
{
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch == '_') || (ch >= '0' && ch <= '9'))
        return true;
    else
    {
        return false;
    }
    
}

bool Hrpc_Config::isKeyName(char ch)
{
    if (ch == '<' || ch == '>' || ch == '/')
        return true;
    else
        return false;
}

bool Hrpc_Config::isValue(const std::string& data, std::string& text)
{
    text = "";
    std::string tmp;
    bool isFlag = false;
    for (size_t i = 0; i < data.size(); i++)
    {
        char now = data[i];
        if (isKeyName(now))
        {
            return false;
        }
        if (isspace(now))
        {
            continue;
            
        }
        else if (isValidCharacter(now))
            text += now;
        else 
        {
            throw Hrpc_ConfigException("[Hrpc_Config::isValue]: error charactor = [" + std::string(1, now) + "]");
        }
    }
    return true;
}

void Hrpc_Config::print()
{
    if (_config)
        std::cout << printConfigNode(_config, -1) << std::endl;
    else
    {
        throw Hrpc_ConfigException("[Hrpc_Config::print]: not initialize the Hrpc_Config");
    }
    
}

std::string Hrpc_Config::printConfigNode(const std::unique_ptr<Hrpc_Config::ConfigNode>& ptr, int deep)
{
    // 获得前缀
    std::string prefix;
    for (int i = 0; i < deep; i++)
        prefix += "\t";
    std::string head, tail;
    if (ptr->_option != "")
    {
        head = prefix + "<" + ptr->_option + ">"; 
        tail = prefix + "</" + ptr->_option + ">";
    }

    if (ptr->_isValue)
    {

        return  head + ptr->_value + "</" + ptr->_option + ">\n";
    }

    
    std::string data;
    for (auto& x : ptr->_map)
    {
        data += printConfigNode(x.second, deep + 1);
    }
    return head + "\n" + data + tail + "\n";
}

bool Hrpc_Config::isValidCharacter(char ch)
{
    if (!isSpace(ch) && !isKeyName(ch))
    {
        return true;
    }
    return false;
    
}

std::string Hrpc_Config::getString(const std::string& path) const
{
    if (!_config)
        throw Hrpc_ConfigException("[Hrpc_Config::print]: not initialize the Hrpc_Config");
    if (path.size() == 0 || path[0] != '/')
        return "";
    return getStringByConfigNode(_config, std::string(path.begin() + 1, path.end()));
}
std::string Hrpc_Config::getStringByConfigNode(const std::unique_ptr<ConfigNode>& ptr, const std::string& path)
{
    if(isOption(path))
    {
        if (path == "")
        {
            if (ptr->_isValue)
                return ptr->_value;
            else
            {
                return "";
            }
            
        }

        auto itr = ptr->_map.find(path);
        if (itr != ptr->_map.end())
        {
            if (itr->second->_isValue)
                return itr->second->_value;
            else
            {
                return "";
            }
            
        }
        else
        {
            return "";
        }
        
    }
    else 
    {
        // 找出第一个元素
        auto first = path.find("/");
        if (first != std::string::npos)
        {
            auto option = std::string(path.begin(), path.begin() + first);
            auto itr = ptr->_map.find(option);
            if (itr != ptr->_map.end())
            {
                return getStringByConfigNode(itr->second, std::string(path.begin() + first + 1, path.end()));
            }
            else
            {
                return "";
            }
        }
        else
        {
            throw Hrpc_ConfigException("[Hrpc_Config::getStringByConfigNode]: unknown error");
        }
        
    }
}

bool Hrpc_Config::isOption(const std::string& name)
{
    if (name.size() == 0)
        return true;
    for (auto& x : name)
    {
        if (!isNameCharacter(x))
            return false;
    }
    return true;
}

const std::unique_ptr<Hrpc_Config::ConfigNode>&
Hrpc_Config::getConfigNode(const std::string& path)
{
    if (!_config)
        throw Hrpc_ConfigException("[Hrpc_Config::getConfigNode]: not initialize the Hrpc_Config");
    if (path.size() == 0 || path[0] != '/')
        throw Hrpc_ConfigException("[Hrpc_Config::getConfigNode]: path = " + path + " is error");
    return getConfigNodeByPath(_config, std::string(path.begin() + 1, path.end()));
}

const std::unique_ptr<Hrpc_Config::ConfigNode>& Hrpc_Config::getConfigNodeByPath(const std::unique_ptr<ConfigNode>& ptr, const std::string& path)
{
    if(isOption(path))
    {
        if (path == "")
        {
            return ptr;
            
        }

        auto itr = ptr->_map.find(path);
        if (itr != ptr->_map.end())
        {
            return itr->second;
            
        }
        else
        {
            throw Hrpc_ConfigException("[Hrpc_Config::getConfigNodeByPath]: path not exist");
        }
        
    }
    else 
    {
        // 找出第一个元素
        auto first = path.find("/");
        if (first != std::string::npos)
        {
            auto option = std::string(path.begin(), path.begin() + first);
            auto itr = ptr->_map.find(option);
            if (itr != ptr->_map.end())
            {
                return getConfigNodeByPath(itr->second, std::string(path.begin() + first + 1, path.end()));
            }
            else
            {
                throw Hrpc_ConfigException("[Hrpc_Config::getConfigNode]: path not exist");
            }
        }
        else
        {
            throw Hrpc_ConfigException("[Hrpc_Config::getStringByConfigNode]: unknown error");
        }
        
    }
}
}