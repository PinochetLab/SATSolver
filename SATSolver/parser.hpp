#pragma once

#include "bench.hpp"
#include "clauses.hpp"

class Parser {
private:
	std::string output;
	std::unordered_map<std::string, std::shared_ptr<Clause>> clauses;
public:
	Parser() { }

	clause_ptr parse(const std::string& filePath) {
		std::ifstream in = std::ifstream(filePath.data());
		std::string line;
		while (std::getline(in, line, '\n')) {
			line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
			if (line == "") continue;
			if (line.starts_with("#")) {
				continue;
			}
			if (line.starts_with("INPUT")) {
				std::string name = line.substr(6, line.length() - 7);
				clauses.insert({ name, std::make_shared<Lit>(name) });
			}
			else if (line.starts_with("OUTPUT")) {
				std::string name = line.substr(7, line.length() - 8);
				output = name;
			}
			else if (line.find('=') != std::string::npos) {
				size_t i = line.find('=');
				std::string name = line.substr(0, i);
				std::string formula = line.substr(i + 1);
				
				size_t start = formula.find('(');
				size_t end = formula.find(')');
				std::string operation = formula.substr(0, start);
				std::string args = formula.substr(start + 1, end - start - 1);
				std::stringstream ss(args);
				std::string sym;
				std::vector<clause_ptr> operands;
				while (std::getline(ss, sym, ',')) {
					std::shared_ptr<Clause> operand = clauses[sym];
					operands.push_back(operand);
				}
				std::shared_ptr<Clause> result;
				if (operation == "NOT") result = std::make_shared<Not>(operands[0]);
				else if (operation == "AND") result = std::make_shared<And>(operands[0], operands[1]);
				else if (operation == "OR") result = std::make_shared<Or>(operands[0], operands[1]);
				else if (operation == "XOR") {
					std::shared_ptr<Clause> notFst = std::make_shared<Not>(operands[0]);
					std::shared_ptr<Clause> notSnd = std::make_shared<Not>(operands[1]);
					std::shared_ptr<Clause> or1 = std::make_shared<Or>(operands[0], operands[1]);
					std::shared_ptr<Clause> or2 = std::make_shared<Or>(notFst, notSnd);
					result = std::make_shared<And>(or1, or2);
				}
				else if (operation == "NAND") result = std::make_shared<Not>(std::make_shared<And>(operands[0], operands[1]));
				else if (operation == "NOR") result = std::make_shared<Not>(std::make_shared<Or>(operands[0], operands[1]));
				else if (operation == "NXOR") {
					std::shared_ptr<Clause> notFst = std::make_shared<Not>(operands[0]);
					std::shared_ptr<Clause> notSnd = std::make_shared<Not>(operands[1]);
					std::shared_ptr<Clause> and1 = std::make_shared<And>(operands[0], operands[1]);
					std::shared_ptr<Clause> and2 = std::make_shared<And>(notFst, notSnd);
					result = std::make_shared<Or>(and1, and2);
				}
				else if (operation == "BUFF") {
					//std::cout << "size: " << operands.size() << std::endl;
					//std::cout << operands[0] << std::endl;
					result = operands[0];
				}
				this->clauses.insert({ name, result });
				//std::cout << "name: " << name << std::endl;
			}
		}
		return clauses.at(output);
	}
};