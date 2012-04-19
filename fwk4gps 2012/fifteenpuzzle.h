#ifndef FIFTEENPUZZLE
#define FIFTEENPUZZLE
#define MOVEUP 0
#define MOVEDOWN 1
#define MOVELEFT 2
#define MOVERIGHT 3
struct Position{
	int row_;
	int col_;
};
class MoveList{
	int* move_;
	int numMoves_;
	int maxMoves_;
public:
	MoveList();
	void add(int move);
	void clear();
	const int* moves() const {return move_;}
	int numMoves(){return numMoves_;}
};
bool moveUp(int board[][4], Position pieces[]);
bool moveDown(int board[][4], Position pieces[]);
bool moveLeft(int board[][4], Position pieces[]);
bool moveRight(int board[][4], Position pieces[]);
class FifteenPuzzle{
	int puzzle_[4][4];    //the 15 puzzle has 15 pieces on a 4 by 4 grid, with one piece left open.
	int state_;
	Position pieces_[16];
public:
	FifteenPuzzle();        //initializes the object
	bool isSolved() const;  //returns true if the puzzle is solved, false otherwise
	void scramble();        //function scrambles the pieces of the puzzle
	bool moveUp();          //move blank tile up if possible, returns true on success, false otherwise
	bool moveDown();        //moves blank tile down if possible
	bool moveLeft();        //moves blank tile left if possible
	bool moveRight();        //moves blank tile right if possible
	void getPuzzle(int board[][4]);
    void getPositions(Position pos[]);    
};
#endif