#pragma once

#include <vector>
#include <map>
#include <set>

#include "cnf.hpp"

using sat2_answer_t = std::unordered_map<std::string, bool>;

class Sat2 {
private:
	int n;
	std::vector<std::vector<int>> g, gt;
	std::vector<bool> used;
	std::vector<int> order, comp;

	sat2_answer_t answer;
	std::unordered_map<std::string, int> indices;
	std::vector<std::string> names;
	std::unordered_map<int, std::vector<int>> ns;
	int index = 0;

	size_t get_index(const Elem& e) {
		return 2 * indices[e.name] + e.inv;
	}

public:
	Sat2(const std::vector<d_t>& cnf) {
		std::set<std::string> names;
		for (const auto& d : cnf) {
			std::vector<Elem> v(d.begin(), d.end());
			Elem e1 = v[0];
			Elem e2 = v[1];
			names.insert(e1.name);
			names.insert(e2.name);
		}
		n = names.size() * 2;
		g = std::vector<std::vector<int>>(n);
		gt = std::vector<std::vector<int>>(n);
		used = std::vector<bool>(n);
		order = std::vector<int>();
		comp = std::vector<int>(n, -1);

		this->names = std::vector<std::string>(names.begin(), names.end());

		for (const auto& name : names) {
			indices[name] = index;
			index++;
		}

		for (const auto& d : cnf) {
			std::vector<Elem> v(d.begin(), d.end());
			Elem e1 = v[0];
			Elem e2 = v[1];
			g[get_index(e1.inverted())].push_back(get_index(e2));
			g[get_index(e2.inverted())].push_back(get_index(e1));

			gt[get_index(e2)].push_back(get_index(e1.inverted()));
			gt[get_index(e1)].push_back(get_index(e2.inverted()));
		}
	}

	void dfs1(int v) {
		used[v] = true;
		for (size_t i = 0; i < g[v].size(); ++i) {
			int to = g[v][i];
			if (!used[to])
				dfs1(to);
		}
		order.push_back(v);
	}

	void dfs2(int v, int cl) {
		comp[v] = cl;
		for (size_t i = 0; i < gt[v].size(); ++i) {
			int to = gt[v][i];
			if (comp[to] == -1)
				dfs2(to, cl);
		}
	}

	bool check() {
		used.assign(n, false);

		for (int i = 0; i < n; ++i)
			if (!used[i])
				dfs1(i);

		comp.assign(n, -1);
		for (int i = 0, j = 0; i < n; ++i) {
			int v = order[n - i - 1];
			if (comp[v] == -1)
				dfs2(v, j++);
		}
		for (int i = 0; i < n; ++i) {
			if (comp[i] == comp[i ^ 1]) {
				return false;
			}
		}
		for (int i = 0; i < n; ++i) {
			if (comp[i] > comp[i ^ 1]) {
				answer.insert_or_assign(names[i / 2], i % 2);
			}
		}
		return true;
	}

	sat2_answer_t getAnswer() {
		return answer;
	}
};