#pragma once

#include <cassert>

#include "clauses.hpp"

inline auto mh = [](const std::string& c) {return std::hash<std::string>{}(c); };
inline auto me = [](const std::string& c1, const std::string& c2) ->
	bool{return c1 == c2; };

using my_map = std::unordered_map<std::string, std::shared_ptr<Clause>, decltype(mh), decltype(me)>;

class Dnf {
private:
	my_map bakedInverted;
	my_map bakedSimplified;
public:
	Dnf() { }

	bool isSolvable(const std::vector<std::shared_ptr<Clause>>& clauses) throw(std::exception) {
		my_set ms(clauses.begin(), clauses.end());
		std::shared_ptr<Clause> clause = clauses.size() == 1 ? clauses[0] : std::make_shared<And>(ms);
		std::shared_ptr<Clause> nClause = simplify(clause);
		//std::cout << nClause->to_string() << '\n';
		if (std::shared_ptr<FClause> f = std::dynamic_pointer_cast<FClause>(nClause)) {
			return false;
		}
		return true;
	}

	std::shared_ptr<Clause> simplify(std::shared_ptr<Clause> clause) throw(std::exception) {
		if (bakedSimplified.contains(clause->to_string())) {
			return bakedSimplified.at(clause->to_string());
		}
		std::shared_ptr<Clause> result;
		if (std::shared_ptr<TClause> tClause = std::dynamic_pointer_cast<TClause>(clause)) {
			result = simplifyTClause(tClause);
		}
		else if (std::shared_ptr<FClause> fClause = std::dynamic_pointer_cast<FClause>(clause)) {
			result = simplifyFClause(fClause);
		}
		else if (std::shared_ptr<Lit> litClause = std::dynamic_pointer_cast<Lit>(clause)) {
			result = simplifyLitClause(litClause);
		}
		else if (std::shared_ptr<Not> notClause = std::dynamic_pointer_cast<Not>(clause)) {
			result = simplifyNotClause(notClause);
		}
		else if (std::shared_ptr<And> andClause = std::dynamic_pointer_cast<And>(clause)) {
			result = simplifyAndClause(andClause);
		}
		else if (std::shared_ptr<Or> orClause = std::dynamic_pointer_cast<Or>(clause)) {
			result = simplifyOrClause(orClause);
		}
		else {
			result = std::make_shared<FClause>();
		}
		std::cout << clause->to_string() << " => " << result->to_string() << std::endl;
		bakedSimplified.insert_or_assign(clause->to_string(), result);
		return result;
	}

	std::shared_ptr<Clause> simplifyTClause(std::shared_ptr<TClause> tClause) {
		return std::shared_ptr<TClause>(new TClause());
	}

	std::shared_ptr<Clause> simplifyFClause(std::shared_ptr<FClause> fClause) {
		return std::shared_ptr<FClause>(new FClause());
	}

	std::shared_ptr<Clause> simplifyLitClause(std::shared_ptr<Lit> litClause) {
		return std::shared_ptr<Lit>(new Lit(litClause->lit));
	}

	std::shared_ptr<Clause> simplifyNotClause(std::shared_ptr<Not> notClause) throw(std::exception) {
		std::shared_ptr<Clause> child = simplify(notClause->child);
		if (std::shared_ptr<Not> notChild = std::dynamic_pointer_cast<Not>(child)) {
			return notChild->child;
		}
		if (std::shared_ptr<And> andChild = std::dynamic_pointer_cast<And>(child)) {
			my_set ors;
			for (std::shared_ptr<Clause> c : andChild->ands) {
				ors.insert(std::shared_ptr<Clause>(new Not(c)));
			}
			return simplify(std::shared_ptr<Clause>(new Or(ors)));
		}
		if (std::shared_ptr<Or> orChild = std::dynamic_pointer_cast<Or>(child)) {
			my_set ands;
			for (std::shared_ptr<Clause> c : orChild->ors) {
				ands.insert(std::shared_ptr<Clause>(new Not(c)));
			}
			return simplify(std::shared_ptr<Clause>(new And(ands)));
		}
		if (std::dynamic_pointer_cast<TClause>(child)) {
			return std::make_shared<FClause>();
		}
		if (std::dynamic_pointer_cast<FClause>(child)) {
			return std::make_shared<TClause>();
		}
		return std::make_shared<Not>(child);
	}

	std::shared_ptr<Clause> invert(std::shared_ptr<Clause> clause) throw(std::exception) {
		if (bakedInverted.contains(clause->to_string())) {
			return bakedInverted.at(clause->to_string());
		}
		std::shared_ptr<Clause> result = simplify(std::shared_ptr<Clause>(new Not(clause)));
		bakedInverted.insert_or_assign(clause->to_string(), result);
		return result;
	}

	std::shared_ptr<Clause> simplifyAndClause(std::shared_ptr<And> andClause) throw(std::exception) {
		//std::cout << "andClause: " << andClause->to_string() << std::endl;
		my_set cs = andClause->ands;
		if (cs.empty()) return std::make_shared<FClause>();
		if (cs.size() == 1) return simplify(*cs.begin());
		size_t half = cs.size() / 2;
		auto it = cs.begin();
		for (size_t i = 0; i < half; i++) it++;
		my_set part1(cs.begin(), it);
		my_set part2(it, cs.end());
		std::shared_ptr<Clause> p1 = simplify(std::make_shared<And>(part1));
		std::shared_ptr<Clause> p2 = simplify(std::make_shared<And>(part2));
		

		std::shared_ptr<Or> p1Or = std::dynamic_pointer_cast<Or>(p1);
		std::shared_ptr<Or> p2Or = std::dynamic_pointer_cast<Or>(p2);


		if (p1Or || p2Or) {
			my_set ors1;
			my_set ors2;
			my_set ors;
			if (p1Or) ors1 = std::move(p1Or->ors);
			else ors1 = { p1 };

			if (p2Or) ors2 = std::move(p2Or->ors);
			else ors2 = { p2 };

			for (std::shared_ptr<Clause> or1 : ors1) {
				for (std::shared_ptr<Clause> or2 : ors2) {
					ors.insert(simplify(std::make_shared<And>(my_set{ or1, or2 })));
				}
			}

			return simplify(std::make_shared<Or>(ors));
		}
		else {
			my_set ms1{p1, p2};
			my_set ms;
			for (std::shared_ptr<Clause> p : ms1) {
				if (std::dynamic_pointer_cast<FClause>(p)) return std::make_shared<FClause>();
				if (std::dynamic_pointer_cast<TClause>(p)) continue;
				if (std::shared_ptr<And> andP = std::dynamic_pointer_cast<And>(p)) {
					my_set mys = andP->ands;
					ms.insert(mys.begin(), mys.end());
				}
				else {
					ms.insert(p);
				}
			}

			for (std::shared_ptr<Clause> p : ms) {
				if (ms.contains(invert(p))) return std::make_shared<FClause>();
			}

			if (ms.empty()) return std::make_shared<TClause>();
			if (ms.size() == 1) return *ms.begin();

			return std::make_shared<And>(ms);
		}
	}

	std::shared_ptr<Clause> simplifyOrClause(std::shared_ptr<Or> orClause) throw(std::exception) {
		my_set cs;
		bool containsFalse = false;
		for (std::shared_ptr<Clause> cl : orClause->ors) {
			std::shared_ptr<Clause> c = simplify(cl);
			if (std::dynamic_pointer_cast<TClause>(c)) return std::make_shared<TClause>();
			if (std::dynamic_pointer_cast<FClause>(c)) {
				containsFalse = true;
				continue;
			}
			if (std::shared_ptr<Or> orC = std::dynamic_pointer_cast<Or>(c)) {
				my_set ms = orC->ors;
				cs.insert(ms.begin(), ms.end());
			}
			else {
				cs.insert(c);
			}
		}

		for (std::shared_ptr<Clause> c : cs) {
			if (cs.contains(invert(c))) return std::make_shared<TClause>();
		}

		if (cs.empty()) {
			if (containsFalse) {
				return std::make_shared<FClause>();
			}
			else {
				return std::make_shared<TClause>();
			}
		}
		if (cs.size() == 1) return *cs.begin();
		return std::make_shared<Or>(cs);
	}
};