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
    IDs.clear();
    ana.reset();
    getNextWord();
}

void GrammerAna::GraAna::program() {
    if(sym.type == KEYWORD && sym.val.k == PROGRAM) {
        // 程序开头必须是关键词语“PROGRAM”
        getNextWord();

        // 程序必须有<标识符>（PS：由于文法化简后该标识符无用，故掠过）
        if(sym.type != IDENTIFIER) {
            error("程序必须有标识符,且标识符必须为小写字母开头");
        }
        else{
            getNextWord();
            // 进入<分程序>
            block();
        }
    } else {
        error("程序开头必须是关键词“PROGRAM”");
    }
}

tCodeItem GrammerAna::GraAna::expression() {
    Word addop;
    tCodeItem res{};

    // 这里是新加的
    long resSymIndex;
    bool ifFirst=true;
    // 上面是新加的
    if(sym.type == OPERATOR && (sym.val.o == MIN || sym.val.o == PLUS)) {// 处理正负号（可选）
        addop=sym;                 // 保存正负号
        getNextWord();

        tCodeItem op2 = term();     // 正负号后面是一个term

        // 生成一个中间变量存放结果，并生成一条中间代码进行运算并存储到该中间变量
        resSymIndex = genTmp(-1);
        ifFirst=false;
        res.type = 1;
        res.value = resSymIndex;

        // 单目运算符 res = 0 [+/-] op1
        gen((FCT)sym.val.o, {0, 0}, op2, res);
    }
    else {// 若开头无加法运算符，则直接进入“项”递归下降函数
        // 报错交给<项>
        res = term();
    }

    while(sym.type == OPERATOR && (sym.val.o == MIN || sym.val.o == PLUS))
    {// 若后面仍有“加法运算符”连接的表达式；则一定产生一个中间变量
        addop=sym;                 // 保存运算符
        getNextWord();

        tCodeItem op1 = res;
        tCodeItem op2 = term();     // 运算符后是一个term

        // 产生一个中间变量存放结果，产生一条中间代码
        // long resSymIndex = genTmp(-1); //TODO 可否删去这里？
        if(ifFirst){
            resSymIndex = genTmp(-1);
            ifFirst=false;
        }
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

tCodeItem GrammerAna::GraAna::term() {
    //直接进入 <因子> 解析，并记录存放结果的中间变量
    Word mulop;
    tCodeItem res{};
    res = factor();

    // 新优化的，只用一个中间变量
    bool isFirst=true;
    long resSymIndex ;
    // 上面是新加的

    // 有<乘法运算符>则继续进行解析；同时存在运算，因此必定返回一个中间变量
    while(sym.type == OPERATOR && (sym.val.o == MUL || sym.val.o == DIV)) {
        mulop = sym;

        getNextWord();

        tCodeItem op1 = res;
        tCodeItem op2 = factor();


        // 生成存放结果的中间变量，并生成对应的中间代码
        // 下面是新加的
        if(isFirst){
            resSymIndex = genTmp(-1);
            isFirst=false;
        }
        // 上面是新加的
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

        getNextWord();
    }

    return res;
}

tCodeItem GrammerAna::GraAna::factor() {
    tCodeItem ret{};

    if(sym.type == IDENTIFIER) {// 标识符
        // 从开头顺序遍历查找符号表
        long symIndex = position(sym.val.s);

        // 标识符未定义
        if(symIndex == -1) {
            error("标识符未定义");
        }

        // 返回标识符
        ret.type = 1;
        ret.value = symIndex;
    }
    else if(sym.type == INTEGER) {// 无符号整数
        long value = sym.val.i;

        ret.type = 0;
        ret.value = value;
    }
    else if(sym.type == OPERATOR && sym.val.o == LP) {// （表达式）
        getNextWord();

        // 进入<表达式>语法分析
        ret = expression();

        // 表达式之后必须有右括号
        if(sym.type != OPERATOR||sym.val.o != RP) {
            error("缺少右括号");
        }
    }
    else {// 上述都不是则报错
        error("因子错误,因子必须为标识符/无符号整数/表达式");
    }

    getNextWord();
    return ret;
}

void GrammerAna::GraAna::getOpValue(tCodeItem &op1, tCodeItem &op2, long &op1Value, long &op2Value) {
    // 如果op1表示 立即数
    if(op1.type == 0) {
        op1Value = op1.value;
    }
    else if(op1.type!=-1){
        // 否则为 标识符
        op1Value = symTable[op1.value].value;
    }
    // 如果op2表示 立即数
    if(op2.type == 0) {
        op2Value = op2.value;
    }
    else if(op2.type!=-1){// 否则为 标识符
        op2Value = symTable[op2.value].value;
    }
}

long GrammerAna::GraAna::genTmp(long value) {
    if(nextSym >= symMax) {
        error("符号表溢出");
    }
    std::string tmpName = "_TMP" + std::to_string(nextTmp++);
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
    //直接进入<表达式>，不检查；并获取其结果对应的符号，作为操作数1
    tCodeItem op1 = expression();

    // 后面必须为关系运算符
    if(sym.type != OPERATOR) {
        error("条件语句后必须为操作符且为关系运算符");
    } else {
        // 暂存关系运算符号
        Word relop = sym;

        getNextWord();

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
                error("条件语句后必须为操作符且为关系运算符");
                break;
        }
    }
}

bool GrammerAna::GraAna::statement() {
    bool isNull = true;

    // 赋值语句
    if(sym.type == IDENTIFIER) {
        // 寻找对应标识符的变量索引
        long resSymIndex = position(sym.val.s);

        //标识符未定义
        if(resSymIndex == -1) {
            error("标识符未定义");
        }
        // 拿到相应的标识符
        SymTableItem resSym = symTable[resSymIndex];
        if(resSym.type == constant) { // 常量不能被赋值
            error("常量不能被赋值");
        }
        // 标识符索引 构建 操作数1
        tCodeItem res = {1, resSymIndex};
        getNextWord();

        // 标识符后必须有 赋值号
        if(sym.type != OPERATOR||sym.val.o != ASSIGN) {
            error("赋值语句必须有赋值");
        }
        Word assignop = sym;

        // <表达式>翻译；并获取其结果的标识符（可能是立即数）
        getNextWord();
        tCodeItem op1 = expression();

        // 生成中间代码：res = op2
        gen(FCT(assignop.val.o), op1, {-1, -1}, res);
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
    else if(sym.type == KEYWORD) {// 条件语句
        if(sym.val.k == IF) {// <条件>解析
            getNextWord();
            condition();

            // 记录要回填的jump中间指令索引
            long jumpCodeIndex = nextTCode - 1;

            // 后面必须有 then
            if(sym.type != KEYWORD||sym.val.k != THEN) {
                error("条件语句后必须有THEN");
            }
            // <语句>解析
            getNextWord();
            statement();

            // 回填<条件>的假出口
            transitionalCodes[jumpCodeIndex].res.value = nextTCode;

            isNull = false;
        }
        else if(sym.val.k == WHILE) {// 循环语句
            // 记录循环跳转
            long whileJumpIndex = nextTCode;

            getNextWord();
            // 解析<条件>，并记录假出口跳转的回填索引
            condition();
            long jumpCodeIndex = nextTCode - 1;

            // 循环语句必须有 DO
            if(sym.type != KEYWORD||sym.val.k != DO) {
                error("循环语句后必须有DO");
            }
            // <语句>解析
            getNextWord();
            statement();

            // 循环无条件跳转回<条件>前进行判断
            gen(FCT::JUP, {-1, 0}, {-1, 0}, {0, whileJumpIndex});
            // 控制语句不满足条件直接跳过<语句>
            transitionalCodes[jumpCodeIndex].res.value = nextTCode;

            isNull = false;
        }
        else if(sym.val.k == BEGIN) {
            getNextWord();

            // <语句>解析
            statement();

            // 如果是分号则循环解析<语句>
            while(sym.type == OPERATOR && sym.val.o == SEMI) {
                getNextWord();

                //分号后面的<语句>不能推空，直接报错
                if(statement()) {
                    error("分号后面必须有语句");
                }
            }

            //复合语句后必须有 END
            if(sym.type != KEYWORD||sym.val.k != END) {
                error("复合语句后必须有END");
            }

            isNull = false;
            getNextWord();
        }
    }

    //不报错，因为可以推<空>
    return isNull;
}

void GrammerAna::GraAna::block() {
    // 如果有常量定义
    if(sym.type == KEYWORD && sym.val.k == CONST) {
        getNextWord();
        // 这里同样留给常量定义去报错
        constDeclaration();

        // 如果后面为“逗号”则循环定义常量
        while(sym.type == OPERATOR && sym.val.o == COMMA) {
            getNextWord();
            constDeclaration();
        }

        // 如果不是分号则报错
        if(sym.type != OPERATOR||sym.val.o != SEMI) {
            error("常量定义后必须有分号");
        }
    }

    // 如果有变量定义
    if(sym.type == KEYWORD && sym.val.k == VAR) {
        getNextWord();

        varDeclaration();

        // 如果为“逗号”则循环定义
        while(sym.type == OPERATOR && sym.val.o == COMMA) {
            getNextWord();
            varDeclaration();
        }

        // 变量定义结尾必须有 分号
        if(sym.type != OPERATOR||sym.val.o != SEMI) {
            error("变量定义后必须有分号");
        }
    }

    getNextWord();
    // 无需报错，直接进入；在<语句>翻译中再进行报错
    statement();
}

void GrammerAna::GraAna::varDeclaration() {
    if(nextSym >= symMax) {
        error("符号表溢出");
    }
    if(sym.type != IDENTIFIER) {
        error("变量定义必须有标识符");
    }
    if(IDs.find(sym.val.s) != IDs.end()) { // 标识符重复
        error("标识符重复");
    }
    // 记录在符号表中
    symTable[nextSym].type = variable;
    strcpy(symTable[nextSym].name, sym.val.s);
    // 变量初始化 -1
    symTable[nextSym].value = -1;
    IDs.insert(symTable[nextSym].name);
    nextSym++;
    totalDel++;

    getNextWord();
}

void GrammerAna::GraAna::constDeclaration() {
    if(nextSym >= symMax) {
        error("符号表溢出");
    }
    // 不是标识符直接报错
    if(sym.type != IDENTIFIER) {
        error("常量定义必须有标识符");
    }
    if(IDs.find(sym.val.s) != IDs.end()) { // 标识符重复
        error("标识符重复");
    }
    // 记录在符号表中
    symTable[nextSym].type = constant;
    strcpy(symTable[nextSym].name, sym.val.s);

    getNextWord();
    // 标识符后必须为赋值符号
    if(sym.type != OPERATOR||sym.val.o != ASSIGN) {
        error("常量定义后必须有赋值符号");
    }

    // 赋值后必须为<无符号整型>
    getNextWord();
    if(sym.type != INTEGER) {
        error("常量定义必须有无符号整数");
    }
    symTable[nextSym].value = sym.val.i;
    IDs.insert(symTable[nextSym].name);
    nextSym++;
    totalDel++;

    getNextWord();
}

void GrammerAna::GraAna::gen(FCT op, tCodeItem p1, tCodeItem p2, tCodeItem res) {
    if(nextTCode >= tcMax) {
        error("中间代码表溢出");
    }
    transitionalCodes[nextTCode].fct = op;
    transitionalCodes[nextTCode].p1 = p1;
    transitionalCodes[nextTCode].p2 = p2;
    transitionalCodes[nextTCode].res = res;
    nextTCode++;
}

void GrammerAna::GraAna::printSymTable() {
    for(int i = 0;i < nextSym; i++) {
        std::cout<<symTable[i].name<<' '<<symTable[i].type<<' '<<symTable[i].value<<std::endl;
    }
}
void GrammerAna::GraAna::printTCode_c(tCode &code) {
    std::cout<<"("<<fctStrings[static_cast<int>(code.fct)]<<", ";
    // 判断操作数是 标识符 还是 直接数
    if(code.p1.type ==1) {
        std::cout<<symTable[code.p1.value].name;
    } else if(code.p1.type == 0){
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

void GrammerAna::GraAna::printTCode_f(tCode &code) {
    fprintf(fp, "(%s, ", fctStrings[static_cast<int>(code.fct)].c_str());

    if (code.p1.type == 1) {
        fprintf(fp, "%s", symTable[code.p1.value].name);
    } else if (code.p1.type == 0) {
        fprintf(fp, "%ld", code.p1.value);
    } else {
        fprintf(fp, "-");
    }

    fprintf(fp, ", ");

    if (code.p2.type == static_cast<unsigned char>(1)) {
        fprintf(fp, "%s", symTable[code.p2.value].name);
    } else if (code.p2.type == static_cast<unsigned char>(0)) {
        fprintf(fp, "%ld", code.p2.value);
    } else {
        fprintf(fp, "-");
    }

    fprintf(fp, ", ");

    if (code.res.type) {
        fprintf(fp, "%s", symTable[code.res.value].name);
    } else {
        fprintf(fp, "%ld", code.res.value);
    }

    fprintf(fp, ")\n");
}
void GrammerAna::GraAna::printTCode(int opt) {
    if(opt == 0) {
        for(int i = 0; i < nextTCode; i++) {
            std::cout<<i<<": ";
            printTCode_c(transitionalCodes[i]);
        }
    } else {
        for(int i = 0; i < nextTCode; i++) {
            fprintf(fp, "%d: ", i);
            printTCode_f(transitionalCodes[i]);
        }
    }
}

void GrammerAna::GraAna::start()
{
    init();
    program();
    if(sym.type != _END) {
        error("程序结尾必须为END");
    }
}

void GrammerAna::GraAna::getNextWord() {
    sym=ana.getWord();
    if(sym.type == ERROR) {
        error();
    }
    else if(sym.type==IDENTIFIER){
        if(!validId(sym.val.s)){
            error("不是合法的标识符");
        }
    }
}

void GrammerAna::GraAna::error(const std::string& msg)
{
    if(!msg.empty()){
        long line = sym.row;
        long col = sym.col;
        std::cerr<<"第"<<line<<"行，第"<<col<<"列<"+ana.getCurrentLine()+">报错："+msg<<std::endl;
        abort();
    }
    else{
        long line = sym.row;
        long col = sym.col;
        std::cerr<<"第"<<line<<"行，第"<<col<<"列<"+ana.getCurrentLine()+">报错：";
        if(sym.type==ERROR){
            std::cerr<<sym.val.msg<<std::endl;
        }
        else{
            std::cerr<<"语法错误"<<std::endl;
        }
        abort();
    }
}

bool GrammerAna::GraAna::validId(const std::string &id) {
    if(id.empty()){
        return false;
    }
    if(!islower(id[0])){
        return false;
    }
    for(size_t i=1;i<id.size();i++){
        if(!islower(id[i])&&!isdigit(id[i])){
            return false;
        }
    }
    return true;
}
