#include <iostream>
#include <filesystem>
#include <exception>

#include "clauses.hpp"
#include "bench.hpp"
#include "parser.hpp"
#include "dnf.hpp"

bool isSolvable(const std::string& path) throw(std::exception) {
	Bench* bench = Bench::fromFile(path);
	Parser* parser = new Parser(bench);
	std::vector<std::shared_ptr<Clause>> clauses = parser->parse();
	Dnf* dnf = new Dnf();
	return dnf->isSolvable(clauses);
}

void fileWork(const std::string path) {
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		std::string fileName(entry.path().string());
		try {
			isSolvable(fileName);
		}
		catch (std::exception& e) {
			std::cout << "bad alloc" << std::endl;
		}
	}
}

int main() {
	//fileWork("benchs_for_test_task");
	std::cout << isSolvable("simple.bench");
	//std::cout << isSolvable("benchs_for_test_task/trVlog_2.bench");
	return 0;
}