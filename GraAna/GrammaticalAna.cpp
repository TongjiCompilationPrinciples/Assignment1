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
                non_terminals.insert(non_terminal); // 将非终结符添加到non_terminals中。
                RHS rhs;
                std::vector<std::string> alternatives = extract_candidates(production);
                for (const auto& alt : alternatives) {
                    std::cout << "Alternative: " << alt << std::endl;
                    // 现在得到了候选式，我们需要将它们分解为单个单元。
                    Candidate candidate;
                    candidate.unit = build_units(alt);
                    rhs.candidates.push_back(candidate);
                }
                productions[non_terminal] = rhs;


            }
            else{
                throw std::runtime_error("文法规则格式错误");
            }
        }
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
        Unit unit;
        unit.setType(GRAMMAR);
        while(i<alt_length){
            char c = rhs[i];
            if(c=='<'){
                // 如果是非终结符，我们需要找到它的结束标记。
                size_t start_index = i + 1;
                while (i < alt_length && rhs[i] != '>') {
                    i++;
                }
                if (i == alt_length) {
                    // 找不到的话，它是一个小于符号
                    unit.addUnit(Unit(TERMINAL, std::string(1, c)));
                    // 恢复i指针
                    i=start_index-1;
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
                // 如果是终结符，我们只需要将它添加到候选式中。
                unit.addUnit(Unit(TERMINAL, std::string(1, c)));
            }
            i++;
        }
        return unit;
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
        switch (type){
            case TERMINAL:
                return name;
            case NON_TERMINAL:
                return "<"+name+">";
            case OPTIONAL:
                return "("+name+")";
            case REPEATABLE:
                return "{"+name+"}";
            default:
                return "";
        }
    }

    void Unit::addUnit(const Unit& unit) {
        units.push_back(unit);
    }

    std::vector<Unit> Unit::getUnits() const {
        return units;
    }

    void Unit::deleteUnit(int index) {
        if(index>=0 && index<units.size()){
            units.erase(units.begin()+index);
        }
    }
}

