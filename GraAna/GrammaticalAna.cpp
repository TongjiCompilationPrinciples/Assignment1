//
// Created by Mac book pro on 2023/12/30.
//
#include "GrammaticalAna.h"
namespace GraAna{
    void Grammer::read_grammer() {
        std::ifstream file(filename);

        std::string line;
        std::regex rule_regex("<([^]+)>→([^\\n]+)"); // 用于匹配并解析文法规则的左侧和右侧。
        // std::regex rule_regex("<(.+?)>→(.+)"); // 这里.+?表示非贪婪匹配任意字符，直到遇到>为止，确保我们只匹配非终结符的名字。

        // 这里\$$.+?\$$和\\{.+?\\}分别用来匹配可选项和可重复项，<.+?>用来匹配非终结符，\\S+用来匹配终结符和其他符号。
        std::smatch match;
        while(std::getline(file,line)){
            if(std::regex_search(line,match,rule_regex)){
                std::string non_terminal = match[1].str();
                std::string production = match[2].str();
                // 删除左右的所有空格
                non_terminal = std::regex_replace(non_terminal, std::regex("\\s+"), ""); // 可选
                production = std::regex_replace(production, std::regex("\\s+"), ""); // 可选
                std::cout<<"---------"<<std::endl;
                std::cout<<non_terminal<<std::endl;
                size_t _pos1=0;
                while(_pos1<production.length()){
                    char c=production[_pos1];
                    if(c=='<'){
                        size_t _pos2=_pos1+1;
                        if(_pos2>=production.length()){
                            throw std::runtime_error("文法规则格式错误");
                        }
                        char next_c=production[_pos2];
                        if(next_c=='='||next_c=='>'){
                            _pos1+=2; // 跳过<=和<>
                        }
                        else{
                            // 如果是非终结符，我们需要找到它的结束标记。
                            while (_pos2 < production.length() && production[_pos2] != '>') {
                                _pos2++;
                            }
                            if(_pos2==production.length()){
                                throw std::runtime_error("文法规则格式错误");
                            }
                            else{
                                std::string _non_terminal = production.substr(_pos1+1, _pos2 - _pos1-1);
                                // 将非终结符替换为终结符

                                if(_map.find(_non_terminal)!=_map.end()){
                                    std::cout<<"找到的非终结符: "<<_non_terminal<<std::endl;
                                    std::cout<<"替换前: "<<production<<std::endl;
                                    production.replace(_pos1,_pos2-_pos1+1,_map[_non_terminal]);
                                    std::cout<<"替换后: "<<production<<std::endl;
                                    _pos1+=_map[_non_terminal].length();
                                }
                                else{
                                    _pos1=_pos2+1;
                                }
                            }
                        }
                    }
                    else{
                        _pos1++;
                    }

                }
                _map[non_terminal]=production; // 将非终结符和它的候选式存储到_map中


                std::cout<<"最终替换后的语法："+non_terminal+"→"+production<<std::endl;
                RHS rhs;
                std::vector<std::string> alternatives = extract_candidates(production);
                for (const auto& alt : alternatives) {
                    std::cout << "Alternative: " << alt << std::endl;
                    // 现在得到了候选式，我们需要将它们分解为单个单元。
                    Candidate candidate;
                    candidate.unit = build_units(alt);
                    std::cout<<"Candidate: "<<candidate.unit.toString()<<std::endl<<std::endl;
                    rhs.candidates.push_back(candidate);
                }
                non_terminals.insert(non_terminal); // 将非终结符添加到non_terminals中。
                productions[non_terminal] = rhs;


            }
            else{
                throw std::runtime_error("文法规则格式错误");
            }
        }
        // eliminate_left_recursion();
        file.close();

    }

    std::vector<std::string> Grammer::extract_candidates(const std::string &rhs) {
        return parse_alternatives(rhs, "");
    }

    std::vector<std::string> Grammer::parse_alternatives(const std::string& segment, const std::string& prefix) {
        std::vector<std::string> alternatives;
        size_t i = 0;
        size_t segment_length = segment.length();
        std::string buffer = prefix;

        while (i < segment_length) {
            char c = segment[i];
            if (c == '[' || c == '{') {
                char closing_char = (c == '[') ? ']' : '}';
                int depth = 1;
                size_t start_index = i + 1;
                while (i < segment_length && depth > 0) {
                    i++;
                    if (segment[i] == c) depth++;
                    else if (segment[i] == closing_char) depth--;
                }
                if (depth == 0) { // 如果深度为0，说明找到了匹配的右括号或右花括号。
                    std::string inner_segment = segment.substr(start_index, i - start_index);
                    std::vector<std::string> inner_alternatives = parse_alternatives(inner_segment, "");// 递归调用,得到内部的候选式
                    size_t pos=std::min(i+1,segment_length);
                    // 求出下一个候选式的开始的位置，在它到|之前应该没有左花括号
                    int _depth=0;
                    while(pos<segment_length){
                        if(segment[pos]=='{'||segment[pos]=='['){
                            _depth++;
                        }
                        else if(segment[pos]=='}'||segment[pos]==']'){
                            _depth--;
                        }
                        else if(segment[pos]=='|' && _depth==0){
                            break;
                        }
                        pos++;
                    }
                    // 给内部的候选式添加前缀
                    for (auto& alt : inner_alternatives) {
                        alt = buffer + c + alt + closing_char;
                        // 它们可能作为前缀的一部分，也可能是当前候选式的一部分。继续处理右花括号到下一个候选式的开始之前的部分.

                        // 如果pos==segment_length，说明没有找到|，那么pos就是segment_length
                        // 如果pos<segment_length，说明找到了|，那么pos就是|的位置
                        // 剩余的同级候选式
                        auto remaining_alternatives = parse_alternatives(segment.substr(i+1,pos-i-1), alt);
                        // 与当前项同级的下一项。
                        alternatives.insert(alternatives.end(), remaining_alternatives.begin(), remaining_alternatives.end());
                    }
                    // 跳到pos的位置
                    i=pos+1;
                    if(i>=segment_length){
                        return alternatives;
                    }
                    continue;

                } else {
                    throw std::runtime_error("Mismatched brackets or braces in grammar");
                }
            } else if (c == '|') {
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

    Unit Grammer::build_units(const std::string &rhs) {
        size_t i = 0;
        size_t alt_length = rhs.length();
        Unit unit(GRAMMAR, rhs);
        while(i<alt_length){
            char c = rhs[i];
            if(c=='<'){
                // 如果是非终结符，我们需要找到它的结束标记。
                size_t start_index = i + 1;
                while (i < alt_length && rhs[i] != '>') {
                    i++;
                }
                if (i == alt_length) {
                    // 找不到的话，它是一个小于符号或小于等于符号或不等于号，我们检查它是否是一个小于等于符号或不等于号。
                    if(rhs[start_index]=='='){
                        unit.addUnit(Unit(TERMINAL, "<="));
                        i=start_index;
                    }
                    else if(rhs[start_index]=='>'){
                        unit.addUnit(Unit(TERMINAL, "<>"));
                        i=start_index;
                    }
                    else{
                        unit.addUnit(Unit(TERMINAL, "<"));
                        i=start_index-1;
                    }
                }
                else{
                    std::string non_terminal = rhs.substr(start_index, i - start_index);
                    unit.addUnit(Unit(NON_TERMINAL, non_terminal));// 因为是非终结符，里面不包含其它单元
                }
            }
            else if(c=='{'){
                // 如果是可重复项，我们需要找到它的结束标记。
                size_t start_index = i + 1;
                while (i < alt_length && rhs[i] != '}') {
                    i++;
                }
                if (i == alt_length) {
                    throw std::runtime_error("Mismatched brackets or braces in grammar");
                }
                // 对括号内的内容进行构建

                std::string non_terminal = rhs.substr(start_index, i - start_index);
                Unit _unit= build_units(non_terminal);
                _unit.setType(REPEATABLE);
                unit.addUnit(_unit);
            }
            else if(c=='['){
                // 如果是可选项，我们需要找到它的结束标记。
                size_t start_index = i + 1;
                while (i < alt_length && rhs[i] != ']') {
                    i++;
                }
                if (i == alt_length) {
                    throw std::runtime_error("Mismatched brackets or braces in grammar");
                }
                std::string non_terminal = rhs.substr(start_index, i - start_index);
                Unit _unit= build_units(non_terminal);
                _unit.setType(OPTIONAL);
                unit.addUnit(_unit);
            }
            else{
                // 如果是终结符，我们只需要将它添加到候选式中。这个单词可能是个单词，也可能是个符号，所以我们找到空格或换行符或者左括号
                // 截取之前的部分
                size_t start_index = i;
                while (i < alt_length && rhs[i] != ' ' && rhs[i] != '\n'&& rhs[i] != '['&& rhs[i] != '{'&& rhs[i] != '<') {
                    i++;
                }
                std::string terminal = rhs.substr(start_index, i - start_index);
                unit.addUnit(Unit(TERMINAL, terminal));
                if(!(rhs[i]==' '||rhs[i]=='\n')){
                    i--;
                }
            }
            i++;
        }
        return unit;
    }

    void Grammer::eliminate_left_recursion() {
        // 先进行非终结符的排序
        std::cout<<"开始进行非终结符排序"<<std::endl;
        std::vector<std::string> non_terminals_sorted(non_terminals.size());
        std::copy(non_terminals.begin(), non_terminals.end(), non_terminals_sorted.begin());
        std::sort(non_terminals_sorted.begin(), non_terminals_sorted.end(), [](const std::string& a, const std::string& b) {
            return a.length() < b.length();
        });
        std::cout<<"排序结果: "<<std::endl;
        for (const auto& non_terminal : non_terminals_sorted) {
            std::cout << non_terminal << std::endl;
        }
        std::cout<<std::endl;
        std::cout<<"开始进行非终结符置换"<<std::endl;
//        for(size_t i=1;i<non_terminals_sorted.size();++i){
//            for(size_t j=0;j<i;++j){
//                std::string A=non_terminals_sorted[i]; // A->B
//                std::string B=non_terminals_sorted[j];
//                auto rhs_A=productions[A].candidates;
//                auto  rhs_B=productions[B].candidates;
//                // 查看A的左侧的候选式是否有B
//                for(auto& candidate_A:rhs_A){
//                   // a的候选式包含多个单元，需要遍历每个单元
//                    for(auto& unit:candidate_A.unit.getUnits()){
//                        // 由于单元的递归性，需要递归地进行替换
//
//                    }
//                }
//            }
//        }

    }

    std::vector<Candidate>  Grammer::replace_unit(Unit &unit, const std::string &non_terminal) {
        // 先拿到non_terminal的候选式
        auto rhs=productions[non_terminal].candidates;
        if(unit.getType()==NON_TERMINAL){
            if(unit.getName()==non_terminal){
                // 如果是匹配的非终结符，直接替换
                return rhs;
            }
            else{
                // 如果不是匹配的非终结符，直接返回
                return {};
            }
        }
        else{
            // 递归地替换，如果替换成功，就将替换后的候选式添加到unit中，并删除原来的单元
            std::vector<Unit> units=unit.getUnits();
            std::vector<Candidate> candidates;
            // 为了避免运行时删除元素导致的迭代器失效，我们先将要删除的元素的下标存储起来
            std::vector<size_t> indexs;
            for(size_t i=0;i<units.size();++i){
                Candidate candidate;
                auto _rhs=replace_unit(units[i],non_terminal);
                if(!_rhs.empty()){
                    indexs.push_back(i);
                    // 替换成功，将替换后的候选式添加到unit中
                    for(auto& candidate:_rhs){
                        unit.addUnit(candidate.unit);
                    }
                }
            }
            // 倒着删除
            for(auto iter=indexs.rbegin();iter!=indexs.rend();++iter){
                unit.deleteUnit(*iter);
            }
            return candidates;

        }
    }

    GTYPE Unit::getType() const {
        return type;
    }

    const std::string &Unit::getName() const {
        return name;
    }

    void Unit::setType(GTYPE type) {
        Unit::type = type;
    }

    void Unit::setName(const std::string &name) {
        Unit::name = name;
    }

    std::string Unit::toString() const {
        if(units.empty()){
            switch (type){
                case TERMINAL:
                    return name;
                case NON_TERMINAL:
                    return "<"+name+">";
                case OPTIONAL:
                    return "["+name+"]";
                case REPEATABLE:
                    return "{"+name+"}";
                case GRAMMAR:
                    return name;
                default:
                    return name;
            }
        }
        else{
            std::string str;
            switch (type){
                case TERMINAL:
                    str=name;
                    break;
                case NON_TERMINAL:
                    str="<"+name+">";
                    break;
                case OPTIONAL:
                    str="["+name+"]";
                    break;
                case REPEATABLE:
                    str="{"+name+"}";
                    break;
                case GRAMMAR:
                    str=name;
                    break;
                default:
                    str="";
                    break;
            }
            str+="->";
            for (const auto& unit : units) {
                str+=unit.toString()+" ";
            }
            return str;
        }
    }

    void Unit::addUnit(const Unit& unit) {
        units.push_back(unit);
    }

    std::vector<Unit> Unit::getUnits() const {
        return units;
    }

    void Unit::deleteUnit(size_t index) {
        if(index>=0 && index<units.size()){
            units.erase(units.begin()+index);
        }
    }

    std::fstream &Unit::operator<<(std::fstream &out) const {
        out<<toString();
        return out;
    }

}

