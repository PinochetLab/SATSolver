#pragma once

#include <cassert>
#include <set>
#include <exception>

#include "clauses.hpp"
#include "cnf.hpp"

class Solver {
private:
	clause_map savedSimplified;
	std::unordered_map<clause_ptr, Elem> addedClauses;
	cnf_map savedCnf;
	size_t varIndex = 0;
	std::string generateName() {
		return std::to_string(varIndex++) + "#";
	}
public:
	Solver() {}

	bool isSolvable(clause_ptr clause) {
		std::shared_ptr<Clause> nClause = simplify(clause);
		std::cout << *nClause << '\n';
		if (std::shared_ptr<FClause> f = std::dynamic_pointer_cast<FClause>(nClause)) {
			return false;
		}
		return true;
	}

	// i don't use it now, because my clause converts to cnf faster than is simplified
	// it works as N with big constant
	// i tried different ways to compare And and Or childs so left == right and left != right.
	// but i think it is possible with helping of some invariant
	// i would like to tried improve simplify algorithm, but i already spend a lot of time on it
	// it can increase cnf size

	clause_ptr simplify(clause_ptr clause) {
		if (savedSimplified.contains(clause)) {
			return savedSimplified.at(clause);
		}
		clause_ptr result;
		if (const auto& tClause = std::dynamic_pointer_cast<TClause>(clause)) result = simplifyT(tClause);
		else if (const auto& fClause = std::dynamic_pointer_cast<FClause>(clause)) result = simplifyF(fClause);
		else if (const auto& litClause = std::dynamic_pointer_cast<Lit>(clause)) result = simplifyLit(litClause);
		else if (const auto& notClause = std::dynamic_pointer_cast<Not>(clause)) result = simplifyNot(notClause);
		else if (const auto& andClause = std::dynamic_pointer_cast<And>(clause)) result = simplifyAnd(andClause);
		else if (const auto& orClause = std::dynamic_pointer_cast<Or>(clause)) result = simplifyOr(orClause);
		else result = std::make_shared<FClause>();
		my_ptr<Not> inv = std::make_shared<Not>(clause);
		savedSimplified.insert({ clause, result });
		return result;
	}

	clause_ptr simplifyT(std::shared_ptr<TClause> tClause) { return std::make_shared<TClause>(); }

	clause_ptr simplifyF(std::shared_ptr<FClause> fClause) { return std::make_shared<FClause>(); }

	clause_ptr simplifyLit(std::shared_ptr<Lit> litClause) { return litClause; }

	clause_ptr simplifyNot(std::shared_ptr<Not> notClause) {
		clause_ptr child = notClause->child;
		if (const auto& notChild = std::dynamic_pointer_cast<Not>(child)) {
			return simplify(notChild->child);
		}
		if (const auto& andChild = std::dynamic_pointer_cast<And>(child)) {
			return simplify(std::make_shared<Or>(
				std::make_shared<Not>(andChild->left),
				std::make_shared<Not>(andChild->right)));
		}
		if (const auto& orChild = std::dynamic_pointer_cast<Or>(child)) {
			return simplify(std::make_shared<And>(
				std::make_shared<Not>(orChild->left),
				std::make_shared<Not>(orChild->right)));
		}
		if (std::dynamic_pointer_cast<TClause>(child)) {
			return std::make_shared<FClause>();
		}
		if (std::dynamic_pointer_cast<FClause>(child)) {
			return std::make_shared<TClause>();
		}
		return std::make_shared<Not>(simplify(child));
	}

	clause_ptr simplifyAnd(std::shared_ptr<And> andClause) {
		clause_ptr left = simplify(andClause->left);
		clause_ptr right = simplify(andClause->right);
		if (std::dynamic_pointer_cast<FClause>(left)) return std::make_shared<FClause>();
		if (std::dynamic_pointer_cast<FClause>(right)) return std::make_shared<FClause>();
		if (std::dynamic_pointer_cast<TClause>(left)) return right;
		if (std::dynamic_pointer_cast<TClause>(right)) return left;
		if (left == right) return left;
		return std::make_shared<And>(left, right);
	}

	clause_ptr simplifyOr(std::shared_ptr<Or> orClause) {
		clause_ptr left = simplify(orClause->left);
		clause_ptr right = simplify(orClause->right);
		if (std::dynamic_pointer_cast<TClause>(left)) return std::make_shared<TClause>();
		if (std::dynamic_pointer_cast<TClause>(right)) return std::make_shared<TClause>();
		if (std::dynamic_pointer_cast<FClause>(left)) return right;
		if (std::dynamic_pointer_cast<FClause>(right)) return left;
		if (left == right) return left;
		return std::make_shared<Or>(left, right);
	}

	// it's old cnf converting method. it works very slowly
	cnf_s to_cnf(clause_ptr clause) {
		if (savedCnf.contains(clause)) {
			return savedCnf.at(clause);
		}
		cnf_s result;
		if (const auto& andClause = std::dynamic_pointer_cast<And>(clause)) {
			result = to_cnf_and(andClause);
		}
		if (const auto& orClause = std::dynamic_pointer_cast<Or>(clause)) {
			result = to_cnf_or(orClause);
		}
		if (std::dynamic_pointer_cast<TClause>(clause)) {
			throw std::exception("True!");
		}
		if (std::dynamic_pointer_cast<FClause>(clause)) {
			throw std::exception("False!");
		}
		if (const auto& litClause = std::dynamic_pointer_cast<Lit>(clause)) {
			result = { {Elem(litClause->lit, false)} };
		}
		if (const auto& notClause = std::dynamic_pointer_cast<Not>(clause)) {
			if (my_ptr<Lit> litClause = std::dynamic_pointer_cast<Lit>(notClause->child)) {
				result = { {Elem(litClause->lit, true)} };
			} else {
				throw std::exception("Not isn't in bottom!");
			}
		}
		savedCnf.insert_or_assign(clause, result);
		return result;
	}

	cnf_s to_cnf_and(my_ptr<And> andClause) {
		cnf_s left = to_cnf(andClause->left);
		cnf_s right = to_cnf(andClause->right);
		left.insert(right.begin(), right.end());
		return left;
	}

	cnf_s to_cnf_or(my_ptr<Or> orClause) {
		cnf_s left = to_cnf(orClause->left);
		cnf_s right = to_cnf(orClause->right);
		cnf_s result;
		for (auto& c1 : left) {
			for (auto& c2 : right) {
				d_t d;
				d.insert(c1.begin(), c1.end());
				d.insert(c2.begin(), c2.end());
				result.insert(d);
			}
		}
		return result;
	}

	// this method uses Tseitin algorithm. it's fast.
	cnf_s to_cnf_fast(const clause_ptr& clause) {
		cnf_s cnf;
		Elem gate = to_cnf_fast(clause, cnf);

		// add disjunction with top element, because expression has to be true
		cnf.insert({ gate });
		return cnf;
	}

	Elem to_cnf_fast(const clause_ptr& clause, cnf_s& cnf) {
		Elem c = Elem(std::move(std::to_string(varIndex) + '#'));
		varIndex++;
		if (addedClauses.contains(clause)) {
			return {addedClauses.at(clause)};
		}
		if (const auto& andClause = std::dynamic_pointer_cast<And>(clause)) {
			Elem a = to_cnf_fast(andClause->left, cnf);
			Elem b = to_cnf_fast(andClause->right, cnf);
			cnf.insert({ a.inverted(), b.inverted(), c});
			cnf.insert({ a, c.inverted()});
			cnf.insert({ b, c.inverted()});
		}
		else if (const auto& orClause = std::dynamic_pointer_cast<Or>(clause)) {
			Elem a = to_cnf_fast(orClause->left, cnf);
			Elem b = to_cnf_fast(orClause->right, cnf);
			cnf.insert({ a, b, c.inverted()});
			cnf.insert({ a.inverted(), c});
			cnf.insert({ b.inverted(), c});
		}
		else if (const auto& notClause = std::dynamic_pointer_cast<Not>(clause)) {
			if (const auto& litClause = std::dynamic_pointer_cast<Lit>(notClause->child)) {
				c = Elem(litClause->lit, true);
			}
			else {
				Elem a = to_cnf_fast(notClause->child, cnf);
				cnf.insert({ a, c });
				cnf.insert({ a.inverted(), c.inverted() });
			}
		}
		else if (const auto& litClause = std::dynamic_pointer_cast<Lit>(clause)) {
			c = Elem(litClause->lit);
		}
		addedClauses.insert({clause, c});
		return c;
	}
};