#include <iostream>
#include <filesystem>
#include <exception>
#include <chrono>
#include <functional>
#include <z3++.h>

#include "clauses.hpp"
#include "bench.hpp"
#include "parser.hpp"
#include "solver.hpp"
#include "2Sat.hpp"


//void fileWork(const std::string path) {
//	for (const auto& entry : std::filesystem::directory_iterator(path)) {
//		std::string fileName(entry.path().string());
//		try {
//			isSolvable(fileName);
//		}
//		catch (std::exception& e) {
//			std::cout << "bad alloc" << std::endl;
//		}
//	}
//}


std::unordered_map<std::string, std::function<bool()>> values;

bool getValue(const std::string& name) {
	if (auto it = values.find(name); it != values.end()) {
		return it->second();
	}
	return false;
}



bool is_true(const d_t& d) {
	for (const auto& e : d) {
		if (d.contains(e.inv())) {
			return true;
		}
	}
	return false;
}

void set_var(std::vector<d_t>& cnf, const Elem& elem) {
	values[elem.name] = [elem]() {return !elem.inverted; };
	size_t s = cnf.size();
	for (size_t i = 0; i < s; i++) {
		size_t r_i = s - i - 1;
		if (cnf[r_i].contains(elem)) {
			cnf.erase(cnf.begin() + r_i);
		}
		else if (cnf[r_i].contains(elem.inv())) {
			cnf[r_i].erase(elem.inv());
		}
	}
}

void set_all_variables(std::vector<d_t>& cnf) {
	while (true) {
		bool var_found = false;
		for (size_t i = 0; i < cnf.size(); i++) {
			if (cnf[i].size() == 1) {
				Elem e = *cnf[i].begin();
				set_var(cnf, e);
				var_found = true;
			}
		}
		if (!var_found) break;
	}
}

void remove_true_ds(std::vector<d_t>& cnf) {
	size_t s = cnf.size();
	
	for (size_t i = 0; i < s; i++) {
		//std::cout << "i = " << i << std::endl;
		size_t r_i = s - i - 1;
		if (is_true(cnf[r_i])) {
			cnf.erase(std::next(cnf.begin(), r_i));
		}
	}
}

bool satisfiableCnf(const cnf_t& cnf, const std::unordered_map<std::string, bool>& m) {
	/*for (const auto& [k, v] : m) {
		std::cout << (v ? "" : "!") << k << "  ";
	}*/
	//std::cout << std::endl;
	for (const auto& d : cnf) {
		bool isTrue = false;
		for (const auto& e : d) {
			if (m.contains(e.name)) {
				bool b = m.at(e.name);
				if (b == !e.inverted) {
					isTrue = true;
					break;
				}
			}
			else {
				std::unordered_map<std::string, bool> m0(m);
				std::unordered_map<std::string, bool> m1(m);
				m0.insert({ e.name, false });
				m1.insert({ e.name, true });
				return satisfiableCnf(cnf, m1) || satisfiableCnf(cnf, m0);
			}
		}
		if (!isTrue) return false;
	}
	return true;
}

bool satisfiable(std::vector<d_t>& v) {

	remove_true_ds(v);
	set_all_variables(v);
	cnf_t cnf(v.begin(), v.end());
	return satisfiableCnf(cnf, {});
}

bool resolve(const d_t& d1, const d_t& d2, d_t& d, Elem& elem) {
	for (const auto& e : d1) {
		if (d2.contains(e.inv())) {
			//std::cout << "yes!" << std::endl;
			//std::cout << d1.size() << " - " << d2.size() << std::endl;
			d = d_t(d1);
			d.insert(d2.begin(), d2.end());
			d.erase(e);
			d.erase(e.inv());
			elem = e;
			//std::cout << "end!" << std::endl;
			return true;
		}
	}
	return false;
}

//void try_solve(std::vector<d_t>& cnf) {
//	remove_true_ds(cnf);
//	set_all_variables(cnf);
//	z3::context context;
//	z3::expr_vector v(context);
//	z3::solver solver(context);
//	for (const d_t& d : cnf) {
//		if (d.size() == 2) {
//			cnf2.push_back(d);
//		}
//	}
//	std::cout << cnf.size() << "/" << cnf2.size();
//
//	z3::context context;
//	z3::expr_vector v(context);
//	z3::solver solver(context);
//}

bool is_solvable(std::vector<d_t>& cnf) {
	while (true) {
		//std::cout << cnf.size() << std::endl;
		remove_true_ds(cnf);
		
		set_all_variables(cnf);
		

		for (const auto& d : cnf) {
			if (d.empty()) {
				return false;
			}
		}

		size_t i = 0;
		for (const auto& d : cnf) {
			if (d.size() > 2) {
				i++;
			}
		}

		//std::cout << i << "/" << cnf.size() << std::endl;

		bool resolution_applied = false;
		for (size_t i = 0; i < cnf.size(); i++) {
			//std::cout << "i = " << i << std::endl;
			for (size_t j = i + 1; j < cnf.size(); j++) {
				//std::cout << "j = " << j << std::endl;
				d_t new_d;
				Elem e("");
				if (resolve(cnf[i], cnf[j], new_d, e)) {
					std::cout << "e: " << e.name << std::endl;
					bool unique = true;
					for (size_t k = 0; k < cnf.size(); k++) {
						if (k == i || k == j) continue;
						if (cnf[k].contains(e) || cnf[k].contains(e.inv())) {
							std::cout << "i: " << i << "; j: " << j << "; k: " << k << std::endl;
							unique = false;
							break;
						}
					}
					if (!unique) continue;
					std::cout << "i: " << i << " j: " << j << std::endl;
					resolution_applied = true;
					cnf.erase(cnf.begin() + j);
					cnf.erase(cnf.begin() + i);
					cnf.push_back(new_d);
					break;
				}
			}
			if (resolution_applied) {
				break;
			}
		}
		if (!resolution_applied) {
			break;
		}
	}
	//printV(cnf);

	return true;
}

bool isSolvable(const cnf_t& cnf) {
	std::vector<d_t> v(cnf.begin(), cnf.end());
	return is_solvable(v);
}

bool isSolvable(const std::string& path) {
	clause_ptr clause = Parser().parse(path);
	//std::cout << clause->count << std::endl;
	Solver solver = Solver();
	clause = solver.simplify(clause);
	if (std::dynamic_pointer_cast<TClause>(clause)) return true;
	if (std::dynamic_pointer_cast<FClause>(clause)) return false;
	cnf_t cnf = solver.to_cnf(clause);
	//return isSolvable(cnf);
	return true;
}

double countTime(clock_t& last) {
	clock_t end = clock();
	double seconds = (double)(end - last) / CLOCKS_PER_SEC;
	last = end;
	return seconds;
}

z3::solver solveZ3(const std::vector<d_t>& vec, z3::context& context) {
	z3::expr_vector v(context);
	z3::solver solver(context);
	for (const auto& d : vec) {
		z3::expr_vector diz(context);
		for (const auto& e : d) {
			z3::expr expr = context.bool_const(e.name.c_str());
			if (e.inverted) diz.push_back(!expr);
			else diz.push_back(expr);
		}
		v.push_back(z3::mk_or(diz));
	}
	solver.add(z3::mk_and(v));
	return solver;
}

bool is_sat(std::vector<d_t>& cnf, z3::context& context) {
	//remove_true_ds(cnf);
	//set_all_variables(cnf);
	//printV(cnf);
	std::vector<d_t> sat2;
	for (const auto& d : cnf) {
		if (d.size() == 2) sat2.push_back(d);
	}
	if (sat2.size() > 10) {
		bool solvable2 = Sat2(sat2).check();
		if (!solvable2) {
			std::cout << "2SAT Part is not satisfiable." << std::endl;
			return false;
		}
	}
	return solveZ3(cnf, context).check();
}

void doOne(const std::string& path) {
	clock_t start = clock();
	
	clause_ptr clause = Parser().parse(path);
	
	double parsingTime = countTime(start);
	std::cout << "parsing finished in " << parsingTime << " seconds" << std::endl;

	Solver solver = Solver();
	//clause = solver.simplify(clause);

	//double simplifyTime = countTime(start);

	//std::cout << "simplifying finished in " << simplifyTime << " seconds" << std::endl;

	double cnfTime = 0;
	double solveTime = 0;

	bool solvable;
	if (std::dynamic_pointer_cast<TClause>(clause)) {
		solvable = true;
	}
	else if (std::dynamic_pointer_cast<FClause>(clause)) {
		std::cout << "cnfization finished in " << cnfTime << " seconds" << std::endl;
		std::cout << "solving finished in " << solveTime << " seconds" << std::endl;
		solvable = false;
	}
	else {
		cnf_t cnf = solver.to_cnf_fast(clause);
		//cnf_t cnf = solver.to_cnf(clause);
		cnfTime = countTime(start);

		std::cout << "cnfization finished in " << cnfTime << " seconds" << std::endl;

		std::vector<d_t> vec(cnf.begin(), cnf.end());
		set_all_variables(vec);
		remove_true_ds(vec);
		
		z3::context context;
		//solvable = solveZ3(vec, context).check();
		solvable = is_sat(vec, context);
		//std::cout << solver.get_model() << std::endl;
		//solvable = is_solvable(vec);

		//std::cout << "satisfiable: " << s << "; solvable: " << solvable << std::endl;

		solveTime = countTime(start);

		std::cout << "solving finished in " << solveTime << " seconds" << std::endl;
	}
	//return isSolvable(cnf);

	std::ifstream in = std::ifstream("results.txt");
	std::ostringstream sstr;
	sstr << in.rdbuf();
	std::ofstream os = std::ofstream("results.txt");
	os << sstr.str() << path << ":  " << (solvable ? "SAT" : "UNSAT") << 
		"\ntime(sec): {parse: " << parsingTime << ", to_cnf: " << 
		cnfTime << ", solve: " << solveTime << ", total: " << (parsingTime + cnfTime + solveTime) << "}\n\n";
}

int main() {
	doOne("benchs_for_test_task/trVn_10.bench");
	//std::cout << (21 ^ 1) << std::endl;
	//doOne("simple.bench");
	return 0;
}