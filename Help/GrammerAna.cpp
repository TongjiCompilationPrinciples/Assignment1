//
// Created by Mac book pro on 2024/1/3.
//

#include "GrammerAna.h"

GrammerAna::GraAna::GraAna(const std::string&sourceCode,const std::string& outputFileName) {
    ana.setfp(sourceCode);
    this->outputFileName = outputFileName;
    this->fp = fopen(outputFileName.c_str(), "w");
    if(this->fp== nullptr){
        std::cout<<"打开文件失败"<<std::endl;
        exit(0);
    }
}

void GrammerAna::GraAna::init() {
    memset(symTable, 0, sizeof(symTable));
    memset(transitionalCodes, 0, sizeof(transitionalCodes));
    totalDel = 0;
    nextTmp = 0;
    nextSym = 0;
    nextTCode = 0;
    ana.reset();
    sym = ana.getWord();
}

void GrammerAna::GraAna::program() {
    if(sym.type == KEYWORD && sym.val.k == PROGRAM) {
        // 程序开头必须是关键词语“PROGRAM”
        sym = ana.getWord();

        // 程序必须有<标识符>（PS：由于文法化简后该标识符无用，故掠过）
        if(sym.type != IDENTIFIER) {
            error();
        }
        else{
            sym = ana.getWord();
            // 进入<分程序>
            block();
        }
    } else {
        error();
    }
}

tCodeItem GrammerAna::GraAna::expression() {
    return tCodeItem();
}

tCodeItem GrammerAna::GraAna::term() {
    return tCodeItem();
}

tCodeItem GrammerAna::GraAna::factor() {
    return tCodeItem();
}

void GrammerAna::GraAna::getOpValue(tCodeItem &op1, tCodeItem &op2, long &op1Value, long &op2Value) {

}

long GrammerAna::GraAna::genTmp(long value) {
    if(nextSym >= symMax) {
        error();
    }
    std::string tmpName = "_tmp" + std::to_string(nextTmp++);
    strcpy(symTable[nextSym].name, (char*)tmpName.c_str());
    symTable[nextSym].type = interVar;
    symTable[nextSym].value = value;

    return nextSym++;
}

long GrammerAna::GraAna::position(char *symName) {
    // 如果变量名为空 或 null则返回 -1
    if(!symName|| symName[0] == '\0') {
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

void GrammerAna::GraAna::condition() {

}

bool GrammerAna::GraAna::statement() {
    bool isNull = true;

    // 赋值语句
    if(sym.type == IDENTIFIER) {
        // 寻找对应标识符的变量索引
        long resSymIndex = position(sym.val.s);

        //标识符未定义
        if(resSymIndex == -1) {
            error();
        }
        // 拿到相应的标识符
        SymTableItem resSym = symTable[resSymIndex];
        if(resSym.type == constant) { // 常量不能被赋值
            error();
        }
        // 标识符索引 构建 操作数1
        tCodeItem res = {1, resSymIndex};
        sym = ana.getWord();

        // 标识符后必须有 赋值号
        if(sym.type != OPERATOR||sym.val.o != ASSIGN) {
            error();
        }
        Word assignop = sym;

        // <表达式>翻译；并获取其结果的标识符（可能是立即数）
        sym = ana.getWord();
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
            sym = ana.getWord();
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
            sym = ana.getWord();
            statement();

            // 回填<条件>的假出口
            transitionalCodes[jumpCodeIndex].res.value = nextTCode;

            isNull = false;
        }
            // 循环语句
        else if(sym.val.k == WHILE) {
            // 记录循环跳转
            long whileJumpIndex = nextTCode;

            sym = ana.getWord();
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
            sym = ana.getWord();
            statement();

            // 循环无条件跳转回<条件>前进行判断
            gen(FCT::JUP, {(unsigned char)-1, 0}, {(unsigned char)-1, 0}, {0, whileJumpIndex});
            // 控制语句不满足条件直接跳过<语句>
            transitionalCodes[jumpCodeIndex].res.value = nextTCode;

            isNull = false;
        }
            // 复合语句!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!(TODO：待商讨)
        else if(sym.val.k == BEGIN) {
            sym = ana.getWord();

            // <语句>解析
            statement();

            // 如果是分号则循环解析<语句>
            while(sym.type == OPERATOR && sym.val.o == SEMI) {
                sym = ana.getWord();

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
            sym = ana.getWord();
        }
    }

    //不报错，因为可以推<空>
    return isNull;
}

void GrammerAna::GraAna::block() {
    // 如果有常量定义
    if(sym.type == KEYWORD && sym.val.k == CONST) {
        sym = ana.getWord();
        // 这里同样留给常量定义去报错
        constDeclaration();

        // 如果后面为“逗号”则循环定义常量
        while(sym.type == OPERATOR && sym.val.o == COMMA) {
            sym = ana.getWord();
            constDeclaration();
        }

        // 如果不是分号则报错
        if(sym.type != OPERATOR||sym.val.o != SEMI) {
            error();
        }
    }

    // 如果有变量定义
    if(sym.type == KEYWORD && sym.val.k == VAR) {
        sym = ana.getWord();

        varDeclaration();

        // 如果为“逗号”则循环定义
        while(sym.type == OPERATOR && sym.val.o == COMMA) {
            sym = ana.getWord();
            varDeclaration();
        }

        // 变量定义结尾必须有 分号
        if(sym.type != OPERATOR||sym.val.o != SEMI) {
            error();
        }
    }

    sym = ana.getWord();
    // 无需报错，直接进入；在<语句>翻译中再进行报错
    statement();
}

void GrammerAna::GraAna::varDeclaration() {

}

void GrammerAna::GraAna::constDeclaration() {
    if()
    // 不是标识符直接报错
    if(sym.type != IDENTIFIER) {
        error();
    }

    // 记录在符号表中
    symTable[nextSym].type = constant;
    strcpy(symTable[nextSym].name, sym.val.s);

    sym = ana.getWord();
    // 标识符后必须为赋值符号
    if(sym.type != OPERATOR||sym.val.o != ASSIGN) {
        error();
    }

    // 赋值后必须为<无符号整型>
    sym = ana.getWord();
    if(sym.type != INTEGER) {
        error();
    }
    symTable[nextSym].value = sym.val.i;
    nextSym++;
    totalDel++;

    sym = ana.getWord();
}

void GrammerAna::GraAna::gen(FCT op, tCodeItem p1, tCodeItem p2, tCodeItem res) {
    if(nextTCode >= tcMax) {
        error();
    }
    transitionalCodes[nextTCode].fct = op;
    transitionalCodes[nextTCode].p1 = p1;
    transitionalCodes[nextTCode].p2 = p2;
    transitionalCodes[nextTCode].res = res;
    nextTCode++;
}

void GrammerAna::GraAna::printSymTable() {

}

void GrammerAna::GraAna::printTCode_c(tCode &code) {

}

void GrammerAna::GraAna::printTCode_f(tCode &code) {

}

void GrammerAna::GraAna::error() {

}
