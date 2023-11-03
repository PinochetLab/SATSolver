#pragma once
#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

struct Clause {
public:
	size_t count;
	virtual bool operator==(const Clause& other) const {
		return true;
	}

	friend std::ostream& operator << (std::ostream& os, const Clause& clause) {
		clause.print(os);
		return os;
	}

	virtual void print(std::ostream& os) const = 0;

	virtual int hash() const = 0;

	virtual std::string prettyString() const = 0;

	virtual std::string opString() const = 0;
};

template<class T>
using my_ptr = std::shared_ptr<T>;

using clause_ptr = my_ptr<Clause>;

using my_set = std::unordered_set<clause_ptr>;

struct TClause : public Clause {
public:
	TClause() {
		count = 1; 
	}

	void print(std::ostream& os) const override {
		os << "True";
	}

	int hash() const override {
		return 1;
	}

	std::string prettyString() const override {
		return opString();
	}

	std::string opString() const override {
		return "True";
	}
};

struct FClause : public Clause {
public:
	FClause() {
		count = 1;
	}

	void print(std::ostream& os) const override {
		os << "False";
	}

	int hash() const override {
		return 2;
	}

	std::string prettyString() const override {
		return opString();
	}

	std::string opString() const override {
		return "False";
	}
};

struct Lit : public Clause {
public:
	std::string lit;

	Lit(const std::string& lit) : lit(lit) {
		count = 1;
	}

	void print(std::ostream& os) const override {
		os << lit;
	}

	int hash() const override {
		std::hash<std::string> h;
		return h(lit) + 3;
	}

	std::string prettyString() const override {
		return opString();
	}

	std::string opString() const override {
		return lit;
	}
};

struct Not : public Clause {
public:
	clause_ptr child;

	Not(clause_ptr child) : child(child) {
		count = 1 + child->count;
	}

	void print(std::ostream& os) const override {
		os << "Not(" << *child << ")";
	}

	int hash() const override {
		std::hash<clause_ptr> h;
		return h(child) << 16 + 5;
	}

	std::string prettyString() const override {
		return "Not(" + child->opString() + ")";
	}

	std::string opString() const override {
		return "Not";
	}
};


struct Or : public Clause {
public:
	clause_ptr left;
	clause_ptr right;

	Or(clause_ptr left, clause_ptr right) : left(left), right(right) {
		count = 1 + left->count + right->count;
	}

	void print(std::ostream& os) const override {
		os << "Or(" << *left << ", " << *right << ")";
	}

	int hash() const override {
		std::hash<clause_ptr> h;
		return h(left) << 32 + h(right) << 16 + 7;
	}

	std::string prettyString() const override {
		return "Or(" + left->opString() + ", " + right->opString() + ")";
	}

	std::string opString() const override {
		return "Or";
	}
};


struct And : public Clause {
public:
	clause_ptr left;
	clause_ptr right;

	And(clause_ptr left, clause_ptr right) : left(left), right(right) {
		count = 1 + left->count + right->count;
	}

	void print(std::ostream& os) const override {
		os << "And(" << *left << ", " << *right << ")";
	}

	int hash() const override {
		std::hash<clause_ptr> h;
		return h(left) << 16 + h(right) << 8 + 11;
	}

	std::string prettyString() const override {
		return "And(" + left->opString() + ", " + right->opString() + ")";
	}

	std::string opString() const override {
		return "And";
	}
};