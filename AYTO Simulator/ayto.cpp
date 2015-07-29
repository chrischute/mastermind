/* Simulator for Are You The One? TV Show */
/* Multi-threaded Minimax Algorithm */

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <pthread.h>

#define TEN_FACT (3628800)
#define CHUNK_FRAC (8)
#define DEBUG (0)

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

typedef struct scoreargs Scoreargs;
struct scoreargs {
	vector<string> *chunk;
	vector<string> *perms;
	pair<string, int> *candidates;
	int thread_id;
};

int evaluate(const string &sol, const string &query);
vector<string> remove_perms(vector<string> &perms, int eval, string &query);
string guess_tb(vector<string> &perms, vector<string> &guessed_tb, int turn);
string guess_pm(vector<string> &perms, vector<string> &guessed, int turn);
void *get_score(void *argstruct);
string get_worst_sequence(int sz);
void sequence_print(string s);
void make_chunks(vector<string> &orig, vector<vector<string> > &chunks, int n);

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
	string tbguess;
	string pmguess;
	vector<string> guessed;
	vector<string> guessed_tb;
	int e;

	cout << "Running Simulation on Hidden Vector: ";
	sequence_print(hidden);
	cout << endl;

	for (int turn = 1; ; ++turn) {
		// stage one: truth booth
		cout << "**** Round " << turn << "A ****" << endl;
		cout << "Num. Possibilities: " << possible_perms.size() << endl;
		tbguess = guess_tb(possible_perms, guessed_tb, turn);
		cout << "Guess: ";
		sequence_print(tbguess);
		cout << endl;
		e = evaluate(hidden, tbguess);
		cout << "Evaluation: " << e << endl;
		possible_perms = remove_perms(possible_perms, e, tbguess);

		// stage two: perfect matching
		cout << "Round " << turn << "B" << endl;
		cout << "Num. Possibilities: " << possible_perms.size() << endl;
		pmguess = guess_pm(possible_perms, guessed, turn);

		cout << "Guess: ";
		sequence_print(pmguess);
		cout << endl;
		e = evaluate(hidden, pmguess);
		cout << "Evaluation: " << e << endl;
		if (e == sz) {
			cout << "Found ";
			sequence_print(hidden);
			cout << " in " << turn << " guesses." << endl;
			break;
		}

		possible_perms = remove_perms(possible_perms, e, pmguess);
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
string guess_tb(vector<string> &possible_perms,
				vector<string> &guessed_tb, int turn)
{
	static const string digits = "0123456789";
	int n = possible_perms[0].size();

	if (turn == 1) {
		guessed_tb.push_back("00");
		return "00";
	}

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

	std::pair<string, double> best_pair;
	best_pair.first = "";
	best_pair.second = 0;
	double best_dist = 1;
	double dist;
	int perm_cnt = possible_perms.size();
	for (map<string, double>::iterator i = pair_to_count.begin();
		 i != pair_to_count.end(); i++) {
		if (find(guessed_tb.begin(), guessed_tb.end(), i->first)
			== guessed_tb.end()) {
			// hasn't been guessed yet
			if ((dist = abs(i->second/perm_cnt - .5)) < best_dist) {
				best_pair = *i;
				best_dist = dist;
			}
		}
	}

	guessed_tb.push_back(best_pair.first);

	return best_pair.first;
}

// guess_pm:  guess a full permutation using minimax
string guess_pm(vector<string> &possible_perms, vector<string> &guessed, int turn)
{
	static const string digits = "0123456789";
	string next_guess;
	int sz = possible_perms[0].size();
	int perm_cnt = possible_perms.size();
	int tflag;

	// on first turn, we guess "0, 1, ..., n-1" if truth booth was correct
	// or "1, 0, ..., n-1" if truth booth was incorrect
	if (turn == 1) {
		int fact, i;
		for (i = 2, fact = 1; i <= sz; fact *= i++)
			;
		if (possible_perms.size() == fact) {
			next_guess = digits.substr(0, sz);
		} else {
			next_guess = "10" + digits.substr(2, sz - 2);
		}
	} else if (perm_cnt == 1) {
			return possible_perms[0];
	} else {
		// run minimax with multithreading to get next guess
		// (i) allocate components of all scoreargs structs, put in array
		vector<vector<string> > chunks;
		make_chunks(ORIGPERMS, chunks, CHUNK_FRAC);
		pair<string, int> *cd = (pair<string, int> *) malloc(sizeof(pair<string, int>) * CHUNK_FRAC);
		pthread_t *jobs = (pthread_t *) malloc(sizeof(pthread_t) * CHUNK_FRAC);
		Scoreargs **args = (Scoreargs **) malloc(sizeof(Scoreargs *));
		assert(args);
		for (int i = 0; i < CHUNK_FRAC; ++i) {
			args[i] = (Scoreargs *) malloc(sizeof(Scoreargs));
			assert(args[i]);
			args[i]->thread_id = i;
			args[i]->chunk = &(chunks[i]);
			args[i]->perms = &possible_perms;
			args[i]->candidates = cd;
		}
		
		// (ii) start all threads evaluating scores
		for (int i = 0; i < CHUNK_FRAC; ++i) {
			tflag = pthread_create(&(jobs[i]), NULL,
								   get_score, (void *) args[i]);
			assert(tflag == 0);
		}

		// (iii) wait for all threads to finish
		for (int i = 0; i < CHUNK_FRAC; ++i) {
			pthread_join(jobs[i], NULL);
		}

		// (iv) get minimum score from candidates
		pair<string, int> low_score;
		low_score.second = possible_perms.size();
		for (int i = 0; i < CHUNK_FRAC; i++) {
			if (cd[i].second < low_score.second) {
				low_score = cd[i];
			}
		}
		next_guess = low_score.first;

		// (v) clean up
		free(cd);
		free(jobs);
		for (int i = 0; i < CHUNK_FRAC; i++)
			free(args[i]);
		free(args);
	}

	guessed.push_back(next_guess);

	return next_guess;
}

// make_chunks:  return pointers to n (nearly) equally sized vectors
//                from the original vector
void make_chunks(vector<string> &orig, vector<vector<string> > &chunks, int n)
{
	int sz = orig.size();
	int chunk_sz = sz / n;
	int n_with_extra = sz % n;
	vector<string>::iterator b = orig.begin();
	vector<string>::iterator e;

	for (int i = 0; i < n; i++) {
		int m = chunk_sz;    // size of this chunk
		if (n_with_extra) {
			++m;
			--n_with_extra;
		}
		e = b + m;
		vector<string> subvec(b, e);
		chunks.push_back(subvec);
		b = e;
	}
}

// get_score:  find the maximum number of remaining solutions over all
//             possible responses to the query s
void *get_score(void *argstruct)
{
	typedef vector<string>::iterator Viter;

	// break up the argstruct, needed for POSIX thread
	Scoreargs *args = (Scoreargs *) argstruct;
	vector<string> *chunk = args->chunk;
	vector<string> *perms = args->perms;
	pair<string, int> *cd = args->candidates;
	int id = args->thread_id;

	int sz = (*perms)[0].size();
	int score;
	pair<string, int> best_guess;
	best_guess.second = perms->size();

	assert(chunk->size() > 0);

	for (Viter guess = chunk->begin(); guess != chunk->end(); ++guess) {
		vector<int> matches_eval(sz + 1, 0);

		// worst-case evaluation will be index m giving matches_eval[m] = max
		for (Viter p = perms->begin(); p != perms->end(); ++p) {
			++matches_eval[evaluate(*guess, *p)];
		}

		score = *max_element(matches_eval.begin(), matches_eval.end());
		if (score < best_guess.second) {
			best_guess.first = *guess;
			best_guess.second = score;
		}
	}

#ifdef DEBUG
	cout << "Sample from chunks is " << (*chunk)[0] << endl;
	cout << "Adding candidate pair<" << best_guess.first << ", "
		 << best_guess.second << "> into cd[" << id << "]" << endl;
#endif
	cd[id] = best_guess;

	return NULL;
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
