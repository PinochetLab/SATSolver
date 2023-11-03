#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "clauses.hpp"

class Elem {
public:
	std::string name;
	bool inverted;

	Elem(const std::string& name, bool inverted = false) : name(name), inverted(inverted) {}

	Elem inv() const {
		return Elem(name, !inverted);
	}


	friend bool operator <(const Elem& lhs, const Elem& rhs) {
		if (lhs.name == rhs.name) {
			return lhs.inverted < rhs.inverted;
		}
		return lhs.name < rhs.name;
	}

	std::string to_str() const {
		return (inverted ? "!" : "") + name;
	}

	friend std::ostream& operator << (std::ostream& os, const Elem& elem) {
		if (elem.inverted) os << '!';
		os << elem.name;
		return os;
	}
};

inline auto myHash = [](const clause_ptr& clause) {return clause->hash(); };

struct ClauseHash {
	std::size_t operator()(const clause_ptr& clause) const {
		return clause->hash();
	}
};

//std::shared_ptr<Clause> simplify(const std::shared_ptr<Clause>& clause) {
//	if (const auto& notClause = std::dynamic_pointer_cast<Not>(clause)) {
//		return simplifyNot(notClause);
//	}
//	if (const auto& andClause = std::dynamic_pointer_cast<And>(clause)) {
//		return simplifyAnd(andClause);
//	}
//	if (const auto& orClause = std::dynamic_pointer_cast<Or>(clause)) {
//		return simplifyOr(orClause);
//	}
//}
//
//std::shared_ptr<Clause> simplifyNot(const std::shared_ptr<Not>& notClause) {
//	std::shared_ptr<Clause> child = notClause->child;
//	if (const auto& notChild = std::dynamic_pointer_cast<Not>(child)) {
//		return simplify(child);
//	}
//	if (const auto& andChild = std::dynamic_pointer_cast<And>(child)) {
//		return simplify(std::make_shared<Or>(std::make_shared<Not>(andChild->left), std::make_shared<Not>(andChild->right)));
//	}
//	if (const auto& orChild = std::dynamic_pointer_cast<Or>(child)) {
//		return simplify(std::make_shared<And>(std::make_shared<Not>(orChild->left), std::make_shared<Not>(orChild->right)));
//	}
//}
//
//std::shared_ptr<Clause> simplifyAnd(const std::shared_ptr<And>& andClause) {
//	return std::make_shared<And>(simplify(andClause->left), simplify(andClause->right));
//}
//
//std::shared_ptr<Clause> simplifyOr(const std::shared_ptr<Or>& orClause) {
//	return std::make_shared<Or>(simplify(orClause->left), simplify(orClause->right));
//}

inline bool eq(const clause_ptr& c1, const clause_ptr& c2) {
	if (std::dynamic_pointer_cast<TClause>(c1) && std::dynamic_pointer_cast<TClause>(c2)) return true;
	if (std::dynamic_pointer_cast<FClause>(c1) && std::dynamic_pointer_cast<FClause>(c2)) return true;
	if (my_ptr<Lit> lit1 = std::dynamic_pointer_cast<Lit>(c1)) {
		if (my_ptr<Lit> lit2 = std::dynamic_pointer_cast<Lit>(c2)) {
			return lit1->lit == lit2->lit;
		}
		return false;
	}
	if (my_ptr<Not> not1 = std::dynamic_pointer_cast<Not>(c1)) {
		if (my_ptr<Not> not2 = std::dynamic_pointer_cast<Not>(c2)) {
			//return eq(not1->child, not2->child);
			return not1->child == not2->child;
		}
		return false;
	}
	if (my_ptr<And> and1 = std::dynamic_pointer_cast<And>(c1)) {
		if (my_ptr<And> and2 = std::dynamic_pointer_cast<And>(c2)) {
			//return eq(and1->left, and2->left) && eq(and1->right, and2->right);
			return and1->left == and2->left && and1->right == and2->right;
		}
		return false;
	}
	if (my_ptr<Or> or1 = std::dynamic_pointer_cast<Or>(c1)) {
		if (my_ptr<Or> or2 = std::dynamic_pointer_cast<Or>(c2)) {
			//return eq(or1->left, or2->left) && eq(or1->right, or2->right);
			return or1->left == or2->left && or1->right == or2->right;
		}
		return false;
	}
	return false;
}

inline auto myEq = [](const clause_ptr& c1, const clause_ptr& c2) -> bool { return eq(c1, c2); };

struct ElemHash {
	std::size_t operator()(const Elem& elem) const {
		std::hash<std::string> h;
		return h(elem.to_str());
	}
};

using clause_map = std::unordered_map<clause_ptr, clause_ptr, ClauseHash, decltype(myEq)>;

using d_t = std::set < Elem>;

using cnf_t = std::set<d_t>;

using cnf_map = std::unordered_map<clause_ptr, cnf_t, ClauseHash, decltype(myEq)>;

inline void printV(const std::vector<d_t>& cnf) {
	std::cout << "[ ";
	for (const auto& d : cnf) {
		std::cout << "{ ";
		for (const Elem& c : d) {
			std::cout << c << ' ';
		}
		std::cout << "} ";
	}
	std::cout << "]\n";
}

inline void print(const cnf_t& cnf) {
	std::cout << "[ ";
	for (const auto& d : cnf) {
		std::cout << "{ ";
		for (const Elem& c : d) {
			std::cout << c << ' ';
		}
		std::cout << "} ";
	}
	std::cout << "]\n";
}

class Cnf {
public:
	cnf_t cnf;
	Cnf(const cnf_t& cnf) : cnf(cnf) {}
};

using cnf_ptr = std::shared_ptr<Cnf>();

//class Cnf {
//private:
//	static size_t size;
//	static std::unordered_map<std::string, size_t> indices;
//	std::vector<size_t> values;
//public:
//	static void setInputs(const std::vector<std::string>& inputs) {
//		indices.clear();
//		size = inputs.size();
//		for (size_t i = 0; i < size; i++) {
//			indices.insert({ inputs[i], i });
//		}
//	}
//	Cnf(std::string lit, bool inverted) {
//		values = std::vector<size_t>(size, 2);
//		values[indices[lit]] = !inverted;
//	}
//	bool tryCombineWith(const Cnf& other) {
//		for (size_t i = 0; i < size; i++) {
//			if (values[i] == 2) values[i] = other.values[i];
//			else if (other.values[i] == 2) continue;
//			else if (values[i] != other.values[i]) {
//				values[i] = 0;
//			}
//		}
//		return true;
//	}
//};