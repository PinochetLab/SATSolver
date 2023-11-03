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
	size_t simplifiedCount = 0;
	size_t loadedCount = 0;

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

	clause_ptr simplify(clause_ptr clause) {
		//std::cout << ++loadedCount << std::endl;
		if (savedSimplified.contains(clause)) {
			//std::cout << "loaded: " << simplifiedCount << "/" << ++loadedCount << std::endl;
			return savedSimplified.at(clause);
		}
		//std::cout << "saved: " << ++simplifiedCount << "/" << loadedCount << std::endl;
		//std::cout << clause->prettyString() << std::endl;
		clause_ptr result;
		if (my_ptr<TClause> tClause = std::dynamic_pointer_cast<TClause>(clause)) result = simplifyT(tClause);
		else if (my_ptr<FClause> fClause = std::dynamic_pointer_cast<FClause>(clause)) result = simplifyF(fClause);
		else if (my_ptr<Lit> litClause = std::dynamic_pointer_cast<Lit>(clause)) result = simplifyLit(litClause);
		else if (my_ptr<Not> notClause = std::dynamic_pointer_cast<Not>(clause)) result = simplifyNot(notClause);
		else if (my_ptr<And> andClause = std::dynamic_pointer_cast<And>(clause)) result = simplifyAnd(andClause);
		else if (my_ptr<Or> orClause = std::dynamic_pointer_cast<Or>(clause)) result = simplifyOr(orClause);
		else result = std::make_shared<FClause>();
		my_ptr<Not> inv = std::make_shared<Not>(clause);
		/*if (!savedSimplified.contains(inv)) {
			savedSimplified.insert({inv, simplifyNot(inv)});
		}*/
		//simplify(std::make_shared<Not>(result));
		//std::cout << *clause << "(" << clause << ")" << " ==> " << *result << "(" << result << ")" << std::endl;
		savedSimplified.insert({ clause, result });
		//std::cout << simplifiedCount++ << "/" << loadedCount << std::endl;
		return result;
	}

	clause_ptr simplifyT(std::shared_ptr<TClause> tClause) { return std::make_shared<TClause>(); }

	clause_ptr simplifyF(std::shared_ptr<FClause> fClause) { return std::make_shared<FClause>(); }

	clause_ptr simplifyLit(std::shared_ptr<Lit> litClause) { return litClause; }

	clause_ptr simplifyNot(std::shared_ptr<Not> notClause) {
		clause_ptr child = notClause->child;
		if (std::shared_ptr<Not> notChild = std::dynamic_pointer_cast<Not>(child)) {
			return simplify(notChild->child);
		}
		if (std::shared_ptr<And> andChild = std::dynamic_pointer_cast<And>(child)) {
			return simplify(std::make_shared<Or>(
				std::make_shared<Not>(andChild->left),
				std::make_shared<Not>(andChild->right)));
		}
		if (std::shared_ptr<Or> orChild = std::dynamic_pointer_cast<Or>(child)) {
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

	clause_ptr dropOr(my_ptr<Or> orClause) {
		clause_ptr left = dropOr(orClause);
		clause_ptr right = dropOr(orClause);
		std::vector<clause_ptr> ls, rs;
		if (const auto& andLeft = std::dynamic_pointer_cast<And>(left)) {
			ls.push_back(andLeft->left);
			ls.push_back(andLeft->right);
		}
		else {
			ls.push_back(left);
		}
		if (const auto& andRight = std::dynamic_pointer_cast<And>(right)) {
			rs.push_back(andRight->left);
			rs.push_back(andRight->right);
		}
	}

	cnf_t simplify_cnf(const cnf_t& cnf) {
		cnf_t result;
		for (const auto& d : cnf) {
			if (!isDTrue(d)) {
				result.insert(d);
			}
		}
		return result;
	}

	bool isDTrue(const d_t& d) {
		for (const auto& e : d) {
			if (d.contains(e.inv())) return true;
		}
		return false;
	}

	cnf_t to_cnf(clause_ptr clause) {
		if (savedCnf.contains(clause)) {
			return savedCnf.at(clause);
		}
		cnf_t result;
		if (my_ptr<And> andClause = std::dynamic_pointer_cast<And>(clause)) {
			result = to_cnf_and(andClause);
		}
		if (my_ptr<Or> orClause = std::dynamic_pointer_cast<Or>(clause)) {
			result = to_cnf_or(orClause);
		}
		if (std::dynamic_pointer_cast<TClause>(clause)) {
			throw std::exception("True!");
		}
		if (std::dynamic_pointer_cast<FClause>(clause)) {
			throw std::exception("False!");
		}
		if (my_ptr<Lit> litClause = std::dynamic_pointer_cast<Lit>(clause)) {
			result = { {Elem(litClause->lit, false)} };
		}
		if (my_ptr<Not> notClause = std::dynamic_pointer_cast<Not>(clause)) {
			if (my_ptr<Lit> litClause = std::dynamic_pointer_cast<Lit>(notClause->child)) {
				result = { {Elem(litClause->lit, true)} };
			} else {
				throw std::exception("Not isn't in bottom!");
			}
		}
		result = simplify_cnf(result);
		savedCnf.insert_or_assign(clause, result);
		return result;
	}

	cnf_t to_cnf_and(my_ptr<And> andClause) {
		cnf_t left = to_cnf(andClause->left);
		cnf_t right = to_cnf(andClause->right);
		left.insert(right.begin(), right.end());
		return left;
	}

	cnf_t to_cnf_or(my_ptr<Or> orClause) {
		cnf_t left = to_cnf(orClause->left);
		cnf_t right = to_cnf(orClause->right);
		cnf_t result;
		//std::cout << left.size() << " - " << right.size() << std::endl;
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

	cnf_t to_cnf_fast(const clause_ptr& clause) {
		cnf_t cnf;
		Elem gate = to_cnf_fast(clause, cnf);
		cnf.insert({ gate });
		return cnf;
	}

	Elem to_cnf_fast(const clause_ptr& clause, cnf_t& cnf) {
		Elem c = Elem(std::move(std::to_string(varIndex) + '#'));
		varIndex++;
		//std::cout << c.name << " = " << *clause << std::endl;
		if (addedClauses.contains(clause)) {
			return {addedClauses.at(clause)};
		}
		if (const auto& andClause = std::dynamic_pointer_cast<And>(clause)) {
			Elem a = to_cnf_fast(andClause->left, cnf);
			Elem b = to_cnf_fast(andClause->right, cnf);
			cnf.insert({ a.inv(), b.inv(), c});
			cnf.insert({ a, c.inv()});
			cnf.insert({ b, c.inv()});
		}
		else if (const auto& orClause = std::dynamic_pointer_cast<Or>(clause)) {
			Elem a = to_cnf_fast(orClause->left, cnf);
			Elem b = to_cnf_fast(orClause->right, cnf);
			cnf.insert({ a, b, c.inv()});
			cnf.insert({ a.inv(), c});
			cnf.insert({ b.inv(), c});
		}
		else if (const auto& notClause = std::dynamic_pointer_cast<Not>(clause)) {
			if (const auto& litClause = std::dynamic_pointer_cast<Lit>(notClause->child)) {
				c = Elem(litClause->lit, true);
			}
			else {
				Elem a = to_cnf_fast(notClause->child, cnf);
				cnf.insert({ a, c });
				cnf.insert({ a.inv(), c.inv() });
			}
		}
		else if (const auto& litClause = std::dynamic_pointer_cast<Lit>(clause)) {
			c = Elem(litClause->lit);
		}
		addedClauses.insert({clause, c});
		return c;
	}
};