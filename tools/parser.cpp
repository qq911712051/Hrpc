#include <iostream>

#include <hrpc_exception.h>
#include <hrpc_common.h>

#include <parser.h>

namespace Hrpc
{
void Parser::decodeFuncParam(std::string& resData, const std::vector<FuncParam>& paraList)
{
    if (paraList.size() == 0)
    {
        return;
    }
    // 添加参数
    for (int i = 0; i < paraList.size(); i++)
    {
        FuncParam x = paraList[i];
        if (x._direct == 1)
        {
            int tokenType = x._type._tokenType;
            if (tokenType != TOKEN_STRING && tokenType != TOKEN_VECTOR && tokenType != TOKEN_MAP)
            {
                if (i == 0)
                {
                    // 不需要添加const
                    resData += x._type._typeName + " " + x._name;     // 添加参数
                }
                else
                {
                    resData += ", " + x._type._typeName + " " + x._name;
                }
                
            }
            else
            {
                if (i == 0)
                {
                    // 不需要添加const
                    resData += "const " + x._type._typeName + "& " + x._name;     // 添加参数
                }
                else
                {
                    resData += ", const" + x._type._typeName + "& " + x._name;
                }
            }
            
        }
        else if (x._direct == 2)
        {
            if (i == 0)
            {
                // 不需要添加const
                resData += x._type._typeName + "& " + x._name;     // 添加参数
            }
            else
            {
                resData += ", " + x._type._typeName + "& " + x._name;
            }
        }
    }
}

std::string Parser::gernarateSwitchCase(const std::string& funcName, const ParamType& retType, const std::vector<FuncParam>& paraList, const std::string& pre)
{
    std::string retData;
    // 申明所有的变量
    retData += pre + retType._typeName + " ret\n"; 
    for (int i = 0; i < paraList.size(); i++)
    {
        FuncParam x = paraList[i];
        retData += pre + x._type._typeName + " " + x._name + ";\n";
        retData += pre + "s.read(" + Hrpc_Common::tostr(i+1) + ", " + x._name + ");\n";
    }
    
    // 进行调用
    retData += pre + "ret = " + funcName + "(";
    // 填入参数
    for (int i = 0; i < paraList.size(); i++)
    {
        if (i == 0)
        {
            retData += paraList[i]._name;
        }
        else
        {
            retData += ", " + paraList[i]._name;
        }
    }
    retData += ");\n";

    retData += pre + "Hrpc_Buffer response;\n";
    retData += pre + "s.setBuffer(std::move(response));\n";
    
    // 回写返回值
    retData += pre + "s.write(0, ret);\n";
    // 回写out参数
    for (int i = 0; i < paraList.size(); i++)
    {
        if (paraList[i]._direct == 2)
        {
            retData += pre + "s.write(" + Hrpc_Common::tostr(i+1) + ", " + paraList[i]._name + ");\n";
        }
    }

    // 添加返回值
    retData += pre + "return std::move(s.getBuffer());\n";

    return retData;
}

std::string Parser::gernarateFunctionClient(const std::string& objName, const std::string& funcName, const ParamType& retType, const std::vector<FuncParam>& paraList)
{
    std::string resData;
    std::string prefix = "\t";

    resData += retType._typeName + " ";      // 添加返回值
    resData += objName + "::";              // 添加对象域
    resData += funcName + "(";              // 添加函数名
    
    decodeFuncParam(resData, paraList);     // 添加函数参数
    resData += ") \n{\n";                         // 闭合函数                    
    
    // 进行函数体的拼装
    resData += "\tHrpc_SerializeStream s;\n";
    resData += "\tHrpc_Buffer buf(512, 32);\n";
    resData += "\t" + retType._typeName + " ret;\n";
    resData += "\ts.setBuffer(std::move(buf));\n";

    // 进行参数的序列化
    for (int i = 1; i <= paraList.size(); i++)
    {
        resData += "\ts.write(" + Hrpc_Common::tostr(i) + ", " + paraList[i-1]._name + ");\n";
    }

    resData += "\tauto response = involve(ObjectProxy::HRPC_FUNC, \"" + funcName + "\", std::move(s.getBuffer()));\n";
    
    // 对返回结果进行解码
    resData += "\ts.setBuffer(std::move(response));\n";

    resData += "\ts.read(0, ret);\n";
    for (int i = 1; i <= paraList.size(); i++)
    {
        if (paraList[i-1]._direct == 2)
        {
            resData += "\ts.read(" + Hrpc_Common::tostr(i) + ", " + paraList[i-1]._name + ");\n";
        }
    }

    resData += "\treturn ret;\n";

    resData += "}\n";         // 函数封装完成

    return resData;
}

std::string Parser::gernarateFunctionServer(const std::string& objName, const std::string& funcName, const ParamType& retType, const std::vector<FuncParam>& paraList)
{
    std::string resData;
    std::string prefix = "\t";

    resData += "virtual " + retType._typeName + " ";      // 添加返回值
    resData += objName + "::";              // 添加对象域
    resData += funcName + "(";              // 添加函数名
    
    decodeFuncParam(resData, paraList);     // 添加函数参数
    resData += ") \n{\n";                         // 闭合函数                    
    
    resData += "\tthrow Hrpc_Exception(\"[" + objName + "]: not implement the " + funcName + " function\");\n";

    resData += "}\n";         // 函数封装完成

    return resData;
}


std::string Parser::gernarateSwitchCode(const std::vector<ServerFuncResult>& vec, const std::string& pre)
{
    std::string retData;
    retData += pre + "switch (diff)\n";
    retData += pre + "{\n";
    // 具体的实现
    std::string newPre = pre + "\t";
    for (int i = 0; i < vec.size(); i++)
    {
        retData += newPre + "case " + Hrpc_Common::tostr(i) + ":\n";
        retData += newPre + "{\n";

        retData += vec[i]._switchCode;  // 添加case代码

        retData += newPre + "}\n";   
    }
    retData += pre + "};\n";
    return retData;
}

int Parser::findToken(const std::vector<ScannerToken>& tokens, int start, int type)
{
    
    for (int i = start; i < tokens.size(); i++)
    {
        if (tokens[i].type == type)
        {
            return i;
        }
    }
    return -1;
}

std::string Parser::transferClass(const std::vector<ScannerToken>& tokens, int start, int end)
{
    // 转化出客户端的代码
    std::string clientClassCode = transferClassForClient(tokens, start, end);

    // 转化出服务端代码
    std::string serverClassCode = transferClassForServer(tokens, start, end);

    std::cout << "------------------------------------------" << std::endl;
    std::cout << clientClassCode << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << serverClassCode << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    return "";
}

std::string Parser::transferClassForClient(const std::vector<ScannerToken>& tokens, int start, int end)
{
    std::string resData;
    std::string objName = tokens[start+1].value;
    
    int cur = start + 3;
    while (cur < end)
    {
        int r1 = findToken(tokens, cur, TOKEN_SEMICOLON);
        std::string funRes = transferFuncForClient(tokens, cur, r1 - 1, objName);
        resData += funRes;
        cur = r1 + 1;
    }
    
    // 添加类的信息
    std::string classData;
    classData += "class " + objName + " : public ObjectProxy\n";    // 添加类名
    classData += "{\npublic:\n"; // 添加隐私控制变量

    classData += resData;

    classData += "\n};\n";
    return classData;
}

std::string Parser::transferClassForServer(const std::vector<ScannerToken>& tokens, int start, int end)
{
    std::vector<ServerFuncResult> funcResVec;
    std::string objName = tokens[start+1].value;

    // 分析函数结果
    int cur = start + 3;
    while (cur < end)
    {
        int r1 = findToken(tokens, cur, TOKEN_SEMICOLON);
        auto funRes = transferFuncForServer(tokens, cur, r1 - 1, objName);
        // 压入结果中存储
        funcResVec.push_back(funRes);
        
        cur = r1 + 1;
    }
    
    // 初始化相关类信息
    std::string classData;
    classData += "class " + objName + " : public HandleBase\n";
    classData += "{\npublic:\n";
    
    classData += "~" + objName + "() {}\n";      // 析构函数

    classData += "std::string getObjectName() const override\n{";  // 添加getObjectName函数
    classData += "\treturn \"" + objName + "\";\n}\n";  

    // 添加默认的服务端代码， 若用户没有重写， 默认抛出异常
    for (auto& x : funcResVec)
    {
        classData += "\n" + x._funcCode + "\n";
    }

    // 添加最重要的分发函数
    classData += "\nHrpc_Buffer distribute(const std::string& funcName, Hrpc_Buffer&& msg) override\n";
    classData += "{\n";
    // 添加一个重要的静态变量
    classData += "\tstatic std::string __funcContainer[] = {";
    // 添加函数名称
    for (int i = 0; i < funcResVec.size(); i++)
    {
        if (i == 0)
        {
            classData += "\"" + funcResVec[i]._funcName + "\"";
        }
        else 
        {
            classData += ", \"" + funcResVec[i]._funcName + "\"";
        }
    }
    classData += "};\n";
    classData += "\tauto itr = std::find(std::begin(__funcContainer), std::end(__funcContainer), funcName);\n";

    classData += "\tif (itr != std::end(__funcContainer))\n";
    classData += "\t{\n";

    // 添加具体实现
    classData += "\t\tauto diff = itr - std::begin(__funcContainer);\n";
    classData += "\t\tHrpc_SerializeStream s;\n";
    classData += "\t\ts.setBuffer(std::move(msg));\n";
    // 添加switch的具体代码
    classData += gernarateSwitchCode(funcResVec, "\t\t");

    classData += "\t}\n";
    classData += "\telse\n";
    classData += "\t{\n";
    // 错误情况，抛出异常
    classData += "\t\tthrow Hrpc_Exception(\"[" + objName + "::distribute]: unknow funcName\");\n";

    classData += "\t}\n";
    classData += "\treturn Hrpc_Buffer();\n";
    classData += "}\n";

    // 返回生成的代码
    return classData;
}
void Parser::parseFuncBody(const std::vector<ScannerToken>& tokens, int start, int end, ParamType& retType, std::string& funcName, std::vector<FuncParam>& paraList)
{
    int st = start;

    st = loadParaType(tokens, st, retType);
    // 失败抛异常
    if (st < 0)
    {
        throw Hrpc_Exception("[Parser::parseFuncBody]: occur error, pos = " + Hrpc_Common::tostr(start));
    }

    if (tokens[st].type != TOKEN_ID)
    {
        throw Hrpc_Exception("[Parser::parseFuncBody]: occur error, pos = " + Hrpc_Common::tostr(start));
    }
    funcName = tokens[st].value;
    st++;
    
    // 判断是否遇到(
    if (tokens[st].type != TOKEN_BRA_1_OP)
    {
        throw Hrpc_Exception("[Parser::parseFuncBody]: occur error, pos = " + Hrpc_Common::tostr(start));
    }
    st++;
    int listStart = st;
    // 寻找函数参数闭合处
    st = findToken(tokens, st, TOKEN_BRA_1_CL);
    if (st < 0)
    {
        throw Hrpc_Exception("[Parser::parseFuncBody]: occur error, pos = " + Hrpc_Common::tostr(start));
    }
    int listEnd = st - 1;
    // 获取参数列表
    paraList = transferParaList(tokens, listStart, listEnd);
    st++;
    // 下一个词素一定是函数结尾
    if (tokens[st].type != TOKEN_SEMICOLON)
    {
        throw Hrpc_Exception("[Parser::parseFuncBody]: occur error, pos = " + Hrpc_Common::tostr(start));
    }
}

std::string Parser::transferFuncForClient(const std::vector<ScannerToken>& tokens, int start, int end, const std::string& objName)
{
    // 读取头部返回值
    ParamType retType;
    std::string funcName;
    std::string resData;
    std::vector<FuncParam> paraList;

    // 对 函数体 进行解析
    parseFuncBody(tokens, start, end, retType, funcName, paraList);
    
    // 开始函数拼装
    return gernarateFunctionClient(objName, funcName, retType, paraList);
}

ServerFuncResult Parser::transferFuncForServer(const std::vector<ScannerToken>& tokens, int start, int end, const std::string& objName)
{
    ServerFuncResult  retData;
    ParamType retType;
    std::string funcName;
    std::vector<FuncParam> paraList;
    
    // 对函数体解析
    parseFuncBody(tokens, start, end, retType, funcName, paraList);
    
    retData._funcName = funcName;   // 设置函数名称
    retData._funcCode = gernarateFunctionServer(objName, funcName, retType, paraList);  // 生成函数代码
    retData._switchCode = gernarateSwitchCase(funcName, retType, paraList, "\t\t\t\t");        // 生成switch代码中case代码
    return retData;
}

std::vector<FuncParam> Parser::transferParaList(const std::vector<ScannerToken>& tokens, int start, int end)
{
    if (end - start <= 0)
    {
        return std::vector<FuncParam>();
    }

    std::vector<FuncParam> retVec;
    // 读取参数
    int st = start;
    while (st <= end)
    {
        FuncParam fp;
        st = loadParameter(tokens, st, fp);
        retVec.push_back(fp);

        // 判断是否已经打了 参数列表尾部
        if (st == end + 1)
        {
            break;
        }
        // 判断是否遇到,
        if (tokens[st].type == TOKEN_COMMA)
        {
            st++;
        }
        else
        {
            throw Hrpc_Exception("[Parser::transferParaList]: occur error, pos = " + Hrpc_Common::tostr(start));
        }
        
    }
    return retVec;
}

bool Parser::isTypeToken(const ScannerToken& token)
{
    auto type = token.type;
    if (type == TOKEN_BOOL || type == TOKEN_FLOAT || type == TOKEN_INT16 || type == TOKEN_INT32 || type == TOKEN_INT64
        || type == TOKEN_INT8 || type == TOKEN_MAP || type == TOKEN_STRING || type == TOKEN_VECTOR)
    {
        return true;
    }
    return false;
}

int Parser::loadParameter(const std::vector<ScannerToken>& tokens, int start, FuncParam& param)
{
    int st = start;
    
    // 检测是否有out
    if (tokens[st].type == TOKEN_OUT)
    {
        param._direct = 2;
        st++;
    }
    else
    {
        param._direct = 1;
    }
    
    
    // 读取类型
    ParamType type;
    st = loadParaType(tokens, st, type);
    if (st < 0)
    {
        throw Hrpc_Exception("[Parser::loadParamter]: occur error, pos = " + Hrpc_Common::tostr(start));
    }
    param._type = type;
    
    // 判断是否读取到id
    if (tokens[st].type != TOKEN_ID)
    {
        throw Hrpc_Exception("[Parser::loadParamter]: occur error, pos = " + Hrpc_Common::tostr(start));
    }
    param._name = tokens[st].value;

    return st + 1;
}

int Parser::loadParaType(const std::vector<ScannerToken>& tokens, int start, ParamType& param)
{
    if (isTypeToken(tokens[start]))
    {
        // 遇到vector
        if (tokens[start].type == TOKEN_VECTOR)
        {
            // 读取分隔符
            if (tokens[start+1].type != TOKEN_BRA_2_OP)
            {
                return -1;
            }
            
            ParamType type;
            int pos = loadParaType(tokens, start + 2, type);
            if (pos < 0)
            {
                return -1;
            }
            // 读取到类型
            if (tokens[pos].type != TOKEN_BRA_2_CL)
            {
                return -1;
            }
            
            // 设置返回参数
            param._tokenType = TOKEN_VECTOR;
            param._typeName = "std::vector<" + type._typeName + ">";

            return pos + 1;
        }
        else if (tokens[start].type == TOKEN_MAP)
        {
            // 遇到了map
            // 读取分隔符
            if (tokens[start+1].type != TOKEN_BRA_2_OP)
            {
                return -1;
            }
            int st = start + 2;
            ParamType type1, type2;
            // 读取第一个参数
            st = loadParaType(tokens, st, type1);
            if (st < 0)
            {
                return -1;
            }
            // map的第一个参数必须要是简单类型
            if (type1._tokenType == TOKEN_VECTOR || type1._tokenType == TOKEN_MAP)
            {
                return -1;
            }
            // 判断分隔符
            if (tokens[st].type != TOKEN_COMMA)
            {
                return -1;
            }
            st++;
            
            // 进行读取第二个参数
            st = loadParaType(tokens, st, type2);
            if (st < 0)
            {
                return -1;
            }
            // 判断是否map是否封闭
            if (tokens[st].type != TOKEN_BRA_2_CL)
            {
                return -1;
            }
            // 向后移动
            st++;
            // 封装参数
            param._tokenType = TOKEN_MAP;
            param._typeName = "std::map<" + type1._typeName + ", " + type2._typeName + ">";
            return st;
        }
        else
        {
            // 普通的简单类型
            switch (tokens[start].type)
            {
                case TOKEN_INT8:
                {
                    param._tokenType = TOKEN_INT8;
                    param._typeName = "Hrpc::Int8";
                    break;
                }
                case TOKEN_INT16:
                {
                    param._tokenType = TOKEN_INT16;
                    param._typeName = "Hrpc::Int16";
                    break;
                }
                case TOKEN_INT32:
                {
                    param._tokenType = TOKEN_INT32;
                    param._typeName = "Hrpc::Int32";
                    break;
                }
                case TOKEN_INT64:
                {
                    param._tokenType = TOKEN_INT64;
                    param._typeName = "Hrpc::Int64";
                    break;
                }
                case TOKEN_BOOL:
                {
                    param._tokenType = TOKEN_BOOL;
                    param._typeName = "bool";
                    break;
                }
                case TOKEN_FLOAT:
                {
                    param._tokenType = TOKEN_FLOAT;
                    param._typeName = "float";
                    break;
                }
                case TOKEN_STRING:
                {
                    param._tokenType = TOKEN_STRING;
                    param._typeName = "std::string";
                    break;
                }
                default:
                    return -1;
                
            };
            return start + 1;
        }
        
    }
    else
    {
        // 失败
        return -1;
    }
    return -1;
}

std::string Parser::parse(const std::vector<ScannerToken>& tokens)
{
    int cur = 0;
    while (cur < tokens.size())
    {
        int pos = findToken(tokens, cur, TOKEN_CLASS);
        if (pos < 0)
        {
            break;
        }
        if (!(tokens[pos + 1].type == TOKEN_ID && tokens[pos + 2].type == TOKEN_BRA_3_OP))
        {
            throw Hrpc_Exception("[Parser::parse]: class error, pos = " + Hrpc_Common::tostr(pos));
        }

        int pos2 = findToken(tokens, pos, TOKEN_BRA_3_CL);
        if (pos2 < 0)
        {
            throw Hrpc_Exception("[Parser::parse]: class error, pos = " + Hrpc_Common::tostr(pos));
        }
        
        std::string data = transferClass(tokens, pos, pos2);
        std::cout << data << std::endl;

        cur = pos2;
    }
    return "";
}

}