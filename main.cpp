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

Word sym;                           // 当前即将要解析的非终结符
const long symMax = 2000;           // 符号表最大符号记录个数
const long tcMax = 2000;            // 中间代码表最大代码记录条数
SymTableItem symTable[symMax];      //存放已经定义的变量/常量/中间变量的符号表
tCode transitionalCodes[tcMax];     //存放已经生成的中间代码
long totalDel = 0;                  //代码中定义的 变量 + 常量 的个数（用于符号表搜索）
long nextTmp = 0;                   //下一个中间变量的序号                   
long nextSym = 0;                   //即将存放符号的符号表索引
long nextTCode = 0;                 //即将存放中间代码的中间代码表索引

std::string fctString[] = {
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

void block();
tCodeItem expression();
void condition();
bool statement();
long position(char*);

void error() {
    std::cout <<"illegal input!" << std::endl;
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
    if(code.p1.type == (unsigned char)1) {
        std::cout<<symTable[code.p1.value].name;
    } else if(code.p1.type == (unsigned char)0){
        std::cout<<code.p1.value;
    } else {
        std::cout<<'-';
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

void printSymTable() {
    for(int i = 0;i < nextSym; i++) {
        std::cout<<symTable[i].name<<' '<<symTable[i].type<<' '<<symTable[i].value<<std::endl;
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
            sym = lexAna.getWord();
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
            sym = lexAna.getWord();
            varDeclaration();
        }

        // 变量定义结尾必须有 分号
        if(sym.type != OPERATOR) {
            error();
        } else if(sym.val.o != SEMI) {
            error();
        }
    }

    sym = lexAna.getWord();
    // 无需报错，直接进入；在<语句>翻译中再进行报错
    statement();
}

// <语句> -> <赋值语句>|<条件语句>|<循环语句>|<复合语句>|空（需要特别判断是否推空）
bool statement() {
    bool isNull = true;

    // 赋值语句
    if(sym.type == IDENTIFIER) {
        // 寻找对应标识符的变量索引
        long resSymIndex = position(sym.val.s);

        //标识符未定义
        if(resSymIndex == -1) {
            error();
        }

        // 标识符索引 构建 操作数1
        tCodeItem res = {1, resSymIndex};
        sym = lexAna.getWord();

        // 标识符后必须有 赋值号
        if(sym.type != OPERATOR) {
            error();
        } else if(sym.val.o != ASSIGN) {
            error();
        }
        Word assignop = sym;

        // <表达式>翻译；并获取其结果的标识符（可能是立即数）
        sym = lexAna.getWord();
        tCodeItem op1 = expression();

        // 生成中间代码：res = op2
        gen(FCT(assignop.val.o), op1, {(unsigned char)-1, -1}, res);
        // 维护符号表值
        long op1Value;
        if(op1.type == 1) {
            op1Value = symTable[op1.value].value;
        } else {
            op1Value = op1.value;
        }
        symTable[res.value].value = op1Value;

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

            // 循环无条件跳转回<条件>前进行判断
            gen(FCT::JUP, {(unsigned char)-1, 0}, {(unsigned char)-1, 0}, {0, whileJumpIndex});
            // 控制语句不满足条件直接跳过<语句>
            transitionalCodes[jumpCodeIndex].res.value = nextTCode;

            isNull = false;
        }
        // 复合语句!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!(TODO：待商讨)
        else if(sym.val.k == BEGIN) {
            sym = lexAna.getWord();

            // <语句>解析
            statement();

            // 如果是分号则循环解析<语句>
            while(sym.type == OPERATOR && sym.val.o == SEMI) {
                sym = lexAna.getWord();

                //分号后面的<语句>不能推空，直接报错
                if(statement()) {
                    error();
                }
            }

            //复合语句后必须有 END
            if(sym.type != KEYWORD) {
                error();
            } else if(sym.val.k != END) {
                error();
            }

            isNull = false;
            sym = lexAna.getWord();
        }
    }

    //不报错，因为可以推<空>
    return isNull;
}

// <条件> -> <表达式><关系运算符><表达式>：产生一个 条件goto，待上层程序回填
void condition() {
    //直接进入<表达式>，不检查；并获取其结果对应的符号，作为操作数1
    tCodeItem op1 = expression();

    // 后面必须为关系运算符
    if(sym.type != OPERATOR) {
        error();
    } else {
        // 暂存关系运算符号
        Word relop = sym;

        sym = lexAna.getWord();

        // 进入右边 <表达式> 解析；并获取其结果对应符号，作为操作数2
        tCodeItem op2 = expression();

        // 根据对应关系运算符生成中间代码
        switch (relop.val.o)
        {
            // 根据<关系运算符>反着填入
            case EQ:
                gen(FCT::JNEQ, op1, op2, {0, 0});
                break;
            case NEQ:
                gen(FCT::JEQ, op1, op2, {0, 0});
                break;
            case GT:
                gen(FCT::JLE, op1, op2, {0, 0});
                break;
            case GE:
                gen(FCT::JLT, op1, op2, {0, 0});
                break;
            case LT:
                gen(FCT::JGE, op1, op2, {0, 0});
                break;
            case LE:
                //  暂时不填入跳转的地址，等待上层递归回填（这里要反着填？？）
                gen(FCT::JGT, op1, op2, {0, 0});
                break;
            default:
                // 不是任何关系运算符则报错
                error();
                break;
        }
    }
}

// 从符号表中顺序查找对应的变量/常量
long position(char* symName) {
    // 如果变量名为空 或 null则返回 -1
    if(symName == nullptr || strlen(symName) == 0) {
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

//传入两个符号，返回其对应的数值
void getOpValue(tCodeItem& op1, tCodeItem& op2, long& op1Value, long& op2Value) {
    // 如果op1表示 立即数
    if(op1.type == 0) {
        op1Value = op1.value;
    } else {
        // 否则为 标识符
        op1Value = symTable[op1.value].value;
    }
    // 如果op2表示 立即数
    if(op2.type == 0) {
        op2Value = op2.value;
    } else {
        // 否则为 标识符
        op2Value = symTable[op2.value].value;
    }
}

// <因子> -> <标识符>|<无符号整数>|(<表达式>)：产生一个中间变量
tCodeItem factor() {
    tCodeItem ret;

    // 标识符
    if(sym.type == IDENTIFIER) {
        // 从开头顺序遍历查找符号表
        long symIndex = position(sym.val.s);

        // 标识符未定义
        if(symIndex == -1) {
            error();
        }

        // 返回标识符
        ret.type = 1;
        ret.value = symIndex;
    }
    // 无符号整数
    else if(sym.type == INTEGER) {
        long value = sym.val.i;

        ret.type = 0;
        ret.value = value;
    }
    // （表达式）
    else if(sym.type == OPERATOR && sym.val.o == LP) {
        sym = lexAna.getWord();

        // 进入<表达式>语法分析
        ret = expression();

        // 表达式之后必须有右括号
        if(sym.type != OPERATOR) {
            error();
        } else if(sym.val.o != RP) {
            error();
        }
    }
    // 上述都不是则报错
    else {
        error();
    }

    sym = lexAna.getWord();
    return ret;
}

// <项> -> <因子>{<乘法运算符><因子>}：产生中间代码 + 一个中间变量
tCodeItem term() {
    //直接进入 <因子> 解析，并记录存放结果的中间变量
    Word mulop;
    tCodeItem res;
    res = factor();

    // 有<乘法运算符>则继续进行解析；同时存在运算，因此必定返回一个中间变量
    while(sym.type == OPERATOR && (sym.val.o == MUL || sym.val.o == DIV)) {
        mulop = sym;

        sym = lexAna.getWord();

        tCodeItem op2 = factor();
        tCodeItem op1 = res;

        // 生成存放结果的中间变量，并生成对应的中间代码
        long resSymIndex = genTmp(-1);
        res.type = 1;
        res.value = resSymIndex;

        gen((FCT)mulop.val.o, op1, op2, res);

        // 维护符号表中的值
        long op1Value, op2Value;
        getOpValue(op1, op2, op1Value, op2Value);
        if(mulop.val.o == MUL) {
            //乘法
            symTable[res.value].value = op1Value * op2Value;
        } else {
            //除法
            symTable[res.value].value = op1Value / op2Value;
        }

        sym = lexAna.getWord();
    }

    return res;
}

// <表达式> -> [-|+]<项>{<加法运算符><项>} （生成中间变量 + 中间代码）
tCodeItem expression() {
    Word addop;
    tCodeItem res;

    // 处理正负号（可选）
    if(sym.type == OPERATOR && (sym.val.o == MIN || sym.val.o == PLUS)) {
        addop=sym;                 // 保存正负号
        sym = lexAna.getWord();

        tCodeItem op2 = term();     // 正负号后面是一个term

        // 生成一个中间变量存放结果，并生成一条中间代码进行运算并存储到该中间变量
        long resSymIndex = genTmp(-1);
        res.type = 1;
        res.value = resSymIndex;

        // 单目运算符 res = 0 [+/-] op1
        gen((FCT)sym.val.o, {0, 0}, op2, res);
    }
    // 若开头无加法运算符，则直接进入“项”递归下降函数
    else {
        // 报错交给<项>
        res = term();
    }

    // 若后面仍有“加法运算符”连接的表达式；则一定产生一个中间变量
    while(sym.type == OPERATOR && (sym.val.o == MIN || sym.val.o == PLUS))
    {
        addop=sym;                 // 保存运算符
        sym = lexAna.getWord();

        tCodeItem op2 = term();     // 运算符后是一个term
        tCodeItem op1 = res;

        // 产生一个中间变量存放结果，产生一条中间代码
        long resSymIndex = genTmp(-1);
        res.type = 1;
        res.value = resSymIndex;

        gen((FCT)addop.val.o, op1, op2, res);
        // 维护符号表的值
        long op1Value, op2Value;
        getOpValue(op1, op2, op1Value, op2Value);
        if(addop.val.o == PLUS) {
            //加法
            symTable[res.value].value = op1Value + op2Value;
        } else {
            // 减法
            symTable[res.value].value = op1Value - op2Value;
        }
    }

    return res;
}

// <程序> -> PROGRAM<分程序>
void program() {
    if(sym.type == KEYWORD && sym.val.k == PROGRAM) {
        // 程序开头必须是关键词语“PROGRAM”
        sym = lexAna.getWord();

        // 程序必须有<标识符>（PS：由于文法花间后该标识符无用，故掠过）
        if(sym.type != IDENTIFIER) {
            error();
        }

        sym = lexAna.getWord();
        // 进入<分程序>
        block();

    } else {
        error();
    }
}

int main() {

    sym = lexAna.getWord();
    // 语法分析
    program();

    for(int i = 0; i < nextTCode; i++) {
        std::cout<<i<<": ";
        printTCode(transitionalCodes[i]);
    }
    printSymTable();

//    test();

    return 0;
}