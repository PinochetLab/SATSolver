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
	bool inv;

	Elem(const std::string& name, bool inverted = false) : name(name), inv(inverted) {}

	Elem inverted() const {
		return Elem(name, !inv);
	}

	friend bool operator <(const Elem& lhs, const Elem& rhs) {
		if (lhs.name == rhs.name) {
			return lhs.inv < rhs.inv;
		}
		return lhs.name < rhs.name;
	}
};

struct ClauseHash {
	std::size_t operator()(const clause_ptr& clause) const {
		return clause->hash();
	}
};

// this equality is slow, but it is used only if hashes are equal
inline bool equal(const clause_ptr& c1, const clause_ptr& c2) {
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
			return not1->child == not2->child;
		}
		return false;
	}
	if (my_ptr<And> and1 = std::dynamic_pointer_cast<And>(c1)) {
		if (my_ptr<And> and2 = std::dynamic_pointer_cast<And>(c2)) {
			return and1->left == and2->left && and1->right == and2->right;
		}
		return false;
	}
	if (my_ptr<Or> or1 = std::dynamic_pointer_cast<Or>(c1)) {
		if (my_ptr<Or> or2 = std::dynamic_pointer_cast<Or>(c2)) {
			return or1->left == or2->left && or1->right == or2->right;
		}
		return false;
	}
	return false;
}

inline auto myEq = [](const clause_ptr& c1, const clause_ptr& c2) -> bool { return equal(c1, c2); };

using clause_map = std::unordered_map<clause_ptr, clause_ptr, ClauseHash, decltype(myEq)>;

using d_t = std::set<Elem>;

using cnf_s = std::set<d_t>;
using cnf_v = std::vector<d_t>;

using cnf_map = std::unordered_map<clause_ptr, cnf_s, ClauseHash, decltype(myEq)>;