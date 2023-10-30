#pragma once

#include "bench.hpp"
#include "clauses.hpp"

class Parser {
private:
	Bench* bench;
	std::unordered_map<std::string, std::shared_ptr<Clause>> clauses;

	std::shared_ptr<Clause> parse(const std::string& name) {
		if (clauses.contains(name)) return clauses.at(name);
		std::string formula = bench->getFormulas().at(name);
 		size_t start = formula.find('(');
		size_t end = formula.find(')');
		std::string operation = formula.substr(0, start);
		std::string args = formula.substr(start + 1, end - start - 1);
		std::stringstream ss(args);
		std::string sym;
		my_set operands;
		while (std::getline(ss, sym, ',')) {
			std::shared_ptr<Clause> operand = clauses.contains(sym) ? clauses.at(sym) : parse(sym);
			operands.insert(operand);
		}
		std::shared_ptr<Clause> result;
		if (operation == "NOT") result = std::make_shared<Not>(*operands.begin());
		else if (operation == "AND") result = std::make_shared<And>(operands);
		else if (operation == "OR") result = std::make_shared<Or>(operands);
		else if (operation == "XOR") {
			auto it = operands.begin();
			std::shared_ptr<Clause> fst = *it;
			std::shared_ptr<Clause> snd = *(it++);
			std::shared_ptr<Clause> notFst = std::make_shared<Not>(fst);
			std::shared_ptr<Clause> notSnd = std::make_shared<Not>(snd);
			std::shared_ptr<Clause> and1 = std::make_shared<And>(my_set{ fst, notSnd });
			std::shared_ptr<Clause> and2 = std::make_shared<And>(my_set{ notFst, snd });
			result = std::make_shared<Or>(my_set{ and1, and2 });
		}
		else if (operation == "NAND") result = std::make_shared<Not>(std::make_shared<And>(operands));
		else if (operation == "NOR") result = std::make_shared<Not>(std::make_shared<Or>(operands));
		else if (operation == "NXOR") {
			auto it = operands.begin();
			std::shared_ptr<Clause> fst = *it;
			std::shared_ptr<Clause> snd = *(it++);
			std::shared_ptr<Clause> notFst = std::make_shared<Not>(fst);
			std::shared_ptr<Clause> notSnd = std::make_shared<Not>(snd);
			std::shared_ptr<Clause> and1 = std::make_shared<And>(my_set{ fst, snd });
			std::shared_ptr<Clause> and2 = std::make_shared<And>(my_set{ notFst, notSnd });
			result = std::make_shared<Or>(my_set{ and1, and2 });
		}
		this->clauses.insert_or_assign(name, result);
		return result;
	}
public:
	Parser(Bench* bench) {
		this->bench = bench;
	}

	std::vector<std::shared_ptr<Clause>> parse() {
		for (const std::string& name : bench->getInputs()) {
			clauses.insert_or_assign(name, std::make_shared<Lit>(name));
		}
		std::vector<std::shared_ptr<Clause>> clauses;
		for (const std::string& name : bench->getOutputs()) {
			clauses.push_back(parse(name));
		}
		return clauses;
	}
};