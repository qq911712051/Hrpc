#ifndef PARSER_H_
#define PARSER_H_

#include <scanner.h>

namespace Hrpc
{

/**
 * 函数参数的类型属性
 */
struct ParamType
{
    std::string     _typeName;  // 参数类型名称
    int             _tokenType;      // 参数的词素类型
};

/**
 * 函数参数的变量属性
 */
struct FuncParam
{
    std::string     _name;  // 参数名称
    ParamType       _type;  // 参数的类型属性

    /**
     * 0 表示返回值位置
     * 1 表示传入
     * 2 表示传出
     */
    int             _direct;    // 方向, 传入或者是传出和位置
};

/**
 * 用作服务端的 func转化的结果
 */
struct ServerFuncResult
{
    std::string _funcCode;      // 生成的函数部分代码
    std::string _switchCode;    // 生成的switch部分的代码
    std::string _funcName;      // 函数的名称
};

/**
 * 自定义语言的转译器
 */
class Parser
{
public:

    /**
     * @description: 对词素流进行翻译
     * @param {type} 
     * @return: 
     */
    static std::string parse(const std::vector<ScannerToken>& tokens);
private:
    /**
     * @description: 在词素流中寻找某种类型的词素
     * @param {type} 
     * @return: 
     */
    static int findToken(const std::vector<ScannerToken>& tokens, int start, int type);
    
    /**
     * @description: 将class转化为c++代码
     * @param {type} 
     * @return: 
     */
    static std::string transferClass(const std::vector<ScannerToken>& tokens, int start, int end);

    /**
     * @description: 对于客户端的代码转化
     * @param {type} 
     * @return: 
     */
    static std::string transferClassForClient(const std::vector<ScannerToken>& tokens, int start, int end);


    /**
     * @description: 对于服务端的代码转化
     * @param {type} 
     * @return: 
     */
    static std::string transferClassForServer(const std::vector<ScannerToken>& tokens, int start, int end);

    /**
     * @description: 转化一个函数为c++代码
     * @param {type} 
     * @return: 
     */
    static std::string transferFuncForClient(const std::vector<ScannerToken>& tokens, int start, int end, const std::string& objName);


    /**
     * @description: 转化一个函数为c++代码， 对于服务端
     * @param {type} 
     * @return: 
     */
    static ServerFuncResult transferFuncForServer(const std::vector<ScannerToken>& tokens, int start, int end, const std::string& objName);

    /**
     * @description: 传入参数列表的词素流， 转化成相关的c++代码
     * @param {type} 
     * @return: 
     */
    static std::vector<FuncParam> transferParaList(const std::vector<ScannerToken>& tokens, int start, int end);

    /**
     * @description: 是否是一个类型词素
     * @param {type} 
     * @return: 
     */
    static bool isTypeToken(const ScannerToken& token);

    /**
     * @description: 从词素流中读取一个类型，返回其结束位置
     * @param {type} 
     * @return: 失败返回-1
     */
    static int loadParameter(const std::vector<ScannerToken>& tokens, int start, FuncParam& param);

    /**
     * @description: 从当前位置读取一个type
     * @param {type} 
     * @return: 
     */
    static int loadParaType(const std::vector<ScannerToken>& tokens, int start, ParamType& param);

    /**
     * @description: 对于函数进行解码
     * @param {type} 
     * @return: 
     */
    static void decodeFuncParam(std::string& resData, const std::vector<FuncParam>& paraList);

    /**
     * @description: 生成客户端函数代码
     * @param {type} 
     * @return: 
     */
    static std::string gernarateFunctionClient(const std::string& objName, const std::string& funcName, const ParamType& retType, const std::vector<FuncParam>& paraList);

    /**
     * @description: 生成服务端函数代码
     * @param {type} 
     * @return: 
     */
    static std::string gernarateFunctionServer(const std::string& objName, const std::string& funcName, const ParamType& retType, const std::vector<FuncParam>& paraList);

    /**
     * @description: 根据函数结果， 生成switch结构的核心代码
     * @param {type} 
     * @return: 
     */
    static std::string gernarateSwitchCode(const std::vector<ServerFuncResult>& vec, const std::string& pre);

    /**
     * @description: 根据func生成 服务端分发 函数中case的代码
     * @param {type} 
     * @return: 
     */
    static std::string gernarateSwitchCase(const std::string& funcName, const ParamType& retType, const std::vector<FuncParam>& paraList, const std::string& pre);

    /**
     * @description: 解析function
     * @param {type} 
     * @return: 
     */
    static void parseFuncBody(const std::vector<ScannerToken>& tokens, int start, int end, ParamType& retType, std::string& funcName, std::vector<FuncParam>& paraList);

    /**
     * @description: 生成文件代码
     * @param {type} 
     * @return: 
     */
    static std::string gernarateFileCode(const std::string& data, const std::string& nameSpace, const std::string& headFileMacro);
};
}
#endif