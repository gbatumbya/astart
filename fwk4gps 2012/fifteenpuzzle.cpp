#include "fifteenpuzzle.h"
#include <cstdlib>
void flip(int& a,int& b){
	int tmp=a;
	a=b;
	b=tmp;
}
MoveList::MoveList(){
	move_=new int[10000];
	maxMoves_=10000;
	numMoves_=0;
}
void MoveList::add(int move){
	if(move>=0 && move<=3){
		int* newarray=new int[maxMoves_+500];
		if(numMoves_ == maxMoves_){
			for(int i=0;i<numMoves_;i++){
				newarray[i]=move_[i];
			}
			maxMoves_+=500;
			delete [] move_;
			move_=newarray;
		}
		move_[numMoves_]=move;
		numMoves_++;
	}
}

void MoveList::clear(){
	numMoves_=0;
}


bool moveUp(int board[][4], Position pieces[]){
	bool rc=false;
	if(pieces[15].row_ != 0){
		rc=true;
		int other=board[pieces[15].row_-1][pieces[15].col_];
		flip(board[pieces[15].row_][pieces[15].col_],board[pieces[other].row_][pieces[other].col_]);
		flip(pieces[15].row_,pieces[other].row_);
	}
	return rc;
}
bool moveDown(int board[][4], Position pieces[]){
	bool rc=false;
	if(pieces[15].row_ != 3){
		rc=true;
		int other=board[pieces[15].row_+1][pieces[15].col_];
		flip(board[pieces[15].row_][pieces[15].col_],board[pieces[other].row_][pieces[other].col_]);
		flip(pieces[15].row_,pieces[other].row_);
	}
	return rc;
}
bool moveLeft(int board[][4], Position pieces[]){
		bool rc=false;
	if(pieces[15].col_ != 0){
		rc=true;
		int other=board[pieces[15].row_][pieces[15].col_-1];
		flip(board[pieces[15].row_][pieces[15].col_],board[pieces[other].row_][pieces[other].col_]);
		flip(pieces[15].col_,pieces[other].col_);
	}
	return rc;
}
bool moveRight(int board[][4], Position pieces[]){
	bool rc=false;
	if(pieces[15].col_ != 3){
		rc=true;
		int other=board[pieces[15].row_][pieces[15].col_+1];
		flip(board[pieces[15].row_][pieces[15].col_],board[pieces[other].row_][pieces[other].col_]);
		flip(pieces[15].col_,pieces[other].col_);
	}
	return rc;
}
FifteenPuzzle::FifteenPuzzle(){
	int k=0;
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			pieces_[k].row_=i;
			pieces_[k].col_=j;
			puzzle_[i][j]=k++;
		}
	}
}

bool FifteenPuzzle::isSolved() const{
	int k=0;
	bool isgood=true;
	for(int i=0;isgood&&i<4;i++){
		for(int j=0;isgood&&j<4;j++){
			if(puzzle_[i][j]!=k){
				isgood=false;
			}
		}
	}
	return isgood;
}

void FifteenPuzzle::getPuzzle(int board[][4]){
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			board[i][j]=puzzle_[i][j];
		}
	}
}
bool FifteenPuzzle::moveDown(){
	bool rc= ::moveDown(puzzle_,pieces_);
	return rc;
}
bool FifteenPuzzle::moveUp(){
	bool rc= ::moveUp(puzzle_,pieces_);
	return rc;
}
bool FifteenPuzzle::moveLeft(){
	bool rc=::moveLeft(puzzle_,pieces_);
	return rc;
}
bool FifteenPuzzle::moveRight(){
	bool rc=::moveRight(puzzle_,pieces_);
	return rc;
}

void FifteenPuzzle::scramble(){
	int r;
	for(int i=0;i<3;i++){
		r=rand()%4;
		switch(r){
 			case 0: moveUp(); break;
 			case 1: moveDown(); break;
 			case 2: moveLeft(); break;
 			case 3: moveRight(); break;
		}
	}
}
void FifteenPuzzle::getPositions(Position pos[]){
	for(int i=0;i<16;i++){
		pos[i]=pieces_[i];
	}
}
