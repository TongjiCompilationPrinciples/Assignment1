//
// Created by Mac book pro on 2023/12/26.
//

#ifndef COMPILING_ANA_H
#define COMPILING_ANA_H
#include<cstring>
#include<string>
#include"Deceleration.h"
#include<cctype>
#include<iostream>
namespace LexAna{
    static const char* keyWordTable[]={
            "PROGRAM", "BEGIN", "END", "CONST", "VAR",
            "WHILE", "DO", "IF", "THEN"
    };
    class Ana{
    public: explicit Ana(FILE *fp);
        explicit Ana(const std::string& fileName);
        explicit Ana()= default;
        Word getWord();
        Word peekWord();
        Word peekWord(int n);
        void setfp(const std::string& fileName);
        void reset();// 重置词法分析器，将文件指针重置到文件开头
    private:
        bool consPattern=false; // 是否通过传递文件指针的方式初始化
        FILE *fp{};
        int row{};
        int col{};
        int pos{};
        static bool isKeyWord(const char *word);

    };
}



#endif //COMPILING_ANA_H
