#include <cstdio>
#include <sstream>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <vector>
#include "disjoint.h"
#include <tuple>
#include <set>

using namespace std;

#define talloc(type, num) (type *) malloc(sizeof(type)*(num))

class Superball {
	public:
		Superball(int argc, char **argv);// read in basic info and update vectors and values
		~Superball();
		int r;
		int c;
		int mss;
		int empty;
		vector <int> board;
		vector <int> goals;
		vector <int> colors;
		DisjointSet *djs;// disjointSet object
		double board_evaluation(vector<int>&); // evaluate a board and return a points for this board
		void check_around(int index, vector<int>& check_board, DisjointSet *pointer);
		void next();
		void swap(double original_points);
		void score(vector<tuple<int, int>>& scorevec);
		int distance(int i, int j);
		vector<tuple<int, int>> analyze();
		tuple<int, int> about_to_score(vector<tuple<int, int>>& scorevec);
		tuple<int, int> swap_bring_benefit(tuple<int, int>index_size);
		tuple<int, int> benefit_check(vector<int>&temp_board, tuple<int, int>index_size);
};

// check argv
void usage(const char *s) 
{
	fprintf(stderr, "usage: sb-read rows cols min-score-size colors\n");
	if (s != NULL) fprintf(stderr, "%s\n", s);
	exit(1);
}

Superball::~Superball(){
	delete djs;
}

// pass in the index of which entry is being checked on check_board, connect corresponding sets of disjointset
void Superball::check_around(int index, vector<int>& check_board, DisjointSet *pointer){
	int up, down, left, right;// up cell index ...

	up = index - c; // find up cell
	down = index + c; // find down cell

	// find left cell
	if(index % c == 0) left = -1;
	else left = index - 1;
	// find right cell
	if((index + 1) % c == 0) right = -1;
	else right = index + 1;

	int upcol, downcol, leftcol, rightcol;//up-color, down-color ...

	// find colors of up/down/left/right cells
	if(up >= 0) upcol = check_board[up];
	else upcol = -1;
	if(down <= r * c - 1) downcol = check_board[down];
	else downcol = -1;
	if(left != -1) leftcol = check_board[left];
	else leftcol = -1;
	if(right != -1) rightcol = check_board[right];
	else rightcol = -1;

	int indexcol = check_board[index];//index color
	int s1, s2;
	if(indexcol != upcol && indexcol != downcol && indexcol != leftcol && indexcol != rightcol) return;
	else{
		// if index color = up cell color
		if(indexcol == upcol){
			s1 = pointer -> Find(index);
			s2 = pointer -> Find(up);
			// if up cell is not connected with index cell, connect them and check on up cell
			if(s1 != s2){
				pointer -> Union(s1, s2);
				check_around(up, check_board, pointer);
			}
		}
		if(indexcol == downcol){
			s1 = pointer -> Find(index);
			s2 = pointer -> Find(down);
			if(s1 != s2){
				pointer -> Union(s1, s2);
				check_around(down, check_board, pointer);
			}
		}
		if(indexcol == leftcol){
			s1 = pointer -> Find(index);
			s2 = pointer -> Find(left);
			if(s1 != s2){
				pointer -> Union(s1, s2);
				check_around(left, check_board, pointer);
			}
		}
		if(indexcol == rightcol){
			s1 = pointer -> Find(index);
			s2 = pointer -> Find(right);
			if(s1 != s2){
				pointer -> Union(s1, s2);
				check_around(right, check_board, pointer);
			}
		}
	}
}
//return a vector of ready-to-score pairs (scoring_index, scoring_size)
//(index, size)
vector<tuple<int, int>> Superball::analyze(){
	int i, j, size, root;
	set<int> root_set;//set of scoring_block
	set<int>::iterator rit;
	vector<tuple<int, int>> pair; //(index, size)
	// only check goal cells
	for(i = 0; i < goals.size(); i ++){
		if(goals[i] == 1 && board[i] != '.' && board[i] != '*'){
			check_around(i, board, djs);
			size = djs -> Size(i);
			// if block is large enough
			if(size >= mss){
				root = djs -> Find(i);
				rit = root_set.find(root);
				if(rit == root_set.end()){
					root_set.insert(root);
					pair.push_back(make_pair(i, size));
				}
			}
		}
	}
	return pair;
}

Superball::Superball(int argc, char **argv){
	int i, j;
	string s;

	// command arguments check
	if (argc != 5) usage(NULL);

	if (sscanf(argv[1], "%d", &r) == 0 || r <= 0) usage("Bad rows");
	if (sscanf(argv[2], "%d", &c) == 0 || c <= 0) usage("Bad cols");
	if (sscanf(argv[3], "%d", &mss) == 0 || mss <= 0) usage("Bad min-score-size");

	colors.resize(256, 0);

	for (i = 0; i < strlen(argv[4]); i++) {
		if (!isalpha(argv[4][i])) usage("Colors must be distinct letters");
		if (!islower(argv[4][i])) usage("Colors must be lowercase letters");
		if (colors[argv[4][i]] != 0) usage("Duplicate color");

		// lowercase and uppercase letter entries in colors vector are now defined
		colors[argv[4][i]] = 2+i;
		colors[toupper(argv[4][i])] = 2+i;
	}

	board.resize(r*c);
	goals.resize(r*c, 0);

	empty = 0;

	// input file check
	for (i = 0; i < r; i++) {
		if (!(cin >> s)) {
			fprintf(stderr, "Bad board: not enough rows on standard input\n");
			exit(1);
		}
		if (s.size() != c) {
			fprintf(stderr, "Bad board on row %d - wrong number of characters.\n", i);
			exit(1);
		}
		for (j = 0; j < c; j++) {
			// if condition satified, meaning s[j] is not */./letter (either lowercase or uppercase)
			if (s[j] != '*' && s[j] != '.' && colors[s[j]] == 0) {
				fprintf(stderr, "Bad board row %d - bad character %c.\n", i, s[j]);
				exit(1);
			}
			board[i*c+j] = s[j];
			if (board[i*c+j] == '.') empty++;
			if (board[i*c+j] == '*') empty++;
			if (isupper(board[i*c+j]) || board[i*c+j] == '*') {
				goals[i*c+j] = 1;
				board[i*c+j] = tolower(board[i*c+j]);
			}
		}
	}

	// disjointSet setup
	djs = new DisjointSetBySize(r * c);
}

int string_to_int(string a){
	istringstream sin;
	sin.clear();
	sin.str(a);
	int b;
	sin >> b;
	return b;
}

string int_to_string(int a){
	stringstream sin;
	sin.clear();
	sin << a;
	string b = sin.str();
	return b;
}

// imagine the real 8*10 board, pass in the index of two cells in a vector and return their distance: if adjacent, distance is 0
// if vertex to vertex, distance is 1
int Superball::distance(int i, int j){
	return (abs(i/c - j/c) + abs(i%c -j%c) - 1);
}

// pass in the original board score and return SWAP instruction based on point evaluation
void Superball::swap(double original_points){
	vector<int> tempvec;
	string instruction, command;
	int i, j, temp, x, y;
	double points;
	for(i = 0; i < board.size() - 1; i ++){
		if(board[i] != '*' && board[i] != '.'){
			x = i;
			break;
		}
	}
	for(j = board.size() - 1; j >= 0; j --){
		if(board[j] != '*' && board[j] != '.' && board[j] != board[x]){
			y = j;
			break;
		}
	}
	map<double, string> point_system; // <points, instruction>
	map<double, string>::reverse_iterator pit;
	for(i = 0; i < board.size() - 1; i ++){
		if(board[i] != '*' && board[i] != '.'){
			for(j = i + 1; j < board.size(); j ++){
				if(board[j] != '*' && board[j] != '.' && board[j] != board[i]){
					tempvec.clear();
					tempvec = board;
					temp = tempvec[i];
					tempvec[i] = tempvec[j];
					tempvec[j] = temp;
					// tempvec is the board after being swapped
					points = board_evaluation(tempvec);
					if(points > original_points){ // this part can be simplified
						original_points = points;
						x = i;
						y = j;
					}
				}
			}
		}
	}
	printf("SWAP %i %i %i %i\n", x/c, x%c, y/c, y%c);
}

// pass in to_be_evaluated board and (socring_index, scoring_block_size)
// return (1, better_scoring_block_size) or (-1, -1)
tuple<int, int> Superball::benefit_check(vector<int>&temp_board, tuple<int, int>index_size){	
	int old_size = get<1>(index_size);
	int scoring_index = get<0>(index_size);
	DisjointSet *pointer = new DisjointSetBySize(r * c);
	int i, j, x, y, root;
	for(i = 0; i < temp_board.size(); i ++){
		if(temp_board[i] != '.' && temp_board[i] != '*'){
			check_around(i, temp_board, pointer);
		}
	}
	int new_size = pointer -> Size(scoring_index);
	if(new_size - old_size >= 4){//could modify here!!!!!!!!!!!!!!!!!!!!!!! 4/5 is better than 3
		delete pointer;
		return make_pair(1, new_size);
	}else{
		delete pointer;
		return make_pair(-1, -1);
	}
}	

// pass in ready_to_scoring (index, size)
// return (x, y)  (swaping index) if there is a better swap before scoring or (-1, -1)
tuple<int, int> Superball::swap_bring_benefit(tuple<int, int>index_size){
	vector<int> tempvec;
	int i, j, temp;
	int better, new_size;
	int old_size = get<1>(index_size);
	int x = -1;
	int y = -1;
	for(i = 0; i < board.size() - 1; i ++){
		if(board[i] != '*' && board[i] != '.'){
			for(j = i + 1; j < board.size(); j ++){
				if(board[j] != '*' && board[j] != '.' && board[j] != board[i]){
					tempvec.clear();
					tempvec = board;
					temp = tempvec[i];
					tempvec[i] = tempvec[j];
					tempvec[j] = temp;
					// tempvec is the board after being swapped

					// pass in to_be_evaluated board and (socring_index, scoring_block_size)
					// return (1, better_scoring_block_size) or (-1, -1)
					//tuple<int, int> benefit_check(vector<int>&temp_board, tuple<int, int>index_size){	

					better = get<0>(benefit_check(tempvec, index_size));
					new_size = get<1>(benefit_check(tempvec, index_size));
					if(better == 1){
						if(new_size > old_size){
							old_size = new_size;
							x = i;
							y = j;
						}
					}
				}
				}
			}
		}
		// before scoring, one more swap brings in a larger scoring_block (3/4/5 larger)
		return make_pair(x, y);
	}

	// return the largest block size of ready_to_score (cell index, block_size)
	tuple<int, int> Superball::about_to_score(vector<tuple<int, int>>& scorevec){
		int i, size, max_size, index;
		max_size = get<1>(scorevec[0]);
		index = get<0>(scorevec[0]);

		// multiple can score, score the largest size
		if(scorevec.size() > 1){
			for(i = 1; i < scorevec.size(); i ++){
				size = get<1>(scorevec[i]);
				if(size > max_size){
					max_size = size;
					index = get<0>(scorevec[i]);
				}
			}
		}
		return make_pair(index, max_size);
	}

	// pass in the ready to score blocks and choose the largest block to score
	void Superball::score(vector<tuple<int, int>>& scorevec){
		int i, max_size, size, index;
		max_size = get<1>(scorevec[0]);
		index = get<0>(scorevec[0]);

		// multiple can score, score the largest size
		if(scorevec.size() > 1){
			for(i = 1; i < scorevec.size(); i ++){
				size = get<1>(scorevec[i]);
				if(size > max_size){
					max_size = size;
					index = get<0>(scorevec[i]);
				}
			}
		}
		printf("SCORE %i %i\n", index/c, index%c);
		return;
	}

	// simply counting set num, the less num of sets, the better with consideration of whether on goal cell
	double Superball::board_evaluation(vector<int>&temp_board){
		DisjointSet *pointer = new DisjointSetBySize(r * c);
		int i, j, x, y, root;
		for(i = 0; i < temp_board.size(); i ++){
			if(temp_board[i] != '.' && temp_board[i] != '*'){
				check_around(i, temp_board, pointer);
			}
		}
		//(root, num_of_times_root_being_hit, num_of_goalcell_on_this_block, block_size)
		vector<tuple<int, int, int, int>>num_sets; // (root, num, goal_cell, size)
		num_sets.clear();
		bool found = false;
		//bool same_color = false;

		// simply counting how many sets
		for(i = 0; i < temp_board.size(); i ++){
			if(temp_board[i] != '.' && temp_board[i] != '*'){

				/*
				// for (no block, all single seperate cell) use
				if(!same_color){
				for(j = i + 1; j < temp_board.size(); j ++){
				if(temp_board[i] == temp_board[j]){
				same_color = true;
				x = i;
				y = j;
				}
				}
				}
				 *//////////////
				found = false;
				x = pointer -> Find(i); // index of the root of the block
				y = pointer -> Size(i);
				//vector<tuple<int, int, int, int>>num_sets; // (root, num, goal_cell, size)
				for(j = 0; j < num_sets.size(); j ++){
					// this block already in the num_sets
					if(x == get<0>(num_sets[j])){
						get<1>(num_sets[j]) ++;
						if(goals[i] == 1)  get<2>(num_sets[j]) ++;
						found = true;
						break;
					}
				}
				if(!found && goals[i] == 1) num_sets.push_back(make_tuple(x, 1, 1, y));
				else if(!found && goals[i] != 1) num_sets.push_back(make_tuple(x, 1, 0, y));
			}	
		}

		/*
		//no blocks, all single seperate cells and there are same colors
		if(num_sets.size() == r*c - empty && same_color){
		return (-1) * 100 * distance(x, y);
		}
		 */
		//(root, num_of_times_root_being_hit, num_of_goalcell_on_this_block, block_size)
		//vector<tuple<int, int, int, int>>num_sets; // (root, num, goal_cell, size)
		int block1, block2, block3, block4, block_enough;
		int goal_1, goal_2, goal_3, goal_4, goal_enough;
		block1 = 0;
		block2 = 0;
		block3 = 0;
		block4 = 0;
		block_enough = 0;
		goal_1 = 0;
		goal_2 = 0;
		goal_3 = 0;
		goal_4 = 0;
		goal_enough = 0;
		for(i = 0; i < num_sets.size(); i ++){
			if(get<3>(num_sets[i]) == 1){
				block1 ++;
				if(get<2>(num_sets[i]) != 0) goal_1 ++;
			}
			else if(get<3>(num_sets[i]) == 2){
				block2 ++;
				if(get<2>(num_sets[i]) != 0) goal_2 ++;
			}
			else if(get<3>(num_sets[i]) == 3){
				block3 ++;
				if(get<2>(num_sets[i]) != 0) goal_3 ++;
			}
			else if(get<3>(num_sets[i]) == 4){
				block4 ++;
				if(get<2>(num_sets[i]) != 0) goal_4 ++;
			}
			else if(get<3>(num_sets[i]) > 4){
				block_enough ++;
				if(get<2>(num_sets[i]) != 0) goal_enough ++;
			}
		}
		/*
		int num_piece = r*c - empty;
		int small_ones = num_piece - (block_enough*5 + block4*4 + block3*3);
		double points = small_ones * (-1);
		
		double p1, p2, p3, p4, p_enough;
		p1 = 0;
		p2 = 0;
		p3 = 0;
		p4 = 0;
		p_enough = 0;

		p1 = block1;
		p2 = 3 * block2;
		p3 = block3 * 5;
		p3 += goal_3 * 0.001;
		p4 = block4 * 7;
		p4 += goal_4 * 0.01;
		p_enough = block_enough * 9;
		p_enough += goal_enough * 0.1;
		
		double points;
		points = p1 + p2 + p3 + p4 + p_enough;
		*/
		//  1000+ ////////////////////////////////////////
		// num of pieces on board
		int num_piece = r*c - empty;
		double points;
		//vector<tuple<int, int, int>>num_sets; // (root, size, goal_cell)
		int largest = get<1>(num_sets[0]);
		int index = 0;
		for(i = 1; i < num_sets.size(); i ++){
		if(get<1>(num_sets[i]) > largest){
		largest = get<1>(num_sets[i]);
		index = i;
		}
		}

		// except the largest block(because it can be easily scored), how many small sets/seperate pieces are there
		// the bigger difference is, the more connected the whole board is
		int difference = num_piece - (num_sets.size() - 1);
		if(get<2>(num_sets[index]) == 1){
		points = difference + largest * 0.1;
		//points = difference * 3 + largest;
		}else{
		points = difference * 0.8 + largest * 0.1;	
		}
		delete pointer;
		return points;
	}

	void Superball::next(){
		vector<tuple<int, int>> scorevec;

		//vector<string> scorevec;
		scorevec = analyze(); // goal cell on board checked, djs updated on goal cells
		vector<int> tempvec;
		double original_points = board_evaluation(board); //whole board checked
		//cout << "mother board score is " << original_points << endl;
		//int i, j, temp, points, row, col;
		//string instruction, command;
		//map<int, string> point_system; // <points, instruction>
		//map<int, string>::reverse_iterator pit;
		int x, y;

		if(scorevec.size() == 0) { // only swap
			swap(original_points);
			return;
		}
		else{ // can score	
			if(empty < 10) {
				score(scorevec);
				return;
			}
			else{
				//tuple<int, int> about_to_score(vector<string>& scorevec);
				//tuple<int, int> swap_bring_benefit(tuple<int, int>index_size);
				//tuple<int, int> benefit_check(vector<int>&temp_board, tuple<int, int>index_size);

				// pass in ready_to_scoring (index, size)
				// return (x, y) if there is a better swap before scoring or (-1, -1)
				//tuple<int, int> Superball::swap_bring_benefit(tuple<int, int>index_size){
				x = get<0>(swap_bring_benefit(about_to_score(scorevec)));
				y = get<1>(swap_bring_benefit(about_to_score(scorevec)));
				if(x != -1){	
					printf("SWAP %i %i %i %i\n", x/c, x%c, y/c, y%c);
					return;
				}else{
					score(scorevec);
					return;
				}
			}
			}
		}

		int main(int argc, char **argv)
		{
			Superball *s;
			//int i, j;
			//int ngoal, tgoal;

			s = new Superball(argc, argv);
			s -> next();
			// cout << s -> distance(12, 23) << endl;
			//s -> board_evaluation(s -> board);
			/*
			   vector<string> stringvec;
			   stringvec = s -> analyze();
			   for(int i = 0; i < stringvec.size(); i ++){
			   cout << stringvec[i] << endl;
			   }	
			 */
			exit(0);
		}
