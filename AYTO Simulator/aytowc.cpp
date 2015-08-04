/* Minimax Algorithm for Are You The One? TV Show */

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <cmath>

#define TEN_FACT (3628800)

using std::cout;
using std::cin;
using std::endl;
using std::vector;
using std::string;
using std::map;
using std::pair;
using std::find;
using std::abs;
using std::atoi;
using std::next_permutation;
using std::max_element;
using std::reverse;

int evaluate(const string &sol, const string &query);
vector<string> remove_perms(vector<string> &perms, int eval, string &query);
pair<string, int> guess_tb(vector<string> &perms, vector<string> &guessed_tb, int turn);
pair<string, int> guess_pm(vector<string> &perms, vector<string> &guessed, int turn);
int get_score(const string &s, vector<string> &perms);
int wc_response(string &guess, vector<string> &perms);
bool prcmp(pair<int, int> x, pair<int, int> y);
string get_worst_sequence(int sz);
void sequence_print(string s);

vector<string> ORIGPERMS;

int main(int argc, char **argv)
{
	int sz;
	const string digits = "0123456789";
	vector<string> possible_perms;
	bool feed = false;

	if (argc != 2) {
		cout << "usage: 'ayto npairs'" << endl;
		return 1;
	} else {
		if ((sz = atoi(argv[1])) < 0) {
			sz = -sz;
			feed = true;
		}
		if (sz < 3 || sz > 10) {
			cout << "usage: 'ayto npairs' where 3 <= npairs <= 10" << endl;;
			return 1;
		}
	}

	// initialize ORIGPERMS and possible_perms
	string range = digits.substr(0, sz);
	do {
		ORIGPERMS.push_back(range);
	} while (next_permutation(range.begin(), range.end()));
	possible_perms = ORIGPERMS;

	// run the game
	string hidden = get_worst_sequence(sz);
	if (feed) {
		cout << "Enter your hidden sequence (contiguous, no spaces): " << endl;
		cin >> hidden;
		if (hidden.size() > 10) {
			cout << "Invalid sequence" << endl;
			return 1;
		}
	}
	pair<string, int> tbguess;
	pair<string, int> pmguess;
	vector<string> guessed;
	vector<string> guessed_tb;
	int e;

	cout << "Running AYTO Simulation with Worst-Case Responses" << endl;

	for (int turn = 1; ; ++turn) {
		// stage one: truth booth
		cout << "**** Round " << turn << "A ****" << endl;
		cout << "Num. Possibilities: " << possible_perms.size() << endl;
		tbguess = guess_tb(possible_perms, guessed_tb, turn);
		cout << "Guess: ";
		sequence_print(tbguess.first);
		cout << endl;
		e = tbguess.second;
		cout << "Evaluation: " << e << endl;
		possible_perms = remove_perms(possible_perms, e, tbguess.first);

		// stage two: perfect matching
		cout << "Round " << turn << "B" << endl;
		cout << "Num. Possibilities: " << possible_perms.size() << endl;
		pmguess = guess_pm(possible_perms, guessed, turn);

		cout << "Guess: ";
		sequence_print(pmguess.first);
		cout << endl;
		e = pmguess.second;
		cout << "Evaluation: " << e << endl;
		if (e == sz) {
			cout << "Found ";
			sequence_print(pmguess.first);
			cout << " in " << turn << " guesses." << endl;
			break;
		}

		possible_perms = remove_perms(possible_perms, e, pmguess.first);
	}

	return 0;
}

// evaluate:  number of black hits from permutation or truth booth query
int evaluate(const string &sol, const string &query)
{
	int hits = 0;

	if (sol.size() == query.size()) {
		// permutation query
		int s = sol.size();
		for (int i = 0; i < s; i++) {
			if (sol[i] == query[i])
				++hits;
		}
	} else {
		// truth booth query
		if (sol[atoi(query.substr(0, 1).c_str())] == query[1])
			++hits;
	}

	return hits;
}

// remove_perms:  remove solutions that are no longer possible after an eval
vector<string> remove_perms(vector<string> &perms, int eval, string &query)
{
	vector<string> new_perms;

	for (vector<string>::iterator i = perms.begin(); i != perms.end(); i++) {
		if (evaluate(*i, query) == eval) {
			new_perms.push_back(*i);
		}
	}

	return new_perms;
}

// guess_tb:  guesses best pair (pos, val) to go to the truth booth
pair<string, int> guess_tb(vector<string> &possible_perms,
						   vector<string> &guessed_tb, int turn)
{
	static const string digits = "0123456789";
	int n = possible_perms[0].size();
	pair<string, int> next_guess;

	if (turn == 1) {
		next_guess.first = "00";
		next_guess.second = 0;
	} else if (possible_perms.size() == 1) {
		next_guess.first = "0" + possible_perms[0].substr(0, 1);
		next_guess.second = 1;
	} else {
		map<string, double> pair_to_count;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				pair_to_count[digits.substr(i, 1) + digits.substr(j, 1)] = 0;
			}
		}

		// count up the occurrences of each pair in the possible perms
		for (vector<string>::iterator p = possible_perms.begin();
			 p != possible_perms.end(); p++) {
			int len = possible_perms[0].size();
			for (int i = 0; i < len; i++) {
				pair_to_count[digits.substr(i, 1) + (*p).substr(i, 1)] += 1;
			}
		}

		double best_dist = 1;
		int perm_cnt = possible_perms.size();
		for (map<string, double>::iterator i = pair_to_count.begin();
			 i != pair_to_count.end(); i++) {
			if (find(guessed_tb.begin(), guessed_tb.end(), i->first)
				== guessed_tb.end()) {
				// hasn't been guessed yet
				if (abs(i->second/perm_cnt - .5) < best_dist) {
					next_guess.first = i->first;
					best_dist = abs(i->second/perm_cnt - .5);
					if (i->second / perm_cnt < 0.5) // occurs in < half perms
						next_guess.second = 0;
					else                            // occurs in >= half perms
						next_guess.second = 1;
				}
			}
		}
	}

	guessed_tb.push_back(next_guess.first);

	return next_guess;
}

// guess_pm:  guess a full permutation using minimax
pair<string, int> guess_pm(vector<string> &possible_perms,
						   vector<string> &guessed, int turn)
{
	static const string digits = "0123456789";
	pair<string, int> next_guess;
	int sz = possible_perms[0].size();

	// on first turn, we guess "0, 1, ..., n-1" if truth booth was correct
	// or "1, 0, ..., n-1" if truth booth was incorrect
	if (turn == 1) {
		int fact, i;
		for (i = 2, fact = 1; i <= sz; fact *= i++)
			;
		if (possible_perms.size() == fact) {
			next_guess.first = digits.substr(0, sz);
			next_guess.second = 1;
		} else {
			next_guess.first = "10" + digits.substr(2, sz - 2);
			next_guess.second = 1;
		}
	} else if (possible_perms.size() == 1) {
		next_guess.first = possible_perms[0];
		next_guess.second = possible_perms[0].size();
	} else {
		// run minimax to get next guess
		int best_score = TEN_FACT;
		int score;
		for (vector<string>::iterator i = ORIGPERMS.begin();
			 i != ORIGPERMS.end(); ++i) {
			if (find(guessed.begin(), guessed.end(), *i)
				== guessed.end()) {
				// hasn't already been guessed
				if ((score = get_score(*i, possible_perms)) < best_score) {
					best_score = score;
					next_guess.first = *i;
				}
			}
		}

		next_guess.second = wc_response(next_guess.first, possible_perms);
	}

	guessed.push_back(next_guess.first);

	return next_guess;
}

// get_score:  find the maximum number of remaining solutions over all
//             possible responses to the query s
int get_score(const string &s, vector<string> &perms)
{
	int sz = perms[0].size();
	vector<int> matches_eval(sz + 1, 0);

	// worst-case evaluation will be index m giving matches_eval[m] = max
	for (vector<string>::iterator p = perms.begin(); p != perms.end(); ++p) {
		++matches_eval[evaluate(s, *p)];
	}

	return *max_element(matches_eval.begin(), matches_eval.end());
}

// wc_response:  the response to guess that eliminates the least solutions
int wc_response(string &guess, vector<string> &perms)
{
	map<int, int> matches_eval;

	for (vector<string>::iterator it = perms.begin(); it!=perms.end(); ++it) {
		++matches_eval[evaluate(guess, *it)];
	}

	return max_element(matches_eval.begin(), matches_eval.end(), prcmp)->first;
}

// prcmp:  comparison function for pair<int, int> types in map
bool prcmp(pair<int, int> x, pair<int, int> y)
{
	return x.second < y.second;
}

// get_worst_sequence:  generates the worst-case minimax sequence (we think)
string get_worst_sequence(int sz)
{
	const string digits = "0123456789";
	string worst = digits.substr(0, sz);

	if (sz % 2 == 1) {
		reverse(worst.begin(), worst.end());
		return worst;
	} else {
		switch (sz) {
		case 10:
			return "9081726354";
		case 8:
			return "70615243";
		case 6:
			return "504132";
		case 4:
			return "3021";
		case 2:
			return "01";
		default:
			return "";
		}
	}
}

void sequence_print(const string s)
{
	for (string::const_iterator i = s.begin(); i != s.end(); i++) {
		if (i == s.begin())
			cout << "(";
		cout << *i;
		if (i != s.end() - 1)
			cout << ", ";
		else
			cout << ")";
	}
}