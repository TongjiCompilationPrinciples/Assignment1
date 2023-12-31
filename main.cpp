#include <iostream>
#include"Help/Ana.h"
using namespace LexAna;
int test() {
    std::cout << "====== LexAnx Test Begin ======" << std::endl;
    FILE *fp=fopen("../test.txt", "r");
    if(!fp){
        std::cout<<"文件打开失败"<<std::endl;
        exit(1);
    }
    Ana lexAna(fp);
    Word word;
    while((word=lexAna.getWord()).type!=_END){
        std::cout << "row: " << word.row << " col: " << word.col << " pos: " << word.pos<<std::endl;
        switch (word.type){
            case KEYWORD:
                std::cout << "KEYWORD: " << keyWordTable[word.val.k] << std::endl;
                break;
            case IDENTIFIER:
                std::cout << "IDENTIFIER: " << word.val.s << std::endl;
                break;
            case INTEGER:
                std::cout << "INTEGER: " << word.val.i << std::endl;
                break;
            case OPERATOR:
                std::cout << "OPERATOR: " << word.val.o << std::endl;
                break;
            case ERROR:
                std::cout << "ERROR: " << word.val.msg<<std::endl;
                break;
            default:
                std::cout << "ERROR: " <<word.val.msg <<std::endl;
                break;
        }
        std::cout<<std::endl;
    }
    std::cout << "====== LexAnx Test End ======" << std::endl;
}


// 下为语法解析测试
Word sym;   // 当前即将要解析的非终结符


// <常量定义> -> <标识符> := <无符号整数>
void constDeclaration() {

}

// <标识符>{, <标识符>}
void carDeclaration() {

}

// <分程序> -> [][]<语句>
void block() {

}

// <语句> -> <赋值语句>|<条件语句>|<循环语句>|<复合语句>|空
void statement() {

}

// <条件> -> <表达式><关系运算符><表达式>
void condition() {

}

// <表达式> -> <>
void expression() {
    unsigned long addop;

    if(sym==plus || sym==minus)    // 处理正负号（可选）
    {
        addop=sym;                 // 保存正负号
        getsym();

        term(fsys|plus|minus);     // 正负号后面是一个term

        if(addop==minus)
        {
            gen(opr,0,1);          // 负号，取反运算
        }
    }
    else	// 若开头无加法运算符，则直接进入“项”递归下降函数
    {
        term(fsys|plus|minus);
    }

    // 若后面仍有“加法运算符”连接的表达式
    while(sym==plus || sym==minus) // 处理加减
    {
        addop=sym;                 // 保存运算符
        getsym();

        term(fsys|plus|minus);     // 运算符后是一个term

        if(addop==plus)
        {
            gen(opr,0,2);          // 加
        }
        else
        {
            gen(opr,0,3);          // 减
        }
    }
}


int main() {
        FILE *fp=fopen("../test.txt", "r");
    if(!fp){
        std::cout<<"文件打开失败"<<std::endl;
        exit(1);
    }



}