#ifndef HRPC_CONFIG_H_
#define HRPC_CONFIG_H_

#include <map>
#include <string>
#include <memory>

#include <hrpc_exception.h>

namespace Hrpc
{

class Hrpc_ConfigException : public Hrpc_Exception
{
public:
    Hrpc_ConfigException(const std::string& msg) : Hrpc_Exception(msg) {}
    Hrpc_ConfigException(const std::string& msg, int err) : Hrpc_Exception(msg, err) {}
    ~Hrpc_ConfigException() {}
};


/**
 * 读取自定义的配置文件文件使用
 * 配置文件类xml， 格式如下
 * <A>
 *      <B>
 *          <Thread>10</Thread>
 *          <ID>12345</ID>
 *      </B>
 * </A>
 * 
 *  元素值只能以<A>XXX</A> 的形式存在， 其中XXX为A标签的元素值
 *  标签里面要么存放一个值(空值也是一个值)， 要么存放其他的多个标签(此时无法存储值)
 *  例如：
 *      <A>
 *          <B></B>
 *          hahaha
 *      </A>
 *  上面的格式是错误的, 其中不仅存放了值hahaha, 还存了标签B
 * 
 * 不支持 value值中存在空格， 例如 "hehe haha", 这个值会被简化为“hehehaha”, 直接去掉空格
 */
class Hrpc_Config
{

public:

    struct ConfigNode
    {
    public:

        // using NodeMap = std::map<std::string, std::unique_ptr<ConfigNode>>;
    public:
        bool        _isValue = {false};       // 当前节点存放的值
        std::map<std::string, std::unique_ptr<ConfigNode>>     _map;               // 标签的映射
        std::string _value; // 标签存储的具体值
        std::string _option;    // 标签名字
    };
public:

    /**
     * @description: 对于配置文件进行解析
     * @param: file 配置文件名称
     * @return: 
     */
    void parse(const std::string& file);

    /**
     * @description: 此config是否有效 
     * @param {type} 
     * @return: 
     */
    bool isValid() {return _config != nullptr;}

    /**
     * @description: 从配置文件中读取 配置项
     * @param: option 配置项路径 
     * @return: 
     */
    std::string getString(const std::string& path) const;

    /**
     * @description: 读取相关节点
     * @param {type} 
     * @return: 
     */
    const std::unique_ptr<Hrpc_Config::ConfigNode>&
    getConfigNode(const std::string& path);

    // 打印成文本
    void print();
private:
    /**
     * @description: 打印当前节点的document树
     * @param: ptr 节点
     * @param: deep 深度
     * @return: 
     */
    static std::string printConfigNode(const std::unique_ptr<ConfigNode>& ptr, int deep = 0);

    /**
     * @description:  解析文本， 翻译成ConfigNode
     * @param: 待解析的数据
     * @return: 
     */
    static std::unique_ptr<ConfigNode> parseString(const std::string& data);

    /**
     * @description: 检测是否是空白字符
     * @param {type} 
     * @return: 
     */
    static bool isSpace(char ch);

    /**
     * @description: 是否是一个名字字符
     *    A-Z a-z 0-9 _
     * @param {type} 
     * @return: 
     */
    static bool isNameCharacter(char ch);

    /**
     * @description: 检测是否是一个值 
     * @param {type} 
     * @return: 
     */
    static bool isValue(const std::string& data, std::string& text);

    static bool isKeyName(char ch);

    /**
     * @description: 是否能成为一个Value的字符
     * @param {type} 
     * @return: 
     */
    static bool isValidCharacter(char ch);

    /**
     * @description: 是否是一个option
     * @param {type} 
     * @return: 
     */
    static bool isOption(const std::string& name);

    /**
     * @description: 在ptr指向的configNode中寻找path
     * @param: ptr 指定的ConfigNode
     * @param: path 目标路径
     * @return: 
     */
    static std::string getStringByConfigNode(const std::unique_ptr<ConfigNode>& ptr, const std::string& path);

    static const std::unique_ptr<ConfigNode>& getConfigNodeByPath(const std::unique_ptr<ConfigNode>& ptr, const std::string& path);
private:
    std::unique_ptr<ConfigNode> _config;    // 配置文件
};

}
#endif