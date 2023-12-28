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
        PROGRAM=1, // program
        BEGIN=2, // begin
        END=3, // end
        CONST=4, // const
        VAR=5, // var
        WHILE=6, // while
        DO=7, // do
        IF=8, // if
        THEN=9 // then
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
}

#endif //COMPILING_DECELERATION_H
