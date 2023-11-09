#include <iostream>
#include <filesystem>
#include <fstream>
#include <exception>
#include <chrono>
#include <functional>
#include <z3++.h>

#include "clauses.hpp"
#include "parser.hpp"
#include "solver.hpp"
#include "sat2.hpp"

bool isControversy(const d_t& d) {
	for (const auto& e : d) {

		// if contains inverted element
		if (d.contains(e.inverted())) {
			return true;
		}
	}
	return false;
}

void substitute(std::vector<d_t>& cnf, const Elem& elem) {
	size_t s = cnf.size();
	for (size_t i = 0; i < s; i++) {
		size_t r_i = s - i - 1;

		// if contains element, this disjunction is true, we can remove it
		if (cnf[r_i].contains(elem)) {
			cnf.erase(cnf.begin() + r_i);
		}

		// if contains inverted element, we can remove that element
		else if (cnf[r_i].contains(elem.inverted())) {
			cnf[r_i].erase(elem.inverted());
		}
	}
}

void substituteObviusVariables(std::vector<d_t>& cnf) {
	while (true) {
		bool var_found = false;
		for (size_t i = 0; i < cnf.size(); i++) {

			// if dizjunction contains only one item, we can substitute it
			if (cnf[i].size() == 1) {
				Elem e = *cnf[i].begin();
				substitute(cnf, e);
				var_found = true;
			}
		}
		if (!var_found) break;
	}
}

void removeControversies(cnf_v& cnf) {
	size_t s = cnf.size();
	for (size_t i = 0; i < s; i++) {
		size_t r_i = s - i - 1;
		if (isControversy(cnf[r_i])) {
			cnf.erase(std::next(cnf.begin(), r_i));
		}
	}
}

bool containsEmpty(const cnf_v& cnf) {
	for (const auto& d : cnf) {
		if (d.empty()) return true;
	}
	return false;
}

void simplify(cnf_v& cnf) {
	substituteObviusVariables(cnf);
	removeControversies(cnf);
}

double countTime(clock_t& last) {
	clock_t end = clock();
	double seconds = (double)(end - last) / CLOCKS_PER_SEC;
	last = end;
	return seconds;
}

z3::solver solveZ3(const std::vector<d_t>& vec, z3::context& context) {
	z3::expr_vector cnfZ3(context);
	z3::solver solver(context);
	for (const auto& d : vec) {
		z3::expr_vector disjunctionZ3(context);
		for (const auto& e : d) {
			z3::expr expr = context.bool_const(e.name.c_str());
			if (e.inv) disjunctionZ3.push_back(!expr);
			else disjunctionZ3.push_back(expr);
		}
		cnfZ3.push_back(z3::mk_or(disjunctionZ3));
	}
	solver.add(z3::mk_and(cnfZ3));
	return solver;
}

bool is_sat(cnf_v& cnf, z3::context& context) {
	// if contains empty disjunction
	if (containsEmpty(cnf)) return false;
	
	// collecting 2-disjunctions
	cnf_v cnf2; 
	for (const auto& d : cnf) {
		if (d.size() == 2) cnf2.push_back(d);
	}

	// solve with Z3
	if (cnf2.empty()) {
		return solveZ3(cnf, context).check();
	}
	Sat2 sat2 = Sat2(cnf2);
	bool sat = sat2.check();

	// our 3sat is 2sat
	if (cnf2.size() == cnf.size()) {
		return sat;
	}
	if (!sat) {
		return false;
	}
	sat2_answer_t answer = sat2.getAnswer();
	bool simplified = false;
	for (auto [name, value] : answer) {
		cnf_v altCnf(cnf2);

		// substitute invert value for check if this value is neccassary
		substitute(altCnf, Elem(name, value));
		simplify(altCnf);

		// if new 2sat is unsat 
		if (containsEmpty(altCnf) || !Sat2(altCnf).check()) {
			simplified = true;

			//substitute neccessary value to cnf
			substitute(cnf, Elem(name, !value));
			simplify(cnf);
		}
	}

	// if no neccessary vars
	if (!simplified) {
		return solveZ3(cnf, context).check();
	}
	return is_sat(cnf, context);
}

void solveBench(const std::string& path) {
	clock_t start = clock();

	clause_ptr clause = Parser().parse(path);

	double parsingTime = countTime(start);

	Solver solver = Solver();

	cnf_s cnf = solver.to_cnf_fast(clause);
	
	double toCnfTime = countTime(start);

	std::vector<d_t> cnfAsVec(cnf.begin(), cnf.end());

	substituteObviusVariables(cnfAsVec);

	removeControversies(cnfAsVec);
		
	z3::context context;

	bool solvable = is_sat(cnfAsVec, context);
	
	double solveTime = countTime(start);

	std::ifstream resultFile = std::ifstream("results.txt");
	std::ostringstream old;
	old << resultFile.rdbuf();
	std::ofstream os = std::ofstream("results.txt");
	os << old.str() << path << ":  " << (solvable ? "SAT" : "UNSAT") <<
		"\ntime(sec): {parse: " << parsingTime << ", to_cnf: " << 
		toCnfTime << ", solve: " << solveTime << ", total: " << (parsingTime + toCnfTime + solveTime) << "}\n\n";
}

int main() {
	solveBench("benchs_for_test_task/12_13_1.bench");
	return 0;
}