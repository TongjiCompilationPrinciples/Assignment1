#include <iostream>
#include <vector>
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
FILE *fp=fopen("../test.txt", "r");
Ana lexAna(fp);

Word sym;   // 当前即将要解析的非终结符
const long symMax = 2000;
const long tcMax = 2000;
SymTableItem symTable[symMax];      //存放已经定义的变量/常量/中间变量的符号表
tCode transitionalCodes[tcMax];     //存放已经生成的中间代码
long nextSym = 0;                   //即将存放符号的符号表索引
long nextTCode = 0;                 //即将存放中间代码的中间代码表索引

char* fctString[] = {
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
};

void error() {
    std::cout <<"非法输入！" << std::endl;
    abort();
}

// <程序> -> PROGRAME<分程序>
void programe() {
    if(sym.type == KEYWORD && sym.val.k == PROGRAM) {
        // 程序开头必须是关键词语“PROGRAME”
        sym = lexAna.getWord();

        // 进入<分程序>
        block();

    } else {
        error();
    }
}

// <常量定义> -> <标识符> := <无符号整数>：将对应常量登记到符号表（需要报错）
void constDeclaration() {
    // 不是标识符直接报错
    if(sym.type != IDENTIFIER) {
        error();
    }

    // 记录在符号表中
    symTable[nextSym].type = constant;
    strcpy(symTable[nextSym].name, sym.val.s);

    sym = lexAna.getWord();
    // 标识符后必须为赋值符号
    if(sym.type != OPERATOR) {
        error();
    } else if(sym.val.o != ASSIGN) {
        error();
    }

    // 赋值后必须为<无符号整型>
    sym = lexAna.getWord();
    if(sym.type != INTEGER) {
        error();
    }
    symTable[nextSym].value = sym.val.i;
    nextSym++;
    
    sym = lexAna.getWord();
}

// <标识符>：将对应变量登记到符号表（需要报错）
void varDeclaration() {
    // 不是标识符直接报错
    if(sym.type != IDENTIFIER) {
        error();
    }

    // 记录在符号表中
    symTable[nextSym].type = variable;
    strcpy(symTable[nextSym].name, sym.val.s);

    sym = lexAna.getWord();
    // 标识符后必须为赋值符号
    if(sym.type != OPERATOR) {
        error();
    } else if(sym.val.o != ASSIGN) {
        error();
    }

    sym = lexAna.getWord();
}

// <分程序> -> [常量说明][变量说明]<语句>
void block() {
    // 如果有常量定义
    if(sym.type == KEYWORD && sym.val.k == CONST) {
        sym = lexAna.getWord();
        // 这里同样留给常量定义去报错
        constDeclaration();

        // 如果后面为“逗号”则循环定义常量
        while(sym.type == OPERATOR && sym.val.o == COMMA) {
            constDeclaration();
        }

        // 如果不是分号则报错
        if(sym.type != OPERATOR) {
            error();
        } else if(sym.val.o != SEMI) {
            error();
        }
    } 

    // 如果有变量定义
    if(sym.type == KEYWORD && sym.val.k == VAR) {
        sym = lexAna.getWord();

        varDeclaration();

        // 如果为“逗号”则循环定义
        while(sym.type == OPERATOR && sym.val.o == COMMA) {
            varDeclaration();
        }
    }

    // 无需报错，直接进入；在<语句>翻译中再进行报错
    sym = lexAna.getWord();
    statement();

}

// <语句> -> <赋值语句>|<条件语句>|<循环语句>|<复合语句>|空（需要报错）
void statement() {




    sym = lexAna.getWord();
}

// <条件> -> <表达式><关系运算符><表达式>：产生一个 条件goto，待上层程序回填
void condition() {
    //直接进入<表达式>，不检查
    expression();

    // 必须关系运算符
    if(sym.type != OPERATOR) {
        error();
    } else if(sym.val.o != )

    


    sym = lexAna.getWord();
}

// <因子> -> <标识符>|<无符号整数>|(<表达式>)：产生一个中间变量
void factor() {



    sym = lexAna.getWord();
}

// <项> -> <因子>{<乘法运算符><因子>}：产生中间代码 + 一个中间变量
void term() {




    sym = lexAna.getWord();
}

// <表达式> -> <>
void expression() {
    Word addop;

    // 处理正负号（可选）
    if(sym.type == OPERATOR && (sym.val.o == MIN || sym.val.o == PLUS)) {
        addop=sym;                 // 保存正负号
        sym = lexAna.getWord();

        term();     // 正负号后面是一个term

        // TODO:此处要生成中间代码......


    }
    // 若开头无加法运算符，则直接进入“项”递归下降函数
    else {
        // 报错交给<项>
        term();
    }

    // 若后面仍有“加法运算符”连接的表达式
    while(sym.type == OPERATOR && (sym.val.o == MIN || sym.val.o == PLUS))
    {
        addop=sym;                 // 保存运算符
        sym = lexAna.getWord();

        term();     // 运算符后是一个term

        // TODO:此处待写中间代码生成
    }
}

// 将中间代码打印到控制台
void testTCode(tCode code) {
    std::cout<<"("<<fctString[code.fct]<<", "<<symTable[code.p1].name<<", "<<symTable[code.p2].name<<", "<<symTable[code.res].name<<std::endl;    
}


int main() {

    sym = lexAna.getWord();
    // 语法分析
    programe();

    return 0;
}