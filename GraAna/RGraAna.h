//
// Created by Mac book pro on 2023/12/31.
//

#ifndef COMPILING_RGRAANA_H
#define COMPILING_RGRAANA_H
#include<string>
#include<vector>
#include<iostream>
#include<fstream>
#include<map>
#include <regex>
#include<set>
namespace GraAna{
    enum class RGTYPE{
        RTERMINAL, // 终结符
        RNON_TERMINAL, // 非终结符
        RMULTI, // 复合类型
        REMPTY // 空
        /*
         * 复合类型的例子：<A><B>ε PROGRAM
         * */
    };
    class [[maybe_unused]] RUnit{
    private:
        RGTYPE type=RGTYPE::REMPTY;
        std::string name; // 名称, 对于非终结符，记录的没有尖括号
        std::vector<RUnit> units; // 复合类型的子元素
    public:
        RUnit(RGTYPE type, std::string name):type(type),name(std::move(name)){};
        RUnit()= default;
        [[nodiscard]] RGTYPE getType() const;
        [[nodiscard]] const std::string &getName() const;
        void setType(RGTYPE type);
        void setName(const std::string &name);
        [[nodiscard]] std::string getUnitsName() const;
        [[nodiscard]] std::string tostring() const;
        void addChild(const RUnit& unit);
        [[nodiscard]] std::vector<RUnit> getUnits() const;
        void deleteChild(size_t index);

    };
    struct RRHS{// 产生式右侧
        std::vector<RUnit> candidates; // 候选式
    };
    class RGraAna {
    private:
        const std::string& filename; // 存储文法的文件名
        std::map<std::string,std::string> _map;// 用于存储非终结符和终结符的映射
        std::map<std::string, RRHS> productions; // 产生式, 注意key没有尖括号
        std::map<std::string, std::set<std::string>> first; // first集
        std::map<std::string, std::set<std::string>> follow; // follow集
    private:
        void readGrammar();
        [[nodiscard]] std::string getNonTerminalName(const std::string& production,bool isHead=true) const; // 获取当前产生式开头的非终结符的名称
        static std::string getNonTerminalName(const std::string& production,size_t& index) ; // 获取当前产生式开头的非终结符的名称，注意这个返回有尖括号
        static std::vector<std::string> extractCandidates(const std::string &rhs);
        static std::vector<std::string> parseAlternatives(const std::string& segment, const std::string& prefix);
        template<typename T> static void eleminate_repeatable(std::vector<T>&vec);
        std::string eleminate_left_recursion(const std::string& name,const std::string &production); // 这个name没有尖括号，一定要注意
        static std::string eleminate_NonTerminalAngleBrackets(const std::string& rhs);
        std::string replaceNonTerminal(const std::string& production,const std::string& other);
        static RUnit buildUnits(const std::string& rhs);
    public:
        explicit RGraAna(const std::string& filename):filename(filename){};
        bool init();
        std::set<std::string> cal_first(RUnit unit,int depth);
        void display();
        bool canBeEmpty(const RUnit& unit);
    };
}




#endif //COMPILING_RGRAANA_H
