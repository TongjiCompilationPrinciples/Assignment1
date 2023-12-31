//
// Created by Mac book pro on 2023/12/30.
//

#ifndef COMPILING_GRAMMATICALANA_H
#define COMPILING_GRAMMATICALANA_H

#include <string>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <fstream>
#include <regex>
#include <iostream>
namespace GraAna{
    enum GTYPE{
        TERMINAL, // 终结符
        NON_TERMINAL, // 非终结符
        OPTIONAL, // 可选
        REPEATABLE, // 可重复
        GRAMMAR, // 文法
    };
    class Unit{
        private :
            GTYPE type;
            std::string name;
            std::vector<Unit> units;
        public:
            Unit(GTYPE type, std::string  name):type(type),name(std::move(name)){};
            Unit()= default;
            [[nodiscard]] GTYPE getType() const;
            [[nodiscard]] const std::string &getName() const;
            void setType(GTYPE type);
            void setName(const std::string &name);
            [[nodiscard]] std::string toString() const;
            void addUnit(const Unit& unit);
            [[nodiscard]] std::vector<Unit> getUnits() const;
            void deleteUnit(size_t index);
            std::fstream& operator<<(std::fstream& out) const;
    };
    // 候选式
    class Candidate{
    public:
        Unit unit;
    };
    struct RHS{ // 产生式右侧的各个部分
        std::vector<Candidate> candidates; // 各个候选式
    };
    class Grammer{ // 存储文法的类
    private :
        const std::string& filename; // 存储文法的文件名
        std::vector<std::string> extract_candidates(const std::string& rhs);
        std::vector<std::string> parse_alternatives(const std::string& segment,const std::string& prefix);
        std::map<std::string,std::string> _map;// 用于存储非终结符和终结符的映射
        std::map<std::string,std::set<std::string>> first_set;// first集
        std::map<std::string,std::set<std::string>> follow_set;// follow集
        Unit build_units(const std::string& rhs);
        static std::string longest_common_prefix(const std::string& str1,const std::string& str2);
        template<typename T>
        static void eleminate_repeatable(std::vector<T>&vec);
        std::string find_most_common_refix(const std::vector<std::string>& strings);// 找到该集合拥有最多候选式的前缀
        std::vector<std::string> extract_left_factors(std::vector<std::string> candidates);// 提取左因子
    public:
        std::map<std::string, RHS> productions; // 产生式
        std::set<std::string> non_terminals; // 非终结符
        std::set<std::string> terminals; // 终结符
        void read_grammer();
        void calculate_first_set();
        void calculate_follow_set();
        explicit Grammer(const std::string& filename):filename(filename){};
    };
}
#endif //COMPILING_GRAMMATICALANA_H
