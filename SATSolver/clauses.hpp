#pragma once
#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

struct Clause {
protected:
	std::string s;
public:
	std::string to_string() {
		return s;
	}

	virtual bool operator==(const Clause& other) const {
		return true;
	}
};

inline auto myHash = [](std::shared_ptr<Clause> c) {return std::hash<std::string>{}(c->to_string()); };
inline auto myEqual = [](const std::shared_ptr<Clause>& c1, const std::shared_ptr<Clause>& c2) {return c1->to_string() == c2->to_string(); };

using my_set = std::unordered_set<std::shared_ptr<Clause>, 
	decltype(myHash),
	decltype(myEqual)>;

struct TClause : public Clause {
public:
	TClause() {
		s = "True";
	}
};

struct FClause : public Clause {
public:
	FClause() {
		s = "False";
	}
};

struct Lit : public Clause {
public:
	std::string lit;

	Lit(const std::string& lit) : lit(lit) {
		s = lit;
	}
};

struct Not : public Clause {
public:
	std::shared_ptr<Clause> child;

	Not(std::shared_ptr<Clause> child) : child(child) {
		s = "Not(" + child->to_string() + ")";
	}
};


struct Or : public Clause {
public:
	my_set ors;

	Or(my_set ors) : ors(ors) {
		s = "Or(";
		for (auto o : ors) {
			s += (o->to_string() + ",");
		}
		s = s.substr(0, s.length() - 1) + ')';
	}
};


struct And : public Clause {
public:
	my_set ands;

	And(my_set ands) : ands(ands) {
		s = "And(";
		for (auto o : ands) {
			s += (o->to_string() + ",");
		}
		s = s.substr(0, s.length() - 1) + ')';
	}
};