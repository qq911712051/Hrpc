#include <iostream>

#include <scanner.h>

namespace Hrpc
{

bool Scanner::isWhite(char ch)
{
    if (ch == '\n' || ch == ' ' || ch == '\t' || ch == '\r')
        return true;
    return false;
}

int Scanner::isSem(char ch)
{
    switch (ch)
    {
        case ',':
            return TOKEN_COMMA;
        case ';':
            return TOKEN_SEMICOLON;
        case '(':
            return TOKEN_BRA_1_OP;
        case ')':
            return TOKEN_BRA_1_CL;
        case '<':
            return TOKEN_BRA_2_OP;
        case '>':
            return TOKEN_BRA_2_CL;
        case '{':
            return TOKEN_BRA_3_OP;
        case '}':
            return TOKEN_BRA_3_CL;
        default:
            return -1;
    }
    return -1;
}


int Scanner::isKeyWord(const std::string& data)
{
    if (data == "class")
    {
        return TOKEN_CLASS;
    }
    else if (data == "namespace")
    {
        return TOKEN_NAMESPACE;
    }
    else if (data == "Int8")
    {
        return TOKEN_INT8;
    }
    else if (data == "Int16")
    {
        return TOKEN_INT16;
    }
    else if (data == "Int32")
    {
        return TOKEN_INT32;
    }
    else if (data == "Int64")
    {
        return TOKEN_INT64;
    }
    else if (data == "map")
    {
        return TOKEN_MAP;
    }
    else if (data == "vector")
    {
        return TOKEN_VECTOR;
    }
    else if (data == "bool")
    {
        return TOKEN_BOOL;
    }
    else if (data == "string")
    {
        return TOKEN_STRING;
    }
    else if (data == "float")
    {
        return TOKEN_FLOAT;
    }
    else if (data == "out")
    {
        return TOKEN_OUT;
    }
    

    return -1;
}

std::string Scanner::toString(const ScannerToken& token)
{
    switch (token.type)
    {
    case TOKEN_NAMESPACE:
    {
        std::cout << "namespace : " << std::endl;
        break;
    }
    case TOKEN_CLASS:
    {
        std::cout << "class : " << std::endl;
        break;
    }
    case TOKEN_ID:
    {
        std::cout << "id : " << token.value << std::endl;
        break;
    }
    case TOKEN_INT8:
    {
        std::cout << "Int8 : " << std::endl;
        break;
    }
    case TOKEN_INT16:
    {
        std::cout << "Int16 : " << std::endl;
        break;
    }
    case TOKEN_INT32:
    {
        std::cout << "Int32 : " << std::endl;
        break;
    }
    case TOKEN_INT64:
    {
        std::cout << "Int64 : " << std::endl;
        break;
    }
    case TOKEN_BOOL:
    {
        std::cout << "bool : " << std::endl;
        break;
    }
    case TOKEN_FLOAT:
    {
        std::cout << "float : " << std::endl;
        break;
    }
    case TOKEN_STRING:
    {
        std::cout << "string : " << std::endl;
        break;
    }
    case TOKEN_VECTOR:
    {
        std::cout << "vector : " << std::endl;
        break;
    }
    case TOKEN_MAP:
    {
        std::cout << "map : " << std::endl;
        break;
    }
    case TOKEN_OUT:
    {
        std::cout << "out : " << std::endl;
        break;
    }
    case TOKEN_COMMA:
    {
        std::cout << ", : " << std::endl;
        break;
    }
    case TOKEN_SEMICOLON:
    {
        std::cout << "; : " << std::endl;
        break;
    }
    case TOKEN_BRA_1_OP:
    {
        std::cout << "( : " << std::endl;
        break;
    }
    case TOKEN_BRA_1_CL:
    {
        std::cout << ") : " << std::endl;
        break;
    }
    case TOKEN_BRA_2_OP:
    {
        std::cout << "< : " << std::endl;
        break;
    }
    case TOKEN_BRA_2_CL:
    {
        std::cout << "> : " << std::endl;
        break;
    }
    case TOKEN_BRA_3_OP:
    {
        std::cout << "{ : " << std::endl;
        break;
    }
    case TOKEN_BRA_3_CL:
    {
        std::cout << "} : " << std::endl;
        break;
    }
    default:
        std::cout << "error " << std::endl;
        break;
    }
}


std::vector<ScannerToken> Scanner::scan(const std::string& ori)
{
    std::string data = ori;
    data.push_back(' ');
    std::vector<ScannerToken> retVec;

    size_t cur = 0;
    int state = 0;
    std::string value = "";
    while (cur < data.size())
    {
        char ch = data[cur];

        if (state == 0)
        {
            if (isWhite(ch))
            {
                cur++;
                continue;
            }
            
            int r1 = isSem(ch);
            if (r1 > 0)
            {
                // 分隔符
                retVec.push_back(ScannerToken(r1));
                cur++;
                continue;
            }
            // 分析是否其他情况
            state = 1;
            value += ch;
            cur++;
        }
        else if (state == 1)
        {
            if (isWhite(ch) || isSem(ch) > 0)
            {
                // 得到一个id或者关键字
                int r1 = isKeyWord(value);
                if (r1 < 0)
                {
                    // id
                    retVec.push_back(ScannerToken(TOKEN_ID, value));
                }
                else
                {
                    // 关键字
                    retVec.push_back(ScannerToken(r1));
                }

                int type = isSem(ch);
                if (type > 0)
                {
                    retVec.push_back(ScannerToken(type));
                }

                value = "";
                cur++;
                state = 0;
                continue;
            }
            else
            {
                value += ch;
                cur++;
            }
            
        }
    }
    return retVec;
}
}