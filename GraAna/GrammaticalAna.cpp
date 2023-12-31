//
// Created by Mac book pro on 2023/12/30.
//
#include "GrammaticalAna.h"
namespace GraAna{
    void Grammer::read_grammer() {
        std::ifstream file(filename);

        std::string line;
        //std::regex rule_regex("<([^]+)>→([^\\n]+)"); // 用于匹配并解析文法规则的左侧和右侧。
         std::regex rule_regex("<(.+?)>→(.+)"); // 这里.+?表示非贪婪匹配任意字符，直到遇到>为止，确保我们只匹配非终结符的名字。

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
                        if(_pos2<production.length()){
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
                                        std::vector<std::string> _alternatives = extract_candidates(_map[_non_terminal]);
                                        // 给替换后的候选式添加前缀和后缀
                                        // 如果候选式之前是|，就不用添加前缀了
                                        // 如果候选式之后是|，就不用添加后缀了
                                        bool isneed_prefix=true;
                                        bool isneed_suffix=true;
                                        if(_pos1>0&&production[_pos1-1]=='|'){
                                            isneed_prefix=false;
                                        }
                                        if(_pos2+1<production.length() && production[_pos2+1]=='|'){
                                            isneed_suffix=false;
                                        }
                                        for (auto& alt : _alternatives) {
                                            if(isneed_prefix){
                                                alt = production.substr(0,_pos1)+alt;
                                            }
                                            if(isneed_suffix){
                                                alt += production.substr(_pos2+1);
                                            }
                                        }

                                        // 用|将替换后的候选式连接起来
                                        std::string _production;
                                        // 如果没有添加前缀，则把原来的前缀加上
                                        if(!isneed_prefix){
                                            _production=production.substr(0,_pos1);
                                        }
                                        for(const auto& alt:_alternatives){
                                            _production+=alt+"|";
                                        }
                                        size_t _new_pos1=_production.length();
                                        // 如果没有添加后缀，则把原来的后缀加上
                                        if(!isneed_suffix){
                                            _production+=production.substr(_pos2+2);
                                        }
                                        // 如果最后一个字符是|，还要删去
                                        if(_production[_production.length()-1]=='|'){
                                            _production=_production.substr(0,_production.length()-1);
                                        }
                                        production=_production;
                                        std::cout<<"替换后: "<<production<<std::endl;
                                        // 设置_pos1的位置
                                        _pos1=_new_pos1;
                                    }
                                    else{
                                        _pos1=_pos2+1;
                                    }
                                }
                            }
                        }

                    }
                    else{
                        _pos1++;
                    }

                }
                std::cout<<"最终替换后的语法："+non_terminal+"→"+production<<std::endl;
                RHS rhs;
                std::vector<std::string> alternatives = extract_candidates(production);
                // 对候选式去重
                eleminate_repeatable(alternatives);
                std::set<std::string> alpha; // 存放当前非终结符开头的候选式（左递归下的候选式剩余部分）
                std::set<std::string> beta; // 存放非当前非终结符开头的候选式
                bool flag=false;
                for (const auto& alt : alternatives) {
                    std::cout << "Alternative: " << alt << std::endl;
                    // 存放非当前非终结符开头的候选式
                    _pos1=0;
                    bool _flag=false;
                    char c=alt[_pos1];
                    if(c=='<'){
                        size_t _pos2=_pos1+1;
                        if(_pos2<alt.length()){ // 如果不是最后一个字符
                            char next_c=alt[_pos2];
                            if(next_c!='='&&next_c!='>'){
                                // 如果是非终结符，我们需要找到它的结束标记。
                                _pos2++;
                                while (_pos2 < alt.length() && alt[_pos2] != '>') {
                                    _pos2++;
                                }
                                if(_pos2==alt.length()){
                                    throw std::runtime_error("文法规则格式错误2");
                                }
                                else{
                                    std::string _non_terminal = alt.substr(_pos1+1, _pos2 - _pos1-1);
                                    if(_non_terminal==non_terminal){// 左递归
                                        std::cout<<"发现左递归！"<<std::endl;
                                        flag=_flag=true;
                                        alpha.insert(alt.substr(_pos2+1)); // 将剩余部分添加到alpha中
                                    }
                                    else{
                                        // 不是当前非终结符开头的候选式
                                    }
                                }
                            }
                        }
                    }
                    if(!_flag){ // 当前候选式不是左递归
                        beta.insert(alt);
                    }
                }
                if(!flag){ // 没有左递归
                    alternatives = extract_candidates(production);
                    // 对候选式去重
                    eleminate_repeatable(alternatives);
                    // 提取候选式的公共左因子，确保所有候选首部集两两不相交


                    for (const auto& alt : alternatives) {
                        Candidate candidate;
                        candidate.unit = build_units(alt);
                        std::cout<<"Candidate: "<<candidate.unit.toString()<<std::endl<<std::endl;
                        rhs.candidates.push_back(candidate);
                    }
                }
                else{
                    // 有左递归
                    // 添加新的非终结符
                    std::string new_non_terminal=non_terminal+"'";
                    // 对应的新的候选式
                    std::string new_production;
                    for(const auto& alt:alpha){
                        new_production+=alt+"<"+new_non_terminal+">|";
                    }
                    // 加入空集符号
                    new_production+="ε";
                    // 将新的非终结符和新的候选式添加到_map中
                    _map[new_non_terminal]=new_production;
                    std::cout<<"新增加的文法规则为："+new_non_terminal+"→"+new_production<<std::endl;
                    RHS new_rhs;
                    std::vector<std::string> new_alternatives = extract_candidates(new_production);
                    // 对候选式去重
                    eleminate_repeatable(new_alternatives);
                    for (const auto& alt : new_alternatives) {
                        Candidate candidate;
                        candidate.unit = build_units(alt);
                        std::cout<<"Candidate: "<<candidate.unit.toString()<<std::endl<<std::endl;
                        new_rhs.candidates.push_back(candidate);
                    }
                    non_terminals.insert(new_non_terminal);
                    productions[new_non_terminal]=new_rhs;
                    // 将beta中的候选式添加到当前非终结符的候选式中
                    production="";
                    for(const auto& alt:beta){
                        Candidate candidate;
                        candidate.unit = build_units(alt+"<"+new_non_terminal+">");
                        std::cout<<"Candidate: "<<candidate.unit.toString()<<std::endl<<std::endl;
                        rhs.candidates.push_back(candidate);
                        production+=alt+"<"+new_non_terminal+">|";
                    }
                    production=production.substr(0,production.length()-1);

                }
                non_terminals.insert(non_terminal); // 将非终结符添加到non_terminals中。
                productions[non_terminal] = rhs;
                _map[non_terminal]=production; // 将非终结符和它的候选式存储到_map中
            }
            else{
                continue;
                //throw std::runtime_error("文法规则格式错误3");
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
                    size_t pos=i+1;
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
                    buffer=prefix;
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


    void Grammer::calculate_first_set() {

    }

    void Grammer::calculate_follow_set() {

    }

    std::string Grammer::longest_common_prefix(const std::string &str1, const std::string &str2) {
        size_t minLength = std::min(str1.length(), str2.length());
        for (int i = 0; i < minLength; ++i) {
            if (str1[i] != str2[i]) {
                return str1.substr(0, i);
            }
        }
        return str1.substr(0, minLength);
    }


    std::string Grammer::find_most_common_refix(const std::vector<std::string> &strings) {
        if (strings.empty()) return "";
        std::string prefix = strings[0];
        for (const std::string& str : strings) {
            prefix = longest_common_prefix(prefix, str);
            if (prefix.empty()) break;
        }
        return prefix;
    }

    std::vector<std::string> Grammer::extract_left_factors(std::vector<std::string> candidates) {
        std::vector<std::string> factoredProductions;// 拥有公共左因子的候选式
        std::map<std::string, std::vector<std::string>> groups;
        while(!candidates.empty()){
            for (const std::string& candidate : candidates) {
                if (!candidate.empty()) {
                    groups[candidate.substr(0, 1)].push_back(candidate);// 以第一个字符为键，将候选式分组
                }
            }
            // 找到拥有最多候选式的最长前缀
            std::string longestPrefix;
            std::vector<std::string>* longestGroup = nullptr;
            for (auto& group : groups) {
                std::string currentPrefix = find_most_common_refix(group.second);
                if (currentPrefix.length() > longestPrefix.length() ||
                    (currentPrefix.length() == longestPrefix.length() && group.second.size() > longestGroup->size())) {
                    longestPrefix = currentPrefix;
                    longestGroup = &group.second;
                }
            }
        }
    }

    template<typename T>
    void Grammer::eleminate_repeatable(std::vector<T> &vec) {
        std::set<T> s(vec.begin(),vec.end());
        vec.assign(s.begin(),s.end());
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
            str+="-> ";
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

