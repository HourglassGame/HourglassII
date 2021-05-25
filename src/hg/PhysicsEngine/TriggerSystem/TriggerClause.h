#ifndef HG_TRIGGER_CLAUSE_H
#define HG_TRIGGER_CLAUSE_H

#include "hg/LuaUtil/LuaUtilities.h"
#include <iostream>
#include <string.h> 
#include <regex>
#include <tuple>
#include "hg/mp/std/map"

namespace hg {
enum class TriggerOperator : unsigned char {
	TRIGGER = 0,
	TRIGGER_OUT = 1,
	CONSTANT = 2,
	FRAME_NUM = 3,
	NOT = 4,
	AND = 5,
	OR = 6,
	XOR = 7,
	ADD = 8,
	SUBTRACT = 9,
	MULT = 10,
	DIVIDE = 11,
	MOD = 12,
	EQUAL = 13,
	GREATER = 14,
	LESS = 15
};

class TriggerClause {
private:
	auto comparison_tuple() const noexcept {
		return std::tie(
			clauseValues,
			clauseOps
			//maxEvalDepth: Intentionally not including this, as it is just a cached value that has a value always implied by clauseValues and clauseOps.
		);
	}

	static std::tuple<std::vector<int>, std::vector<TriggerOperator>, int> processClauseString(std::string &rawClause)
	{
		std::istringstream iss(rawClause);
		std::vector<std::string> splitClause(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

		std::vector<int> clauseValues;
		std::vector<TriggerOperator> clauseOps(splitClause.size());

		unsigned int evalDepth = 0;
		unsigned int maxEvalDepth = 0;
		for (unsigned int i = 0; i < splitClause.size(); ++i) {
			//std::cerr << rawClause << " splitClause[i] " << i << ", " << splitClause[i] << "\n";
			if (std::regex_match(splitClause[i], std::regex("[to][0-9]+"))) {
				clauseOps[i] = (splitClause[i].at(0) == 't' ? TriggerOperator::TRIGGER : TriggerOperator::TRIGGER_OUT);
				clauseValues.push_back(lua_index_to_C_index(std::stoi(splitClause[i].substr(1, splitClause[i].length()))));
				clauseValues.push_back(0);
				evalDepth += 1;
				maxEvalDepth = std::max(maxEvalDepth, evalDepth);
			}
			else if (std::regex_match(splitClause[i], std::regex("[to][0-9]+,[0-9]+"))) {
				clauseOps[i] = (splitClause[i].at(0) == 't' ? TriggerOperator::TRIGGER : TriggerOperator::TRIGGER_OUT);
				size_t found = splitClause[i].find(",");
				//std::cerr << "first " << splitClause[i].substr(1, found - 1) << ", second " << splitClause[i].substr(found + 1, splitClause[i].length()) << "\n";
				clauseValues.push_back(lua_index_to_C_index(std::stoi(splitClause[i].substr(1, found - 1))));
				clauseValues.push_back(lua_index_to_C_index(std::stoi(splitClause[i].substr(found + 1, splitClause[i].length()))));
				evalDepth += 1;
				maxEvalDepth = std::max(maxEvalDepth, evalDepth);
			}
			else if (std::regex_match(splitClause[i], std::regex("-?[0-9]+"))) {
				clauseOps[i] = TriggerOperator::CONSTANT;
				clauseValues.push_back(std::stoi(splitClause[i]));
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
	explicit TriggerClause(std::tuple<std::vector<int>, std::vector<TriggerOperator>, int> &&processedClause) :
		clauseValues(std::move(std::get<0>(processedClause))),
		clauseOps(std::move(std::get<1>(processedClause))),
		maxEvalDepth(std::move(std::get<2>(processedClause)))
	{
	}
public:
	explicit TriggerClause(std::string &rawClause) :
		TriggerClause(processClauseString(rawClause))
	{
	}

	static int doBinaryOperation(TriggerOperator op, int val1, int val2) {
		switch (op) {
		case TriggerOperator::AND: {
			if (val2 != 0 && val1 != 0) {
				return val1;
			}
			return 0;
		}
		case TriggerOperator::OR: {
			if (val2 != 0) {
				return val2;
			}
			if (val1 != 0) {
				return val1;
			}
			return 0;
		}
		case TriggerOperator::XOR: {
			if (val2 != 0 && val1 == 0) {
				return val2;
			}
			if (val1 != 0 && val2 == 0) {
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
		return 0;
	}

	int execute(mp::std::vector<mp::std::vector<int>> const &triggers, int frameNum) const {
		std::vector<int> evalStack;
		evalStack.reserve(maxEvalDepth);
		int val1;
		int val2;
		int cIndex = 0;

		for (unsigned int i = 0; i < clauseOps.size(); ++i) {
			switch (clauseOps[i]) {
			case TriggerOperator::TRIGGER: {
				int firstIndex = clauseValues[cIndex];
				int secondIndex = clauseValues[cIndex + 1];
				cIndex += 2;

				evalStack.push_back(triggers[firstIndex][secondIndex]);
				break;
			}
			case TriggerOperator::CONSTANT: {
				evalStack.push_back(clauseValues[cIndex]);
				cIndex += 1;
				break;
			}
			case TriggerOperator::FRAME_NUM: {
				evalStack.push_back(frameNum);
				break;
			}
			case TriggerOperator::NOT: {
				val1 = evalStack.back();
				evalStack.pop_back();
				evalStack.push_back(val1 != 0 ? 0 : 1);
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

	int executeOnArrivalAndOut(mp::std::vector<mp::std::vector<int>> const &triggers, mp::std::map<std::size_t, mt::std::vector<int>> const &outputTriggers, int frameNum) const {
		std::vector<int> evalStack;
		evalStack.reserve(maxEvalDepth);
		int val1;
		int val2;
		int cIndex = 0;

		//std::cerr << "Top:";

		for (unsigned int i = 0; i < clauseOps.size(); ++i) {
			//if (!evalStack.empty()) {
			//    std::cerr << " (" << i << ", " << evalStack.back() << "),";
			//}

			switch (clauseOps[i]) {
			case TriggerOperator::TRIGGER_OUT: {
				int firstIndex = clauseValues[cIndex];
				int secondIndex = clauseValues[cIndex + 1];
				cIndex += 2;

				auto const outputTriggerIt{ outputTriggers.find(firstIndex) };
				if (outputTriggerIt == outputTriggers.end()) {
					evalStack.push_back(0);
					break;
				}
				if (outputTriggerIt->second.size() < secondIndex + 1) {
					evalStack.push_back(0);
					break;
				}
				evalStack.push_back(outputTriggerIt->second[secondIndex]);
				break;
			}
			case TriggerOperator::TRIGGER: {
				int firstIndex = clauseValues[cIndex];
				int secondIndex = clauseValues[cIndex + 1];
				cIndex += 2;
				evalStack.push_back(triggers[firstIndex][secondIndex]);
				break;
			}
			case TriggerOperator::CONSTANT: {
				evalStack.push_back(clauseValues[cIndex]);
				cIndex += 1;
				break;
			}
			case TriggerOperator::FRAME_NUM: {
				evalStack.push_back(frameNum);
				break;
			}
			case TriggerOperator::NOT: {
				val1 = evalStack.back();
				evalStack.pop_back();
				evalStack.push_back(val1 != 0 ? 0 : 1);
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

		//if (!evalStack.empty()) {
		//    std::cerr << " (end, " << evalStack.back() << "),";
		//}
		//std::cerr << "\n";

		val1 = evalStack.back();
		evalStack.pop_back();
		assert(evalStack.empty() && "Trigger clause not empty");
		return val1;
	}

	bool operator==(TriggerClause const &o) const noexcept {
		return comparison_tuple() == o.comparison_tuple();
	}

private:

	std::vector<int> const clauseValues;
	std::vector<TriggerOperator> const clauseOps;
	int const maxEvalDepth;
};
}//namespace hg
#endif //HG_TRIGGER_CLAUSE_H
