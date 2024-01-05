//
// Created by Mac book pro on 2024/1/3.
//

#ifndef COMPILING_GRAMMERANA_H
#define COMPILING_GRAMMERANA_H
#include<string>
#include"Ana.h"
#include<vector>
#include<set>
#include <cstdio>
namespace GrammerAna{
    class GraAna{
    private:
        void program();                             // <程序> -> PROGRAM<分程序>
        tCodeItem expression();                     // <表达式> -> [+|-]<项>{<加法运算符><项>}（生成中间变量 + 中间代码）
        tCodeItem term();                           // <项> -> <因子>{<乘法运算符><因子>}：产生中间代码 + 一个中间变量
        tCodeItem factor();                         // <因子> -> <标识符>|<无符号整数>|(<表达式>)：产生一个中间变量
        void getOpValue(tCodeItem&, tCodeItem&, long&, long&); // 传入两个符号，返回其对应的数值
        long genTmp(long);                          // 生成中间变量，并统一命名 tmpi（i为中间变量序号）,并返回其符号表索引
        long position(char*);                       // 从符号表中顺序查找对应的变量/常量
        void condition();                           // <条件> -> <表达式><关系运算符><表达式>：产生一个 条件goto，待上层程序回填
        bool statement();                           // <语句> -> <赋值语句>|<条件语句>|<循环语句>|<复合语句>|空（需要特别判断是否推空）
        void block();                               // <分程序> -> [常量说明][变量说明]<语句>
        void varDeclaration();                      // <标识符>：将对应变量登记到符号表（需要报错）
        void constDeclaration();                    // <常量定义> -> <标识符> := <无符号整数>：将对应常量登记到符号表（需要报错）
        void gen(FCT, tCodeItem, tCodeItem, tCodeItem); // 生成一条中间代码并存入表中
        void getNextWord();                         // 获取下一个单词
        void error(const std::string& msg="");      // 报错
        static bool validId(const std::string& id); // 判断标识符是否合法
        void init();                                // 初始化
    public:
        const std::vector<std::string>fctStrings={
                "useless",
                "ADD",          // +
                "SUB",          // -
                "MUL",          // *
                "DIV",          // /
                "BEC",          // :=
                "JEQL",         // =
                "JNEQ",         // <>
                "JGT",          // >
                "JGE",          // >=
                "JLT",          // <
                "JLE",          // <=
                "JUP",          // jump
        };
        explicit GraAna(const std::string&sourceCode,const std::string& outputFileName);
        void printSymTable();
        void printTCode_c(tCode& code); // 打印中间代码到控制台
        void printTCode_f(tCode& code); // 打印中间代码到文件
        void printTCode(int opt); // 打印中间代码到文件或控制台, opt=0:控制台，opt=1:文件
        void start();                               // 开始语法分析
    private:
        FILE *fp;                                   // 输出文件指针
        std::string outputFileName;                 // 输出文件名
        LexAna::Ana ana;                            // 词法分析器
        Word sym;                                   // 当前即将要解析的非终结符
        static const long symMax = 2000;            // 符号表最大符号记录个数
        static const long tcMax = 2000;             // 中间代码表最大代码记录条数

        SymTableItem symTable[symMax]{};            //存放已经定义的变量/常量/中间变量的符号表
        tCode transitionalCodes[tcMax]{};           //存放已经生成的中间代码
        long totalDel = 0;                          // 代码中定义的 变量 + 常量 的个数（用于符号表搜索）
        long nextTmp = 0;                           // 下一个中间变量的序号
        long nextSym = 0;                           // 即将存放符号的符号表索引
        long nextTCode = 0;                         // 即将存放中间代码的中间代码表索引
        std::set<std::string> IDs;                  // 用作标识符查重
    };
}
#endif //COMPILING_GRAMMERANA_H
