#include <iostream>
#include <string>
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
long totalDel = 0;                  //代码中定义的 变量 + 常量 的个数（用于符号表搜索）
long nextTmp = 0;                   //下一个中间变量的序号                   
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

// 生成一条中间代码并存入表中
void gen(FCT op, tCodeItem p1, tCodeItem p2, tCodeItem res) {
    transitionalCodes[nextTCode].fct = op;
    transitionalCodes[nextTCode].p1 = p1;
    transitionalCodes[nextTCode].p2 = p2;
    transitionalCodes[nextTCode].res = res;
    nextTCode++;
}

// 将中间代码打印到控制台
void printTCode(tCode& code) {
    std::cout<<"("<<fctString[static_cast<int>(code.fct)]<<", ";

    // 判断操作数是 标识符 还是 直接数
    if(code.p1.type) {
        std::cout<<symTable[code.p1.value].name;
    } else {
        std::cout<<code.p1.value;
    }
    std::cout<<", ";
    
    if(code.p2.type == (unsigned char)1) {
        std::cout<<symTable[code.p2.value].name;
    } else if(code.p2.type == (unsigned char)0){
        std::cout<<code.p2.value;
    } else {
        std::cout<<'-';
    }
    std::cout<<", ";
    
    if(code.res.type) {
        std::cout<<symTable[code.res.value].name;
    } else {
        std::cout<<code.res.value;
    }
    std::cout<<")"<<std::endl;
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
    totalDel++;
    
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
    // 变量初始化 -1
    symTable[nextSym].value = -1;

    nextSym++;
    totalDel++;

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
    statement();
}

// <语句> -> <赋值语句>|<条件语句>|<循环语句>|<复合语句>|空（需要特别判断是否推空）
bool statement() {
    bool isNull = true;

    // 赋值语句
    if(sym.type == IDENTIFIER) {
        // 寻找对应标识符的变量索引
        long resIndex = position(sym.val.s);

        //标识符未定义
        if(resIndex == -1) {
            error();
        }

        sym = lexAna.getWord();

        // 标识符后必须有 赋值号
        if(sym.type != OPERATOR) {
            error();
        } else if(sym.val.o != ASSIGN) {
            error();
        }
        Word assignop = sym;

        // <表达式>翻译
        sym = lexAna.getWord();
        expression();

        // 记录<表达式>结果的对应中间变量索引，并生成中间代码：{resIndex} = {tmp1Index}
        long tmp1Index = nextSym - 1;
        gen(FCT(assignop.val.o), {1, tmp1Index}, {(unsigned char)-1, -1}, {1, resIndex});
    
        // 没有推空
        isNull = false;
    }
    else if(sym.type == KEYWORD) {
        // 条件语句
        if(sym.val.k == IF) {
            // <条件>解析
            sym = lexAna.getWord();
            condition();

            // 记录要回填的jump中间指令索引
            long jumpCodeIndex = nextTCode - 1;

            // 后面必须有 then
            if(sym.type != KEYWORD) {
                error();
            } else if(sym.val.k != THEN) {
                error();
            }

            // <语句>解析
            sym = lexAna.getWord();
            statement();

            // 回填<条件>的假出口
            transitionalCodes[jumpCodeIndex].res.value = nextTCode;

            isNull = false;
        }
        // 循环语句
        else if(sym.val.k == WHILE) {
            // 记录循环跳转
            long whileJumpIndex = nextTCode;

            sym = lexAna.getWord();
            // 解析<条件>，并记录假出口跳转的回填索引
            condition();
            long jumpCodeIndex = nextTCode - 1;

            // 循环语句必须有 DO
            if(sym.type != KEYWORD) {
                error();
            } else if(sym.val.k != DO) {
                error();
            }

            // <语句>解析
            sym = lexAna.getWord();
            statement();

            isNull = false;
        }
        // 复合语句!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!(TODO：待商讨)
        else if(sym.val.k == BEGIN) {
            sym = lexAna.getWord();
            
            // 循环解析<语句>
            while(1) {
                if(sym.type == KEYWORD && sym.val.k == END) {
                    break;
                }

                // 报错留给<语句>
                if(statement()) {

                    // 若推空了则报错，因为复合语句中至少有一条语句
                    error();
                }
            }

            isNull = false;
        }
    }

    //不报错，因为可以推<空>
    sym = lexAna.getWord();
    return isNull;
}

// <条件> -> <表达式><关系运算符><表达式>：产生一个 条件goto，待上层程序回填
void condition() {
    //直接进入<表达式>，不检查
    expression();
    // 记录<表达式>生成的中间变量
    long tmp1Index = nextSym - 1;

    // 必须关系运算符
    if(sym.type != OPERATOR) {
        error();
    } else {
        // 暂存关系运算符号
        Word relop = sym;

        sym = lexAna.getWord();

        // 进入右边 <表达式> 解析
        expression();
        // 记录右边<表达式>生成的中间变量
        long tmp2Index = nextSym - 1;

        // 根据对应关系运算符生成中间代码
        switch (relop.val.o)
        {
            // 根据<关系运算符>反着填入
            case EQ:
                gen(FCT::JNEQ, {1, tmp1Index}, {1, tmp2Index}, {0, 0});
                break;
            case NEQ:
                gen(FCT::JEQ, {1, tmp1Index}, {1, tmp2Index}, {0, 0});
                break;
            case GT:
                gen(FCT::JLE, {1, tmp1Index}, {1, tmp2Index}, {0, 0});
                break;
            case GE:
                gen(FCT::JLT, {1, tmp1Index}, {1, tmp2Index}, {0, 0});
                break;
            case LT:
                gen(FCT::JGE, {1, tmp1Index}, {1, tmp2Index}, {0, 0});
                break;
            case LE:
                //  暂时不填入跳转的地址，等待上层递归回填（这里要反着填？？）
                gen(FCT::JGT, {1, tmp1Index}, {1, tmp2Index}, {0, 0});
                break;
            default:
                // 不是任何关系运算符则报错
                error();
                break;
        }
    }

    


    sym = lexAna.getWord();
}

// 从符号表中顺序查找对应的变量/常量
long position(char* symName) {
    // 如果变量名为空 或 null则返回 -1
    if(symName == NULL || strlen(symName) == 0) {
        return -1;
    }

    // 按顺序从上到下遍历符号表寻找
    for(int i = 0; i < totalDel; i++) {

        // 发现对应定义则返回 索引
        if(!strcmp(symTable[i].name, symName)) {
            return i;
        }
    }

    // 未定义则返回 -1
    return -1;
}

// 生成中间变量，并统一命名 tmpi（i为中间变量序号）,并返回其符号表索引
long genTmp(long value) {
    std::string tmpName = "tmp" + std::to_string(nextTmp);
    strcpy(symTable[nextSym].name, (char*)tmpName.c_str());
    symTable[nextSym].type = interVar;
    symTable[nextSym].value = value;
        
    nextSym++;
    nextTmp++;

    return nextSym - 1;
}

// <因子> -> <标识符>|<无符号整数>|(<表达式>)：产生一个中间变量
void factor() {
    // 标识符
    if(sym.type == IDENTIFIER) {
        // 从开头顺序遍历查找符号表
        long symIndex = position(sym.val.s);

        // 标识符未定义
        if(symIndex == -1) {
            error();
        }

        // 生成中间变量
        genTmp(symTable[symIndex].value);
    }
    // 无符号整数
    else if(sym.type == INTEGER) {
        long value = sym.val.i;

        // 生成中间变量
        genTmp(value);
    }
    // （表达式）
    else if(sym.type == OPERATOR && sym.val.k == LP) {
        sym = lexAna.getWord();

        // 进入<表达式>语法分析
        expression();

        // 表达式之后必须有右括号
        if(sym.type != OPERATOR) {
            error();
        } else if(sym.val.k != RP) {
            error();
        }
    }
    // 上述都不是则报错
    else {
        error();
    }

    sym = lexAna.getWord();
}

// <项> -> <因子>{<乘法运算符><因子>}：产生中间代码 + 一个中间变量
void term() {
    //直接进入 <因子> 解析，并记录存放结果的中间变量
    Word mulop;
    factor();
    long resIndex = nextSym - 1;

    // 有<乘法运算符>则继续进行解析
    while(sym.type == OPERATOR && (sym.val.o == MUL || sym.val.o == DIV)) {
        mulop = sym;

        sym = lexAna.getWord();

        factor();

        // 记录存放<因子>结果的中间变量索引
        long tmp1Index = nextSym - 1;
        long tmp2Index = resIndex;

        // 生成存放结果的中间变量，并生成对应的中间代码
        resIndex = genTmp(-1);
        gen((FCT)mulop.val.o, {1, tmp1Index}, {1, tmp2Index}, {1, resIndex});

        sym = lexAna.getWord();
    }
}

// <表达式> -> [-|+]<项>{<加法运算符><项>} （生成中间变量 + 中间代码）
void expression() {
    Word addop;
    long resIndex = -1; //存放上一次计算的中间变量的符号表索引

    // 处理正负号（可选）
    if(sym.type == OPERATOR && (sym.val.o == MIN || sym.val.o == PLUS)) {
        addop=sym;                 // 保存正负号
        sym = lexAna.getWord();

        term();     // 正负号后面是一个term

        // 记录<项>生成的中间变量
        long tmp1Index = nextSym - 1;

        // 生成一个中间变量存放结果，并生成一条中间代码进行运算并存储到该中间变量
        resIndex = genTmp(-1);
        gen((FCT)sym.val.o, {0, 0}, {1, tmp1Index}, {1, resIndex});    
    }
    // 若开头无加法运算符，则直接进入“项”递归下降函数
    else {
        // 报错交给<项>
        term();

        // 记录项产生的中间变量
        resIndex = nextSym - 1;
    }

    // 若后面仍有“加法运算符”连接的表达式
    while(sym.type == OPERATOR && (sym.val.o == MIN || sym.val.o == PLUS))
    {
        addop=sym;                 // 保存运算符
        sym = lexAna.getWord();

        term();     // 运算符后是一个term

        // 记录两个操作数的符号表索引
        long tmp1Index = nextSym - 1;
        long tmp2Index = resIndex;

        // 产生一个中间变量存放结果，产生一条中间代码
        resIndex = genTmp(-1);
        gen((FCT)sym.val.o, {1, tmp1Index}, {1, tmp2Index}, {1, resIndex});
    }
}

int main() {

    sym = lexAna.getWord();
    // 语法分析
    programe();

    for(int i = 0; i < nextTCode; i++) {
        printTCode(transitionalCodes[i]);
    }

    return 0;
}