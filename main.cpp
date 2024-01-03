#include <iostream>
#include"LexAna/Ana.h"
#include "GraAna/GrammaticalAna.h"
#include "GraAna/RGraAna.h"

using namespace LexAna;
int main() {
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

    std::cout<<"====== GraAna Test Begin ======"<<std::endl;
    GraAna::RGraAna grammer("../grammer.txt");
    grammer.init();
    grammer.display();
    grammer.cal_all_first();
    //std::cout<<GraAna::RGraAna::eleminate_NonTerminalAngleBrackets("<A><B>ε PROGRAM")<<std::endl;
    std::cout<<"====== GraAna Test End ======"<<std::endl;
}
