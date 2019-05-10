#ifndef SCANNER_H_
#define SCANNER_H_

#include <string>
#include <vector>
namespace Hrpc
{
enum 
{
    TOKEN_NAMESPACE = 1, // namespace 关键字
    TOKEN_CLASS = 2,         // class 关键字
    TOKEN_ID = 3,            // 变量名
    TOKEN_INT8 = 4,          // int8
    TOKEN_INT16 = 5,          // int16
    TOKEN_INT32 = 6,          // int32
    TOKEN_INT64 = 7,          // int64
    TOKEN_BOOL = 8,          // bool
    TOKEN_STRING = 9,       // string
    TOKEN_FLOAT = 10,       // float
    TOKEN_VECTOR = 11,       // vector
    TOKEN_MAP = 12,       // map
    TOKEN_OUT = 13,      // out标识符
    
    // 下面主要是一些分隔符
    TOKEN_COMMA =14,    // 逗号
    TOKEN_SEMICOLON = 15,    // 分号
    TOKEN_BRA_1_OP = 16,     // (
    TOKEN_BRA_1_CL = 17,     // )
    TOKEN_BRA_2_OP = 18,     // <
    TOKEN_BRA_2_CL = 19,     // >
    TOKEN_BRA_3_OP = 20,     // {
    TOKEN_BRA_3_CL = 21,     // }
};
struct ScannerToken
{
    ScannerToken(int type, const std::string& value = "") : type(type), value(value){}

    int             type;   // 词素的类型
    std::string     value;  // 具体的值
};

class Scanner
{
public:
    /**
     * @description: 扫描数据， 获取所有的token
     * @param {type} 
     * @return: 
     */
    static std::vector<ScannerToken> scan(const std::string& data);

    /**
     * @description: 是否是空白字符
     * @param {type} 
     * @return: 
     */
    static bool isWhite(char ch);

    /**
     * @description: 是否是分隔符
     * @param {type} 
     * @return: 
     */
    static int isSem(char ch);

    /**
     * @description: 是否关键字
     * @param {type} 
     * @return: 
     */
    static int isKeyWord(const std::string& data);

    static std::string toString(const ScannerToken& token);
};
}
#endif