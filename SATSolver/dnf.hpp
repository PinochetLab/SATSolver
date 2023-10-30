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
		std::cout << "Start: " << clause->to_string() << std::endl << std::endl;
		std::shared_ptr<Clause> nClause = simplify(clause);
		std::cout << std::endl << "Final: " << nClause->to_string() << std::endl << std::endl;
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
				ors.insert(simplify(std::shared_ptr<Clause>(new Not(c))));
			}
			return simplify(std::shared_ptr<Clause>(new Or(ors)));
		}
		if (std::shared_ptr<Or> orChild = std::dynamic_pointer_cast<Or>(child)) {
			my_set ands;
			for (std::shared_ptr<Clause> c : orChild->ors) {
				ands.insert(simplify(std::shared_ptr<Clause>(new Not(c))));
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
		my_set cs;
		for (std::shared_ptr<Clause> cl : andClause->ands) {
			std::shared_ptr<Clause> c = simplify(cl);
			if (std::dynamic_pointer_cast<FClause>(c)) {
				return std::make_shared<FClause>();
			}
			if (std::dynamic_pointer_cast<TClause>(c)) {
				continue;
			}
			if (std::shared_ptr<And> andC = std::dynamic_pointer_cast<And>(c)) {
				my_set ms = andC->ands;
				cs.insert(ms.begin(), ms.end());
			}
			else if (std::shared_ptr<Or> orC = std::dynamic_pointer_cast<Or>(c)) {
				my_set ands(andClause->ands);
				ands.erase(c);
				std::shared_ptr<Clause> other = simplify(std::shared_ptr<Clause>(new And(ands)));
				my_set res;
				my_set myOrs = orC->ors;
				my_set otherOrs;
				if (std::shared_ptr<Or> orCl = std::dynamic_pointer_cast<Or>(other)) {
					otherOrs = orCl->ors;
				}
				else {
					otherOrs = { other };
				}
				for (std::shared_ptr<Clause> c1 : myOrs) {
					for (std::shared_ptr<Clause> c2 : otherOrs) {
						res.insert(simplify(std::shared_ptr<Clause>(new And({c1, c2}))));
					}
				}
				return std::make_shared<Or>(res);
			}
			else {
				cs.insert(c);
			}
		}

		for (std::shared_ptr<Clause> c : cs) {
			if (cs.contains(invert(c))) return std::make_shared<FClause>();
		}

		if (cs.empty()) return std::make_shared<TClause>();
		if (cs.size() == 1) return *cs.begin();
		return std::make_shared<And>(cs);
	}

	std::shared_ptr<Clause> simplifyOrClause(std::shared_ptr<Or> orClause) throw(std::exception) {
		my_set cs;
		for (std::shared_ptr<Clause> cl : orClause->ors) {
			std::shared_ptr<Clause> c = simplify(cl);
			if (std::dynamic_pointer_cast<TClause>(c)) return std::make_shared<TClause>();
			if (std::dynamic_pointer_cast<FClause>(c)) continue;
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

		if (cs.empty()) return std::make_shared<TClause>();
		if (cs.size() == 1) return *cs.begin();
		return std::make_shared<Or>(cs);
	}
};