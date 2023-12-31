//
// Created by Mac book pro on 2023/12/31.
//
#include "RGraAna.h"
GraAna::RGTYPE GraAna::RUnit::getType() const {
    return this->type;
}

const std::string &GraAna::RUnit::getName() const {
    return this->name;
}

void GraAna::RUnit::setType(GraAna::RGTYPE _type) {
    this->type=_type;
}

void GraAna::RUnit::setName(const std::string &_name) {
    this->name=_name;
}

std::string GraAna::RUnit::getUnitsName() const {
    std::string res;
    for(auto& unit:this->units){
        res+=unit.tostring();
    }
    return res;
}

std::string GraAna::RUnit::tostring() const {
    if(this->units.empty()){
        switch (this->type) {
            case RTERMINAL:
                return this->name;
            case RNON_TERMINAL:
                return "<"+this->name+">";
            case RMULTI:
                return "("+this->name+")"; // TODO: 复合类型的输出
            default:
                return "";
        }
    }
    else{
        switch (this->type) {
            case RTERMINAL:
                return this->getUnitsName();
            case RNON_TERMINAL:
                return "<"+this->getUnitsName()+">";
            case RMULTI:
                return "("+this->getUnitsName()+")"; // TODO: 复合类型的输出
            default:
                return "";
        }
    }
}

void GraAna::RUnit::addChild(const GraAna::RUnit &unit) {
    this->units.push_back(unit);
}

std::vector<GraAna::RUnit> GraAna::RUnit::getUnits() const {
    return this->units;
}

void GraAna::RUnit::deleteChild(size_t index) {
    this->units.erase(this->units.begin()+index);
}

void GraAna::RGraAna::readGrammar() {
    std::fstream in(this->filename);
    if(!in){
        throw std::runtime_error("文件打开失败");
    }
    std::string line;
    std::regex parser_line("<(.+?)>→(.+)"); // 解析一行的正则表达式
    std::smatch match;
    while(getline(in,line)){
        std::clog<<"read grammar: "+line<<std::endl;
        std::cout<<std::endl;
        if(std::regex_search(line,match,parser_line)){
            std::string non_terminal = match[1].str();
            std::string production = match[2].str();
            // non_terminal = std::regex_replace(non_terminal, std::regex("\\s+"), ""); // 可选
            // production = std::regex_replace(production, std::regex("\\s+"), ""); // 可选
            std::cout<<"当前规则起始："<<non_terminal<<std::endl;
            auto alternatives = extractCandidates(production);
            for(auto& alternative:alternatives){
                std::clog<<"当前规则中的候选式："<<alternative<<std::endl;
            }
            size_t pos1=0;
            std::map<std::string,std::string> _replace;
            while(pos1<production.length()){
                size_t pos2=pos1;
                std::string non_terminal_name=GraAna::RGraAna::getNonTerminalName(production,pos2);
                std::cout<<"789: "<<non_terminal_name<<std::endl;
                if(!non_terminal_name.empty()){
                    non_terminal_name=non_terminal_name.substr(1,non_terminal_name.length()-2);
                    std::cout<<"规则中的非终结符："<<non_terminal_name<<std::endl;
                    size_t left=pos2-non_terminal_name.length()-1;
                    std::cout<<production.substr(left,pos2-left+1)<<std::endl;
                    if(_map.find(non_terminal_name)!=_map.end()){
                        std::cout<<"该非终结符已经加入_map，进行替换"<<std::endl;
                        std::cout<<"替换前: "<<production<<std::endl;
                        auto _alternatives = extractCandidates(_map[non_terminal_name]);
                        eleminate_repeatable(_alternatives);
                        // 如果候选式之前是|，就不用添加前缀了
                        // 如果候选式之后是|，就不用添加后缀了
                        bool need_prefix=true;
                        bool need_suffix=true;
                        if(pos1>0){
                            if(production[left]=='|'){
                                need_prefix=false;
                            }
                        }
                        if(pos2<production.length()-1){
                            if(production[pos2+1]=='|'){
                                need_suffix=false;
                            }
                        }
                        std::string prefix,suffix;
                        if(need_prefix){
                            prefix=production.substr(0,left);
                        }
                        if(need_suffix){
                            suffix=production.substr(pos2+1);
                        }
                        std::cout<<"前缀："<<prefix<<std::endl;
                        std::cout<<"用作替换的候选式："<<_map[non_terminal_name]<<std::endl;
                        std::cout<<"后缀："<<suffix<<std::endl;
                        for(auto& alt:_alternatives){
                            if(need_prefix){
                                alt = prefix+alt;
                            }
                            if(need_suffix){
                                alt += suffix;
                            }
                        }
                        // 用|将替换后的候选式连接起来
                        std::string new_production;
                        // 如果没有添加前缀，则把原来的前缀加上
                        if(!need_prefix){
                            new_production+=production.substr(0,left);
                        }
                        for(auto& alt:_alternatives){
                            new_production+=alt+"|";
                        }
                        // 如果没有添加后缀，则把原来的后缀加上
                        size_t new_pos1=new_production.length();
                        if(!need_suffix){
                            new_production+=production.substr(pos2+2);
                        }
                        // 如果最后一个字符是|，还要删去
                        if(new_production[new_production.length()-1]=='|'){
                            new_production=new_production.substr(0,new_production.length()-1);
                        }
                        production=new_production;
                        std::cout<<"替换后: "<<production<<std::endl;
                        pos1=left+1;
                    }
                    else{
                        std::cout<<"该非终结符还未加入_map，不能进行替换"<<std::endl;
                        pos1=pos2+1;
                    }
                }
                else{
                    std::cout<<"该规则中没有非终结符"<<std::endl;
                    pos1=production.length(); // 证明已经到达了行末尾
                }
            }
            // 开始进行消除左递归
            alternatives = extractCandidates(production);
            eleminate_repeatable(alternatives);
            std::set<std::string> alpha; // 存放当前非终结符开头的候选式（左递归下的候选式剩余部分）
            std::set<std::string> beta; // 存放非当前非终结符开头的候选式
            bool existLeftRecursion=false;
            for (const auto& alt : alternatives){
                pos1=0;
                bool isLeftRecursion=false;
                std::cout<<"当前候选式："<<alt<<std::endl;
                // 消除多余空格
                std::string _alt=std::regex_replace(alt, std::regex("\\s+"), "");
                if(_alt[0]=='<'){
                    std::string non_terminal_name=GraAna::RGraAna::getNonTerminalName(_alt,pos1);
                    non_terminal_name=non_terminal_name.substr(1,non_terminal_name.length()-2);
                    std::cout<<"123:"<<non_terminal_name<<std::endl;
                    std::cout<<"456:"<<non_terminal<<std::endl;
                    if(non_terminal_name==non_terminal.substr(1,non_terminal.length()-2)){
                        existLeftRecursion=isLeftRecursion=true;
                        std::cout<<"发现左递归！"<<std::endl;
                        alpha.insert(_alt.substr(pos1));
                    }
                }
                if(!isLeftRecursion){ // 当前候选式不是左递归
                    beta.insert(_alt);
                }
            }
            RRHS rhs;
            if(existLeftRecursion){ // 存在左递归
                // 添加新的非终结符
                std::string new_non_terminal=non_terminal+"'";
                // 对应的新的候选式
                std::string new_production;
                for(const auto& alt:alpha){
                    new_production+=alt+"<"+new_non_terminal+">|";
                }
                // 加入空集符号
                new_production+="ε";
                std::cout<<"新增加的文法规则为："+new_non_terminal+"→"+new_production<<std::endl;
                // 将新的非终结符和新的候选式添加到_map中
                _map[new_non_terminal]=new_production;
                RRHS new_rhs;
                auto new_alternatives = extractCandidates(new_production);
                eleminate_repeatable(new_alternatives);
                for (const auto& alt : new_alternatives){
                    auto unit=buildUnits(alt);
                    new_rhs.candidates.push_back(unit);
                }
                productions[new_non_terminal]=new_rhs;
                // 将beta中的候选式添加到当前非终结符的候选式中
                production="";
                for(const auto& alt:beta){
                    auto unit=buildUnits(alt+"<"+new_non_terminal+">");
                    rhs.candidates.push_back(unit);
                    production+=alt+"<"+new_non_terminal+">|";
                }
                production=production.substr(0,production.length()-1);
            }
            else{ // 不存在左递归
                alternatives = extractCandidates(production);
                eleminate_repeatable(alternatives);
                for (const auto& alt : alternatives){
                    auto unit=buildUnits(alt);
                    rhs.candidates.push_back(unit);
                }
            }
            _map[non_terminal]=production;
            productions[non_terminal]=rhs;
        }
        else{
            throw std::runtime_error("文法格式错误");
        }
    }

}

bool GraAna::RGraAna::init() {
    try{
        this->readGrammar();
    }
    catch (std::runtime_error& e){
        std::cerr<<e.what()<<std::endl;
        return false;
    }
    return true;
}

std::string GraAna::RGraAna::getNonTerminalName(const std::string& production,bool isHead) const {
    size_t pos1=0;
    if(isHead){
        pos1=0;
        char left=production[0];
        if(left!='<'){
            return "";
        }
        else{
            return this->getNonTerminalName(production,pos1);
        }
    }
    else{
        return this->getNonTerminalName(production,pos1);
    }
}

std::string GraAna::RGraAna::getNonTerminalName(const std::string &production, size_t &index) {
    size_t pos1=production.find('<',index);// 查找下一个非终结符的位置
    size_t pos2=pos1+1;
    while(pos2<production.size()&&pos1<production.size()){
        char right=production[pos2];
        if(right=='>'||right=='='||right=='|'||right==' '){
            pos1=production.find('<',pos2);
            pos2=pos1+1;
        } // 证明pos1是符号的一部分
        else{
            // pos1是非终结符的开始
            pos2=production.find('>',pos1);
            break;
        }
    }
    if(pos1>=production.size()||pos2>=production.size()){
        return "";
    }
    else{
        index=pos2;
        return production.substr(pos1,pos2-pos1+1);
    }

}

std::vector<std::string> GraAna::RGraAna::extractCandidates(const std::string &rhs) {
    return GraAna::RGraAna::parseAlternatives(rhs, "");
}

std::vector<std::string> GraAna::RGraAna::parseAlternatives(const std::string &segment, const std::string &prefix) {
    std::vector<std::string> alternatives;
    size_t i = 0;
    size_t segment_length = segment.length();
    std::string buffer = prefix;
    while (i < segment_length) {
        char c = segment[i];
        if (c == '|') {
            alternatives.push_back(buffer);
            buffer = prefix; // 恢复默认值
        } else {
            buffer += c;
        }
        i++;
    }

    if (!buffer.empty()) {
        alternatives.push_back(buffer);
    }

    return alternatives;
}

GraAna::RUnit GraAna::RGraAna::buildUnits(const std::string &candidate) {
    RUnit unit(RGTYPE::REMPTY,"");
    size_t tot_len=candidate.length();
    size_t pos1=0;
    while(pos1<tot_len){
        char c=candidate[pos1];
        if(c=='<'){
            size_t pos2=pos1+1;
            char next_c=candidate[pos2];
            if(next_c=='='){ // 是小于等于号
                unit.addChild(RUnit(RGTYPE::RTERMINAL,"<="));
                pos1=pos2+1;
            }
            else if(next_c=='>'){ // 是不等于号
                unit.addChild(RUnit(RGTYPE::RTERMINAL,"<>"));
                pos1=pos2+1;
            }
            else{
                // 此时对应两种情况：1.是非终结符 2.是小于号
                while(pos2<tot_len){
                    next_c=candidate[pos2];
                    if(next_c=='>') { // 证明是非终结符
                        std::string non_terminal_name = candidate.substr(pos1 + 1, pos2 - pos1 - 1);
                        unit.addChild(RUnit(RGTYPE::RNON_TERMINAL, non_terminal_name));
                        pos1 = pos2 + 1;
                        break;
                    }
                    pos2++;
                }
                if(pos2>=tot_len){ // 证明是小于号
                    unit.addChild(RUnit(RGTYPE::RTERMINAL,"<"));
                    pos1++;
                }
            }
        }
        else if(c==' '){
            pos1++;
        }
        else{
            size_t pos2=pos1+1;
            while (pos2 < tot_len && candidate[pos2] != ' ' && candidate[pos2] != '\n'&&candidate[pos2] != '<') {
                pos2++;
            }
            std::string terminal_name = candidate.substr(pos1, pos2 - pos1);
            unit.addChild(RUnit(RGTYPE::RTERMINAL, terminal_name));
            pos1 = pos2;
        }
    }

    if(!unit.getUnits().empty()){
        unit.setType(RGTYPE::RMULTI);
        unit.setName(unit.getUnitsName());
    }
    return unit;
}

template<typename T>
void GraAna::RGraAna::eleminate_repeatable(std::vector<T> &vec) {
    std::set<T> s(vec.begin(),vec.end());
    vec.assign(s.begin(),s.end());
}
