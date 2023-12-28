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
        Word getWord();
    private:
        FILE *fp;
        int row;
        int col;
        int pos;
        static bool isKeyWord(const char *word);

    };
}



#endif //COMPILING_ANA_H
