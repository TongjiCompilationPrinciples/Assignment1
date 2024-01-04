//
// Created by Mac book pro on 2023/12/26.
//
#include"Ana.h"
namespace LexAna{
    Ana::Ana(FILE *fp) {
        this->fp=fp;
        this->col=this->row=this->pos=0;
        consPattern=true;
    }

    Word Ana::getWord() {
        Word word;
        static char word_temp[MAX_WORD_LEN];
        int index=0;
        char ch;
        while((ch = fgetc(fp)) != EOF && isspace(ch)){
            if(ch=='\n'){
                this->row++;
                this->col=0;
            }
            else{
                this->col++;
            }
            this->pos++;
            // 跳过空白字符
        }
        // 弥补上面的多读了一个字符
        this->pos++;
        this->col++;
        // 如果到达了文件末尾，返回_END
        if(ch==EOF){
            word.row=this->row--;
            word.col=this->col--;
            word.pos=this->pos--;
            word.type=_END;
            return word;
        }
        // 开始读取word
        if(isalpha(ch)){ // 如果是字母
            do{
                word_temp[index++]=ch;
                ch=fgetc(fp);
                this->col++;
                this->pos++;
            }while((isalpha(ch) || isdigit(ch))&&index<MAX_WORD_LEN);
            if(index==MAX_WORD_LEN){
                strcpy(word.val.msg, "标识符过长");
            }
            else{
                word_temp[index]='\0';
                ungetc(ch, fp); // 回退多读的一个字符
                this->col--;
                this->pos--;
                word.type=isKeyWord(word_temp)?KEYWORD:IDENTIFIER;
                if(word.type==KEYWORD){
                    for (auto & i : keyWordTable){
                        if(strcmp(word_temp, i)==0){
                            word.val.k=static_cast<KEY>(&i-keyWordTable);
                            break;
                        }
                    }
                }
                else{
                    strcpy(word.val.s, word_temp);
                }
            }
        }
        else if(isdigit(ch)){ // 如果是整数
            do{
                word_temp[index++]=ch;
                ch=fgetc(fp);
                this->col++;
                this->pos++;
            } while(isdigit(ch)&&index<MAX_WORD_LEN);
            if(index==MAX_WORD_LEN){
                strcpy(word.val.msg, "数字过长");
            }
            else{
                word_temp[index]='\0';
                ungetc(ch, fp); // 回退多读的一个字符
                this->col--;
                this->pos--;
                word.type=INTEGER;
                word.val.i=strtol(word_temp, nullptr, 10);
            }

        }
        else{ // 算符或界符或其他错误
            word_temp[index++]=ch;
            word_temp[index]='\0';
            word.type=OPERATOR;
            switch(ch){
                case '+':word.val.o=PLUS;break;
                case '-':word.val.o=MIN;break;
                case '*':word.val.o=MUL;break;
                case '/':word.val.o=DIV;break;
                case '=':word.val.o=EQ;break;
                case '<':{
                    ch=fgetc(fp);
                    this->col++;
                    this->pos++;
                    if(ch=='='){
                        word_temp[index++]=ch;
                        word_temp[index]='\0';
                        word.val.o=LE;
                    }
                    else if(ch=='>'){
                        word_temp[index++]=ch;
                        word_temp[index]='\0';
                        word.val.o=NEQ;
                    }
                    else{
                        ungetc(ch, fp);
                        this->col--;
                        this->pos--;
                        word.val.o=LT;
                    }
                    break;
                }
                case '>':{
                    ch=fgetc(fp);
                    this->col++;
                    this->pos++;
                    if(ch=='='){
                        word_temp[index++]=ch;
                        word_temp[index]='\0';
                        word.val.o=GE;
                    }
                    else{
                        ungetc(ch, fp);
                        this->col--;
                        this->pos--;
                        word.val.o=GT;
                    }
                    break;
                }
                case ':':{
                    ch=fgetc(fp);
                    this->col++;
                    this->pos++;
                    if(ch=='='){
                        word_temp[index++]=ch;
                        word_temp[index]='\0';
                        word.val.o=ASSIGN;
                    }
                    else{
                        ungetc(ch, fp);
                        this->col--;
                        this->pos--;
                        word.val.o=COLON;
                    }
                    break;
                }
                case '(':word.val.o=LP;break;
                case ')':word.val.o=RP;break;
                case ',':word.val.o=COMMA;break;
                case ';':word.val.o=SEMI;break;
                case '.':word.val.o=PERIOD;break;
                default:word.type=ERROR;break;
            }
        }
        word.row=this->row;
        word.col=this->col;
        word.pos=this->pos;
        return  word;
    }

    bool Ana::isKeyWord(const char *word) {
        for (auto & i : keyWordTable) {
            if (strcmp(word, i) == 0) {
                return true;
            }
        }
        return false;
    }

    Word Ana::peekWord() {
        Word word;
        long i=ftell(fp);
        word=getWord();
        fseek(fp, i, SEEK_SET);
        return word;
    }

    Word Ana::peekWord(int n) {
        Word word;
        long i=ftell(fp);
        for(int j=0;j<n;j++){
            word=getWord();
        }
        fseek(fp, i, SEEK_SET);
        return word;
    }

    Ana::Ana(const std::string &fileName) {
        this->fp=fopen(fileName.c_str(), "r");
        this->col=this->row=this->pos=0;
        if(this->fp== nullptr){
            std::cout<<"打开文件失败"<<std::endl;
            exit(0);
        }
        consPattern=false;
    }

    void Ana::setfp(const std::string& fileName) {
        if(!consPattern&&this->fp!= nullptr){ // 如果不是通过文件指针构造的，且文件指针不为空，则关闭文件指针
            fclose(this->fp);
        }
        this->fp=fopen(fileName.c_str(), "r");
        this->col=this->row=this->pos=0;
        if(this->fp== nullptr){
            std::cout<<"打开文件失败"<<std::endl;
            exit(0);
        }
        consPattern=false;
    }

    void Ana::reset() {
        fseek(this->fp, 0, SEEK_SET);
        this->col=this->row=this->pos=0;
    }

    std::string Ana::getCurrentLine() {
        long originalPos=ftell(fp); // 记录原始位置
        if (originalPos == -1L) return ""; // ftell失败
        // 向前搜索换行符，找到行的开始
        long lineStart = originalPos;
        while (lineStart > 0) {
            fseek(fp, --lineStart, SEEK_SET);
            if (fgetc(fp) == '\n') {
                // 找到上一行的换行符，移动到下一个字符，即当前行的开头
                fseek(fp, lineStart + 1, SEEK_SET);
                break;
            }
        }
        // 如果已经到达文件开头，设置lineStart为0
        if (lineStart == 0 && ftell(fp) > 0) {
            fseek(fp, 0, SEEK_SET);
        }

        // 读取整行
        std::string line;
        int ch;
        while ((ch = fgetc(fp)) != '\n' && ch != EOF) {
            line += static_cast<char>(ch);
        }

        // 恢复原始的文件指针位置
        fseek(fp, originalPos, SEEK_SET);

        return line;
    }
}
