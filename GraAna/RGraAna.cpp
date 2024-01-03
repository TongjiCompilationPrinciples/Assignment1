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
            case RGTYPE::RTERMINAL:
                return this->name;
            case RGTYPE::RNON_TERMINAL:
                return "<"+this->name+">";
            case RGTYPE::RMULTI:
                return "("+this->name+")"; // TODO: 复合类型的输出
            case RGTYPE::REMPTY:
                return "ε";
            default:
                return this->name;
        }
    }
    else{
        switch (this->type) {
            case RGTYPE::RTERMINAL:
                return this->getUnitsName();
            case RGTYPE::RNON_TERMINAL:
                return "<"+this->getUnitsName()+">";
            case RGTYPE::RMULTI:
                return "("+this->getUnitsName()+")"; // TODO: 复合类型的输出
                case RGTYPE::REMPTY:
                return "ε";
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
            eleminate_repeatable(alternatives);
            for(auto& alternative:alternatives){
                size_t pos1=0;
                std::set<std::string> names;// 存放当前候选式中的非终结符
                while(pos1<alternative.length()){
                    size_t pos2=pos1;
                    std::string non_terminal_name=GraAna::RGraAna::getNonTerminalName(alternative,pos2);// non_terminal_name 是有尖括号的
                    if(!non_terminal_name.empty()){
                        non_terminal_name= eleminate_NonTerminalAngleBrackets(non_terminal_name); // 去掉尖括号
                        names.insert(non_terminal_name);
                        pos1=pos2+1;
                    }
                    else{
                        pos1=alternative.length(); // 证明该候选式中没有非终结符
                    }
                }
                for(auto&name:names){
                    alternative=replaceNonTerminal(alternative,name);
                }
            }
            production="";
            for(auto& alternative:alternatives){
                production+=alternative+"|";
            }
            production=production.substr(0,production.length()-1);
            std::cout<<"替换后的产生式："<<production<<std::endl;

            // 开始进行消除左递归
            production=eleminate_left_recursion(non_terminal,production);
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
            auto _terminal_name = std::regex_replace(terminal_name, std::regex("\\s+"), "");
            if(_terminal_name=="ε"){
                unit.addChild(RUnit(RGTYPE::REMPTY,""));
            }
            else{
                unit.addChild(RUnit(RGTYPE::RTERMINAL, terminal_name));
            }
            pos1 = pos2;
        }
    }

    if(unit.getUnits().size()>1){
        unit.setType(RGTYPE::RMULTI);
        unit.setName(unit.getUnitsName());
    }
    else{
        unit=unit.getUnits()[0];
    }
    return unit;
}

std::string
GraAna::RGraAna::replaceNonTerminal(const std::string &production, const std::string& other) {
    // 先拿到other的候选式
    std::string other_production=_map[other];
    if(other_production.empty()){
        std::clog<<other<<"对应的产生式不存在"<<std::endl;
        return production;
    }
    std::string match="<"+other+">"; // 要替换的非终结符
    if(production.find(match)==std::string::npos){
        std::clog<<production<<"中不存在"<<match<<std::endl;
        return production;
    }
    auto alternatives = extractCandidates(other_production);
    if(alternatives.size()==1){
        // 如果只有一个候选式，就直接替换
        std::string new_production;
        new_production=std::regex_replace(production,std::regex(match),alternatives[0]);
        return new_production;
    }

    // 有多个候选式
    std::string new_production;
    for(auto& alt:alternatives){
        // 拿到前缀，即match之前的部分
        std::string prefix=production.substr(0,production.find(match));
        // 拿到后缀，即match之后的部分
        std::string suffix=production.substr(production.find(match)+match.length());
        new_production=prefix+alt+replaceNonTerminal(suffix,other)+"|";
    }
    // 去掉最后一个|
    new_production=new_production.substr(0,new_production.length()-1);
    return new_production;

}

std::string GraAna::RGraAna::eleminate_NonTerminalAngleBrackets(const std::string &rhs) {
    return std::regex_replace(rhs,std::regex("<|>"),"");
}

std::string GraAna::RGraAna::eleminate_left_recursion(const std::string &name, const std::string &production) {
    std::cout<<"开始进行消除左递归"<<std::endl;
    auto alternatives=extractCandidates(production);
    eleminate_repeatable(alternatives);
    size_t pos1;
    std::set<std::string> alpha; // 存放当前非终结符开头的候选式（左递归下的候选式剩余部分）
    std::set<std::string> beta; // 存放非当前非终结符开头的候选式(非左递归的候选式)
    bool existLeftRecursion=false;
    for(auto&alt:alternatives){
        pos1=0;
        bool isLeftRecursion=false; // 当前候选式是否是左递归
        std::cout<<"当前候选式："<<alt<<std::endl;

        std::string _alt=std::regex_replace(alt, std::regex("\\s+"), "");// 消除多余空格后的候选式
        if(_alt[0]=='<'){
            std::string non_terminal_name=GraAna::RGraAna::getNonTerminalName(_alt,pos1);
            non_terminal_name= eleminate_NonTerminalAngleBrackets(non_terminal_name); // 去掉尖括号
            if(non_terminal_name==name){
                existLeftRecursion=isLeftRecursion=true;
                std::cout<<"发现左递归！"<<std::endl;
                std::cout<<"当前候选式的剩余部分："<<_alt.substr(pos1+1)+"，将其加入alpha集合"<<std::endl;
                alpha.insert(_alt.substr(pos1+1));
            }
        }
        if(!isLeftRecursion){ // 当前候选式不是左递归
            beta.insert(_alt);
        }
    }
    RRHS rhs;
    if(existLeftRecursion){ // name存在左递归
        std::string new_non_terminal=name+"'";//
        // 对应的新的候选式
        std::string new_production;
        for(auto& alt:alpha){
            new_production+=alt+"<"+new_non_terminal+">|";
        }
        // 加入空集符号
        new_production+="ε";
        std::cout<<"新增加的文法规则为：<"+new_non_terminal+">→"+new_production<<std::endl;
        // 将新的非终结符和新的候选式添加到_map中
        _map[new_non_terminal]=new_production;
        RRHS _rhs; // 构建新增加的文法规则的右部
        auto new_alternatives = extractCandidates(new_production);
        eleminate_repeatable(new_alternatives);
        for (const auto& alt : new_alternatives){
            auto unit=buildUnits(alt);
            _rhs.candidates.push_back(unit);
        }
        productions[new_non_terminal]=_rhs; // 将新增加的文法规则添加到productions中
        std::string original_production="";
        for(auto& alt:beta){
            std::string c="";
            if(alt=="ε"){
                c="<"+new_non_terminal+">";
            }
            else{
                c=alt+"<"+new_non_terminal+">";
            }
            auto unit=buildUnits(c);
            rhs.candidates.push_back(unit);
            original_production+=c+"|";
        }
        original_production=original_production.substr(0,original_production.length()-1);
        _map[name]=original_production;
        productions[name]=rhs;
        std::cout<<"存在左递归，并且原先的产生式替换为：<"+name+">→"+original_production<<std::endl;
        return original_production;
    }
    else{ // name不存在左递归
        alternatives = extractCandidates(production);
        eleminate_repeatable(alternatives);
        for (const auto& alt : alternatives){
            auto unit=buildUnits(alt);
            rhs.candidates.push_back(unit);
        }
        _map[name]=production;
        productions[name]=rhs;
        std::cout<<"不存在左递归, 产生式保持为：<"+name+">→"+production<<std::endl;
        return production;
    }

}

std::set<std::string> GraAna::RGraAna::cal_first(RUnit unit,int depth) {
    if(depth>MAX_REC_DEPTH){
        std::cerr<<"递归深度过大"<<std::endl;
        return {};
    }
    if(unit.getType()==RGTYPE::REMPTY){
        // return first[unit.getName()]={"ε"}; // 不能这么写，因为getName为空字符串
        return {"ε"};
    }
    std::set<std::string> res;
    if(unit.getType()==RGTYPE::RTERMINAL){
        res.insert(unit.getName().substr(0,1));
        first[unit.getName()]=res;
        return res;
    }
    if(unit.getType()==RGTYPE::RNON_TERMINAL){
        const std::string& name=unit.getName();
        if(first.find(name)!=first.end()){
            return first[name];
        }
        else{
            std::string _name="<"+name+">";
            std::set<std::string> _first;
            if(canBeEmpty(unit, 0)){
                _first.insert("ε");
            }
            for(auto& candidate:productions[name].candidates){
                auto _temp=cal_first(candidate,depth+1);
                _first.insert(_temp.begin(),_temp.end());
            }
            first[name]=_first;
            return _first;
        }
    }
    if(unit.getType()==RGTYPE::RMULTI){
        for(auto& child:unit.getUnits()){
            auto _temp=cal_first(child,depth+1);
            if(canBeEmpty(child, 0)){
                res.insert(_temp.begin(),_temp.end());
                continue;
            }
            else{
                res.insert(_temp.begin(),_temp.end());
                break;
            }
        }
        first[unit.getName()]=res;
        return res;
    }
    return {};
}

void GraAna::RGraAna::display() {
    for(auto&[name,rhs]:productions){
        std::string out="<"+name+">→";
        for(auto& candidate:rhs.candidates){
            out+=candidate.tostring()+"|";
        }
        out=out.substr(0,out.length()-1);
        std::cout<<out<<std::endl;
        std::cout<<"can be empty? "<<(canBeEmpty(name)?"yes":"no")<<std::endl;
        std::cout<<"---------------"<<std::endl;
    }
}

bool GraAna::RGraAna::canBeEmpty(const GraAna::RUnit &unit,int depth) {
    if(emptyCache.find(unit.getName())!=emptyCache.end()){
        return emptyCache[unit.getName()];
    }
    if(depth>MAX_REC_DEPTH){
        return emptyCache[unit.getName()]=false;
    }
    if(unit.getType()==RGTYPE::REMPTY){

        return emptyCache[unit.getName()]=true;
    }
    if(unit.getType()==RGTYPE::RTERMINAL){
        auto name=unit.getName();
        name=std::regex_replace(name,std::regex("\\s+"),"");
        if(name=="ε"||name.empty()){
            return emptyCache[unit.getName()]=true;
        }
        else{
            return emptyCache[unit.getName()]=false;
        }
    }
    if(unit.getType()==RGTYPE::RNON_TERMINAL){
        const std::string& name=unit.getName();
        auto production=productions[name];
        return emptyCache[unit.getName()]=std::any_of(production.candidates.begin(),production.candidates.end(),[&](const auto& candidate){
            return canBeEmpty(candidate,depth+1);
        });
    }
    if(unit.getType()==RGTYPE::RMULTI){
       for(auto& child:unit.getUnits()){
           if(!canBeEmpty(child,depth+1)){
               return emptyCache[unit.getName()]=false;
           }
       }
       return emptyCache[unit.getName()]=true;
    }
    return emptyCache[unit.getName()]=false;
}

bool GraAna::RGraAna::canBeEmpty(const std::string &name) {
    auto production=productions[name];
    return std::any_of(production.candidates.begin(),production.candidates.end(),[&](const auto& candidate){
        return canBeEmpty(candidate,0);
    });
}

void GraAna::RGraAna::cal_all_first() {
    for(auto&[name,rhs]:productions){
        for(auto& candidate:rhs.candidates){
            auto _temp=cal_first(candidate,0);
            first[name].insert(_temp.begin(),_temp.end());
        }
    }
    for(auto&[name,rhs]:productions){
        std::string out="first("+name+")={";
        if(first[name].empty()){
            out+="}";
            std::cout<<out<<std::endl;
            continue;
        }
        else{
            for(auto& item:first[name]){
                out+=item+",";
            }
            out=out.substr(0,out.length()-1);
            out+="}";
            std::cout<<out<<std::endl;
        }
        for(auto& candidate:rhs.candidates){
            std::cout<<"\tfirst("+candidate.tostring()+")={";
            for(auto& item:first[candidate.getName()]){
                std::cout<<item<<",";
            }
            std::cout<<"}"<<std::endl;

        }
    }
}

std::set<std::string> GraAna::RGraAna::cal_follow(GraAna::RUnit unit, int depth) {

}

template<typename T>
void GraAna::RGraAna::eleminate_repeatable(std::vector<T> &vec) {
    std::set<T> s(vec.begin(),vec.end());
    vec.assign(s.begin(),s.end());
}
