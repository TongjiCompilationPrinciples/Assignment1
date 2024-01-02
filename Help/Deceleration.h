//
// Created by Mac book pro on 2023/12/28.
//

#ifndef COMPILING_DECELERATION_H
#define COMPILING_DECELERATION_H
#include<string>

#define MAX_WORD_LEN 20
namespace LexAna{
    typedef enum{
        KEYWORD=1, // 关键字
        IDENTIFIER=2, // 标识符
        INTEGER=3, // 整数
        OPERATOR=4, // 运算符或界符
        ERROR=5, // 错误
        _END=6 // 文件结束
    }Type;

    typedef enum{
        PROGRAM=0, // program
        BEGIN=1, // begin
        END=2, // end
        CONST=3, // const
        VAR=4, // var
        WHILE=5, // while
        DO=6, // do
        IF=7, // if
        THEN=8 // then
    }KEY; // 关键字的具体类型

    typedef enum{
        PLUS=1, // +
        MIN=2, // -
        MUL=3, // *
        DIV=4, // /
        ASSIGN=5, // :=
        EQ=6, // =
        NEQ=7, // <>
        GT=8, // >
        GE=9, // >=
        LT=10, // <
        LE=11, // <=
        LP=12, // (
        RP=13, // )
        COMMA=14, // ,
        SEMI=15, // ;
        PERIOD=16, // .
        COLON=17 // :
    }OPorDEL; // 运算符或界符的具体类型

    union Val{
        long i; // 整数
        char s[20]; // 标识符
        KEY k; // 关键字
        OPorDEL o; // 运算符或界符
        char msg[20]; // 错误信息
    };
    struct Word{
        Type type; // 单词所属类型
        long row; // 行号
        long col; // 列号
        long pos; // 文件中的位置
        Val val{} ; //
        Word(){
            row=col=pos=0; // 默认为0
            type=ERROR; // 默认为错误
        }
    };

    enum class FCT{
        ADD=1, // +
        SUB=2, // -
        MUL=3, // *
        DIV=4, // /
        BEC=5, // :=
        JEQ=6, // =
        JNEQ=7, // <>
        JGT=8, // >
        JGE=9, // >=
        JLT=10, // <
        JLE=11, // <=
        JUP=12, // 无条件jump
    }; //中间代码指令
    struct tCodeItem{   //中间代码操作数（可为直接数/标识符）
        unsigned char type;      //-1为空 0为立即数，1为标识符/中间变量
        long value;
    };
    struct tCode{
        FCT fct;    //指令
        tCodeItem p1;    //操作数1
        tCodeItem p2;    //操作数2
        tCodeItem res;   //结果数
    };

    typedef enum{
        constant=0, //常量
        variable=1, //变量
        interVar=2, //中间变量
    }ItemType;
    struct SymTableItem{
        char name[20];      // 标识符
        ItemType type;      //变量类型
        long value;         //值
    };
    
}

#endif //COMPILING_DECELERATION_H
