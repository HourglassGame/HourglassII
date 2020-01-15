#ifndef HG_TRIGGER_CLAUSE_H
#define HG_TRIGGER_CLAUSE_H

#include "LuaUtilities.h"
#include <iostream>
#include <string.h> 
#include <regex>
#include "mp/std/map"

namespace hg {
enum class TriggerOperator : unsigned int {
    TRIGGER = 0,
    CONSTANT = 1,
    FRAME_NUM = 2,
    NOT = 3,
    AND = 4,
    OR = 5,
    XOR = 6,
    ADD = 7,
    SUBTRACT = 8,
    MULT = 9,
    DIVIDE = 10,
    MOD = 11,
    EQUAL = 12,
    GREATER = 13,
    LESS = 14
};

class TriggerClause {
public:

    std::tuple<std::vector<int>, std::vector<TriggerOperator>, int> processClauseString(std::string &rawClause)
    {
        std::istringstream iss(rawClause);
        std::vector<std::string> splitClause(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

        std::vector<int> clauseValues(splitClause.size());
        std::vector<TriggerOperator> clauseOps(splitClause.size());

        unsigned int evalDepth = 0;
        unsigned int maxEvalDepth = 0;
        for (unsigned int i = 0; i < splitClause.size(); ++i) {
            if (std::regex_match(splitClause[i], std::regex("t[0-9]+"))) {
                clauseOps[i] = TriggerOperator::TRIGGER;
                clauseValues[i] = lua_index_to_C_index(std::stoi(splitClause[i].substr(1, splitClause[i].length())));
                evalDepth += 1;
                maxEvalDepth = std::max(maxEvalDepth, evalDepth);
            }
            else if (std::regex_match(splitClause[i], std::regex("-?[0-9]+"))) {
                clauseOps[i] = TriggerOperator::CONSTANT;
                clauseValues[i] = std::stoi(splitClause[i]);
                evalDepth += 1;
                maxEvalDepth = std::max(maxEvalDepth, evalDepth);
            }
            else if (!splitClause[i].compare("f")) {
                clauseOps[i] = TriggerOperator::FRAME_NUM;
                evalDepth += 1;
                maxEvalDepth = std::max(maxEvalDepth, evalDepth);
            }
            else if (!splitClause[i].compare("!")) {
                clauseOps[i] = TriggerOperator::NOT;
                evalDepth += 0;
            }
            else {
                evalDepth += -1; // All the rest are binary operations
                if (!splitClause[i].compare("&")) {
                    clauseOps[i] = TriggerOperator::AND;
                }
                if (!splitClause[i].compare("|")) {
                    clauseOps[i] = TriggerOperator::OR;
                }
                if (!splitClause[i].compare("~")) {
                    clauseOps[i] = TriggerOperator::XOR;
                }
                if (!splitClause[i].compare("+")) {
                    clauseOps[i] = TriggerOperator::ADD;
                }
                if (!splitClause[i].compare("-")) {
                    clauseOps[i] = TriggerOperator::SUBTRACT;
                }
                if (!splitClause[i].compare("*")) {
                    clauseOps[i] = TriggerOperator::MULT;
                }
                if (!splitClause[i].compare("/")) {
                    clauseOps[i] = TriggerOperator::DIVIDE;
                }
                if (!splitClause[i].compare("%")) {
                    clauseOps[i] = TriggerOperator::MOD;
                }
                if (!splitClause[i].compare("=")) {
                    clauseOps[i] = TriggerOperator::EQUAL;
                }
                if (!splitClause[i].compare(">")) {
                    clauseOps[i] = TriggerOperator::GREATER;
                }
                if (!splitClause[i].compare("<")) {
                    clauseOps[i] = TriggerOperator::LESS;
                }
            }
        }

        return { clauseValues, clauseOps, maxEvalDepth };
    }

    TriggerClause(std::string &rawClause) :
        clauseValues(std::get<0>(processClauseString(rawClause))),
        clauseOps(std::get<1>(processClauseString(rawClause))),
        maxEvalDepth(std::get<2>(processClauseString(rawClause))) {}

    int doBinaryOperation(TriggerOperator op, int val1, int val2) const {
        switch (op) {
        case TriggerOperator::AND: {
            if (val2 > 0 && val1 > 0) {
                return val2;
            }
            return 0;
        }
        case TriggerOperator::OR: {
            if (val2 > 0 && val1 <= 0) {
                return val2;
            }
            if (val1 > 0) {
                return val1;
            }
            return 0;
        }
        case TriggerOperator::XOR: {
            if (val2 > 0 && val1 <= 0) {
                return val2;
            }
            if (val1 > 0 && val2 <= 0) {
                return val1;
            }
            return 0;
        }
        case TriggerOperator::ADD: {
            return val2 + val1;
        }
        case TriggerOperator::SUBTRACT: {
            return val2 - val1;
        }
        case TriggerOperator::MULT: {
            return val2 * val1;
        }
        case TriggerOperator::DIVIDE: {
            return val2 / val1;
        }
        case TriggerOperator::MOD: {
            return val2 % val1;
        }
        case TriggerOperator::EQUAL: {
            return (val2 == val1 ? 1 : 0);
        }
        case TriggerOperator::GREATER: {
            return (val2 > val1 ? 1 : 0);
        }
        case TriggerOperator::LESS: {
            return (val2 < val1 ? 1 : 0);
        }
        }
    }

    int GetOutput(mp::std::vector<mp::std::vector<int>> const &triggers, int frameNum) const {
        std::vector<int> evalStack;
        evalStack.reserve(maxEvalDepth);
        int val1;
        int val2;

        for (unsigned int i = 0; i < clauseOps.size(); ++i) {
            switch (clauseOps[i]) {
            case TriggerOperator::TRIGGER: {
                evalStack.push_back(triggers[clauseValues[i]][0]);
                break;
            }
            case TriggerOperator::CONSTANT: {
                evalStack.push_back(clauseValues[i]);
                break;
            }
            case TriggerOperator::FRAME_NUM: {
                evalStack.push_back(frameNum);
                break;
            }
            case TriggerOperator::NOT: {
                val1 = evalStack.back();
                evalStack.pop_back();
                evalStack.push_back(val1 > 0 ? 0 : 1);
                break;
            }
            default: {
                val1 = evalStack.back();
                evalStack.pop_back();
                val2 = evalStack.back();
                evalStack.pop_back();
                evalStack.push_back(doBinaryOperation(clauseOps[i], val1, val2));
                break;
            }
            }
        }
        val1 = evalStack.back();
        evalStack.pop_back();
        assert(evalStack.empty() && "Trigger clause not empty");
        return val1;
    }

    int GetOutput(mp::std::map<std::size_t, mt::std::vector<int>> const &outputTriggers, int frameNum) const {
        std::vector<int> evalStack;
        evalStack.reserve(maxEvalDepth);
        int val1;
        int val2;

        for (unsigned int i = 0; i < clauseOps.size(); ++i) {
            switch (clauseOps[i]) {
            case TriggerOperator::TRIGGER: {
                auto const outputTriggerIt{ outputTriggers.find(clauseValues[i]) };
                if (outputTriggerIt == outputTriggers.end()) {
                    evalStack.push_back(0);
                }
                if (outputTriggerIt->second.size() < 1) {
                    evalStack.push_back(0);
                }
                evalStack.push_back(outputTriggerIt->second[0]);
                break;
            }
            case TriggerOperator::CONSTANT: {
                evalStack.push_back(clauseValues[i]);
                break;
            }
            case TriggerOperator::FRAME_NUM: {
                evalStack.push_back(frameNum);
                break;
            }
            case TriggerOperator::NOT: {
                val1 = evalStack.back();
                evalStack.pop_back();
                evalStack.push_back(val1 > 0 ? 0 : 1);
                break;
            }
            default: {
                val1 = evalStack.back();
                evalStack.pop_back();
                val2 = evalStack.back();
                evalStack.pop_back();
                evalStack.push_back(doBinaryOperation(clauseOps[i], val1, val2));
                break;
            }
            }
        }
        val1 = evalStack.back();
        evalStack.pop_back();
        assert(evalStack.empty() && "Trigger clause not empty");
        return val1;
    }

    //bool operator==(TriggerClause const &o) const noexcept {
    //    return comparison_tuple() == o.comparison_tuple();
    //}

private:
    //auto comparison_tuple() const noexcept -> decltype(auto) {
    //    return std::tie(
    //        clauseValues,
    //        clauseOps
    //    );
    //}

    std::vector<int> const clauseValues;
    std::vector<TriggerOperator> const clauseOps;
    int const maxEvalDepth;
};
}//namespace hg
#endif //HG_TRIGGER_CLAUSE_H
