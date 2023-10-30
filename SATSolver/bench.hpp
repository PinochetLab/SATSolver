#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
#include <ranges>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

struct Bench {
private:
	std::unordered_set<std::string> inputs;
	std::vector<std::string> outputs;
	std::unordered_map<std::string, std::string> formulaStrings;

	Bench(const std::string& benchCode) {
		std::stringstream ss(benchCode);
		std::string line;
		while (std::getline(ss, line, '\n')) {
			line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
			if (line == "") continue;
			if (line.starts_with("#")) {
				continue;
			}
			if (line.starts_with("INPUT")) {
				std::string name = line.substr(6, line.length() - 7);
				inputs.insert(name);
			}
			else if (line.starts_with("OUTPUT")) {
				std::string name = line.substr(7, line.length() - 8);
				outputs.push_back(name);
			}
			else if (line.find('=') != std::string::npos) {
				size_t i = line.find('=');
				std::string name = line.substr(0, i);
				std::string formula = line.substr(i + 1);
				formulaStrings.insert_or_assign(name, formula);
			}
		}
	}
public:
	static Bench* fromCode(const std::string& benchCode) {
		return new Bench(benchCode);
	}

	static Bench* fromFile(const std::string& filePath) {
		std::ifstream in = std::ifstream(filePath.data());
		std::ostringstream sstr;
		sstr << in.rdbuf();
		return new Bench(sstr.str());
	}

	std::unordered_set<std::string>& getInputs() {
		return inputs;
	}

	std::vector<std::string>& getOutputs() {
		return outputs;
	}

	std::unordered_map<std::string, std::string>& getFormulas() {
		return formulaStrings;
	}
};