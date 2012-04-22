/* Design Implementation - Application Layer
 *
 * Design.cpp
 * fwk4gps version 3.0
 * gam666/dps901/gam670/dps905
 * January 14 2012
 * copyright (c) 2012 Chris Szalwinski 
 * distributed under TPL - see ../Licenses.txt
 */

#include "Design.h"          // for the Design class definition
#include "iText.h"           // for the Text Interface
#include "iHUD.h"            // for the HUD Interface
#include "iSound.h"          // for the Sound Interface
#include "iLight.h"          // for the Light Interface
#include "iObject.h"         // for the Object Interface
#include "iTexture.h"        // for the Texture Interface
#include "Camera.h"          // for the Camera Interface
#include "iGraphic.h"        // for the Graphic Interface
#include "iUtilities.h"      // for strcpy()
#include "iAPIBase.h"
#include "MathDefinitions.h" // for MODEL_Z_AXIS
#include "ModellingLayer.h"  // for MOUSE_BUTTON_SCALE, ROLL_SPEED
#include "Common_Symbols.h"  // for Action and Sound enumerations
#include "Physics.h"
#include "CSphere.h"
#include "iSimpleCollisionSpace.h"
#include "minHeap.h"
#include "llist.h"
#include "adjacencylist.h"
#include "searchdata.h"

#include <set>
#include <vector>
#include <algorithm>
#include <math.h>
#include <Windows.h>
#include <WinBase.h>
#include <sstream>

const wchar_t* orient(wchar_t*, const iFrame*, char, unsigned = 1u);
const wchar_t* position(wchar_t*, const iFrame*, char = ' ', unsigned = 1u);
const wchar_t* onOff(wchar_t*, const iSwitch*);
//const Colour red(1,0,0);
//-------------------------------- Design -------------------------------------
//
// The Design class implements the game design within the Modelling Layer
//
// constructor initializes the engine and the instance pointers
//
Design::Design(void* h, int s) : Coordinator(h, s){


}

// initialize initializes the general display design coordinator, creates the 
// primitive sets, textures, objects, lights, sounds, cameras, and text items
//
void Design::initialize() {

    // general parameters
    //
	int board[4][4];
	Position pos[16];
    wchar_t fname[20]=L"boxA.bmp";


	Reflectivity redish = Reflectivity(red);
	Reflectivity greenish = Reflectivity(green);
	Reflectivity bluish = Reflectivity(blue);
	Reflectivity whitish = Reflectivity(white);
	iGraphic* box;
    setProjection(0.9f, 1.0f, 1000.0f);
    setAmbientLight(0.9f, 0.9f, 0.9f);

    iCamera* camera = CreateCamera();
    camera->translate(0,150,0);
    camera->setRadius(17.8f);
	camera->rotatex(3.1459/2.0f);
	puzzle_.scramble();
	puzzle_.getPuzzle(board);


    int i;
    for(i=0;i<4;i++){
		for(int j=0;j<4;j++){
			if(board[i][j]!=15){
				 fname[3]=L'A'+board[i][j];
				 iTexture* boxTexture=CreateTexture(fname,TEX_MIN_ANISOTROPIC | \
							 TEX_MAG_ANISOTROPIC);
				 box  = CreateBox(-5, -1, -5 * MODEL_Z_AXIS,  5, 1, 5 * MODEL_Z_AXIS);
				 (pieces_[board[i][j]] = CreateObject(box,&redish))->translate(-22+j*11,0,22-(i*11)*MODEL_Z_AXIS);
				 pieces_[board[i][j]]->rotatex(3.14159f);
				 pieces_[board[i][j]]->attach(boxTexture);
			}
			else{			  
			  highlighterRow_=i;
			  highlighterCol_=j;
			}
		}
    }

	searchroutine_=ASTAR;


	hud = CreateHUD(0.72f, 0.01f, 0.27f, 0.99f, CreateTexture(HUD_IMAGE));


	velocitytxt_=CreateText(Rectf(0.05f,0.27f,0.95f,0.37f),hud,L"",TEXT_HEIGHT,TEXT_TYPEFACE,TEXT_LEFT);
	deltatxt_=CreateText(Rectf(0.05f,0.17f,0.95f,0.27f),hud,L"",TEXT_HEIGHT,TEXT_TYPEFACE,TEXT_LEFT);
	positiontxt_=CreateText(Rectf(0.05f,0.38f,0.95f,0.48f),hud,L"",TEXT_HEIGHT,TEXT_TYPEFACE,TEXT_LEFT);

	ismoving_=false;
	lasttextupdate=now;
	lastFireTime_=0;
}

// update updates the position and orientation of each object according to the 
// actions initiated by the user
//
void Design::update() {
	int board[4][4];
	Position pos[16];
	int delta = now -lastUpdate;
	int movedPiecePos=-1;
	lastUpdate = now;
	puzzle_.getPuzzle(board);
	int oldRow=highlighterRow_;
	int oldCol=highlighterCol_;
	if(now - lastFireTime_ > 200){
		if(pressed(UPBOX)){
			lastFireTime_=now;
			if(highlighterRow_!=0){
				movedPiecePos=board[highlighterRow_-1][highlighterCol_];
				puzzle_.moveUp();
				highlighterRow_--;
			}
		}
		if(pressed(DOWNBOX)){
			lastFireTime_=now;
			if(highlighterRow_!=3){
				movedPiecePos=board[highlighterRow_+1][highlighterCol_];
				puzzle_.moveDown();
				highlighterRow_++;
			}
		}
		if(pressed(RIGHTBOX)){
			lastFireTime_=now;
			if(highlighterCol_!=3){
				movedPiecePos=board[highlighterRow_][highlighterCol_+1];
				puzzle_.moveRight();
				highlighterCol_++;
			}
		}
		if(pressed(LEFTBOX)){
			lastFireTime_=now;
			if(highlighterCol_!=0){
				movedPiecePos=board[highlighterRow_][highlighterCol_-1];
				highlighterCol_--;
				puzzle_.moveLeft();
			}
		}
		if(pressed(ASTAR_KEY)){
			searchroutine_=ASTAR;
		}
		if(pressed(IDASTAR_KEY)){
			searchroutine_=IDASTAR;
 		}
		else
		if(ismoving_ && movelist_.moves()[curr_]==MOVEUP){
			curr_++;
			lastFireTime_=now;
			if(highlighterRow_!=0){
				movedPiecePos=board[highlighterRow_-1][highlighterCol_];
				puzzle_.moveUp();
				highlighterRow_--;
			}
		}
		else if(ismoving_ && movelist_.moves()[curr_]==MOVEDOWN){
			curr_++;
			lastFireTime_=now;
			if(highlighterRow_!=3){
				movedPiecePos=board[highlighterRow_+1][highlighterCol_];
				puzzle_.moveDown();
				highlighterRow_++;
			}
		}
		else if(ismoving_ && movelist_.moves()[curr_]==MOVERIGHT){
			curr_++;
			lastFireTime_=now;
			if(highlighterCol_!=3){
				movedPiecePos=board[highlighterRow_][highlighterCol_+1];
				puzzle_.moveRight();
				highlighterCol_++;
			}
		}
		else if(ismoving_ && movelist_.moves()[curr_]==MOVELEFT){
			curr_++;
			lastFireTime_=now;
			if(highlighterCol_!=0){
				movedPiecePos=board[highlighterRow_][highlighterCol_-1];
				highlighterCol_--;
				puzzle_.moveLeft();
			}
		}

		if(movedPiecePos!=-1){
			Vector 	hpos=pieces_[movedPiecePos]->position();
			pieces_[movedPiecePos]->translate(-hpos.x,-hpos.y,-hpos.z);
			pieces_[movedPiecePos]->translate(-22+oldCol*11,0,22-(oldRow*11)*MODEL_Z_AXIS);
			if(curr_==movelist_.numMoves()){
				curr_=0;
				ismoving_=false;
			}
		}
		else if(pressed(STARTANIM)){
			lastFireTime_=now;
			if(!ismoving_){
				puzzle_.getPositions(pos);
				puzzle_.getPuzzle(board);
				bool searchsuccess=false;        //stores return value of path finding function
				int searchstart=now;
				switch (searchroutine_){
					case ASTAR: searchsuccess=aStar(board,pos); break;
					case IDASTAR: searchsuccess=iDAStar(board,pos); break;
				}/*switch*/
				int searchtime=now-searchstart;
				if(searchsuccess){
					ismoving_=true;
					curr_=0;
				}
			}//!ismoving_
		}
	}
    lastUpdate = now;
 
}




/*For your assignment, code either the aStar() or iDAStar function.  This function is passed two
  representations of the 16 puzzle. each piece has an id from 0 to 15.
  0 to 14 are the pieces.  The pieces fit going across left to right and row by row.
  and piece 15 is the "blank"
  A solved puzzle has this config:

  0  1  2  3
  4  5  6  7
  8  9  10 11
  12 13 14

  board is a two dimensional array representing the pieces as they are shown visually.
        board[row][col] contains the piece number at that position.  Thus if
		board[0][0] == 2, the third piece (visually C) will be in top left corner.

  pos is a different representation of the board that lets you see the position of
	  a piece immediately.  pos[0] holds the location in the board of piece 0 (visually A)
      pos[15] is location of blank.

  To alter the board(make a move) use the move*(int [][4], Position); functions.
  These functions move the blank spots in the direction specified if possible.
  The functions will ensure that pos and board stay synced up.

  Place the solution that you find into the movelist_ variable.
  The movelist_ is essentially an array that only contains 0,1,2,3 (defined as MOVEUP,
  MOVEDOWN, MOVELEFT and MOVERIGHT respectively).  If you go through the movelist_, 
  and move the blank according to the values in the list, the puzzle will be solved.

 */
class Board
{
   int
      _board[4][4],        // the game board
      _move,                // move to get here from parent
      _cost;                // cost for this board
   float _heuristic;       // heuristic cost to goal
   unsigned __int64 _key;   // unique identifier for this board
   Board* _parent;         // parent of this board

public:
   Board()
   {
      ZeroMemory(_board, 16 * sizeof(int));
      _move = -1;
      _key = 0;
      _cost = -1;
      _heuristic = 0;
      _parent = 0;
   }
   
   Board(int board[][4], Position pos[16], Board* parent = nullptr) :
      _move(-1), _key(0) , _heuristic(0), _cost(0), _parent(parent)
   { 
      int temp;
      memcpy(_board, board, 16 * sizeof(int));

      for (int i = 0; i < 16; ++i)
      {  
         temp = _board[i/4][i%4];

         if (temp > 9)
         {
            if(temp != 10)
            {
               _key = _key * 10 + 1;
               _key = _key * 10 + (long)(temp % 10);
            }
         }
         else
         {
            _key = _key * 10 + (long)temp;
         }

         // manhattan distance to correct position
         if (i/4 - pos[i].row_ || i%4 - pos[i].col_)
         {
            _heuristic += abs(i/4 - pos[i].row_) + abs(i%4 - pos[i].col_);
            //_heuristic += abs(_pos[15].row_ - i/4) + abs(_pos[15].col_ - i%4);
         }
      }

#ifdef LOG_BOARD
      std::wstringstream ss;
      ss<< "heuristics: " << _heuristic << L", key: " << key << std::endl;
      OutputDebugStringW(ss.str().c_str());
#endif
      if (_heuristic == LONG_MAX)
      {
         _heuristic = LONG_MAX;
      }
   }

   ~Board ()
   {
      if (_parent)
         delete _parent;
   }

   void getPosition(Position pos[16]) const  
   {
      for(int i = 0; i < 4; ++i)
         for(int k = 0; k < 4; ++k)
         {
            pos[_board[i][k]].row_ = i;
            pos[_board[i][k]].col_ = k;
         }
   }

   void setMove(int move)           { _move = move; }

   int getMove() const              { return _move; }

   unsigned __int64 getKey() const  { return _key; }

   Board* getParent() const         { return _parent; }

   void setParent(Board *parent)    { _parent = parent; }

   int getCost() const              { return _cost; }

   void setCost(int cost)           { _cost = cost; }

   float heuristic() const          { return _heuristic; }

   float score() const              { return _heuristic + _cost; }

   void getBoard(int board[][4]) const       { memcpy(board, _board, 16 * sizeof(int)); }

   friend bool operator<(const Board& a, const Board& b)   { return a.score() < b.score(); }

   bool operator<=(const Board& b)  { return score() <= b.score(); }

   bool operator>(const Board& b)   { return score() > b.score(); }

   bool operator>=(const Board& b)  { return score() >= b.score(); }

   bool operator==(const Board& b)  { return _key == b.getKey(); }
};

void getMoves(Board* board, MoveList moveList)
{
#define LOG_MOVES
#ifdef LOG_MOVES
   OutputDebugStringW(L"\n Solution: ");
#endif

   std::vector<int> moves;
   while(board->getParent())
   {
      moves.push_back(board->getMove());
      board = board->getParent();
#ifdef LOG_MOVES
      switch(moves.back())
      {
         case MOVEUP: OutputDebugStringW(L"Up, "); break;
         case MOVEDOWN: OutputDebugStringW(L"Down, "); break;
         case MOVELEFT: OutputDebugStringW(L"Left, "); break;
         case MOVERIGHT: OutputDebugStringW(L"Right, "); break;
      }
#endif
   }

   std::vector<int>::reverse_iterator itr = moves.rbegin();

   while(itr != moves.rend())
      moveList.add(*itr++);
}

bool Design::aStar(int board[4][4],Position pos[])
{
   std::set<unsigned __int64> closed;
   std::set<unsigned __int64> openList;
   MinHeap<Board *> openHeap;
   Board
      *up      = nullptr,
      *down    = nullptr,
      *left    = nullptr,
      *right   = nullptr,
      *current = nullptr;
   int
      cost     =-1,
      move     = -1;
   
   openHeap.insert(new Board(board, pos));

for (int i = 0; i < 100000000; i++)
   if(!openHeap.isempty())
   {
#ifdef LOG_ASTAR
      std::wstringstream ss;
      ss << L"open: ";
      std::set<unsigned __int64>::iterator itr = openList.begin();

      while(itr != openList.end())
         ss<<*itr++<<L",";
            
      ss<< L", closed: ";
      itr = closed.begin();

      while(itr != closed.end())
         ss<<*itr++<<L",";

      ss<< std::endl;
      OutputDebugStringW(ss.str().c_str());
#endif
      current = openHeap.remove();
      current->getBoard(board);
      current->getPosition(pos);

      closed.insert(current->getKey());
      openList.erase(current->getKey());

      if (!current->heuristic())
         break;

      cost = current->getCost() + 1;

      // up
      if(current->getMove() != MOVEDOWN && moveUp(board,pos))
      {
         up = new Board(board, pos);
         moveDown(board,pos);

         if (closed.find(up->getKey()) == closed.end() || openList.find(up->getKey()) == openList.end())
         {
            up->setParent(current);
            up->setMove(MOVEUP);
            up->setCost(cost);
            openHeap.insert(up);
            openList.insert(up->getKey());
         }
      }
         
      // down
      if(current->getMove() != MOVEUP && moveDown(board,pos))
      {
         down = new Board(board, pos);
         moveUp(board,pos);

         if (closed.find(down->getKey()) == closed.end() || openList.find(down->getKey()) == openList.end())
         {
            down->setParent(current);
            down->setMove(MOVEDOWN);
            down->setCost(cost);
            openHeap.insert(down);
            openList.insert(down->getKey());
         }
      }
         
      // left
      if(current->getMove() != MOVERIGHT && moveLeft(board,pos))
      {
         left = new Board(board, pos);
         moveRight(board,pos);

         if (closed.find(left->getKey()) == closed.end() || openList.find(left->getKey()) == openList.end())
         {
            left->setParent(current);
            left->setMove(MOVELEFT);
            left->setCost(cost);
            openHeap.insert(left);
            openList.insert(left->getKey());
         }
      }

      // right
      if(current->getMove() != MOVELEFT && moveRight(board,pos))
      {
         right = new Board(board, pos);
         moveLeft(board,pos);

         if (closed.find(right->getKey()) == closed.end() || openList.find(right->getKey()) == openList.end())
         {
            right->setParent(current);
            right->setMove(MOVERIGHT);
            right->setCost(cost);
            openHeap.insert(right);
            openList.insert(right->getKey());
         }
      }
   }

   getMoves(current, movelist_);

   return true;
}

bool iDAStar(Board* current, int cost, int& threshold, Board*& best)
{
   if (!current->heuristic())
   {
      best = current;

      return true;
   }

   if (current->score() > threshold)
   {
      if (best->score() == -1 || current->score() < best->score())
         best = current;

      return false;
   }

   bool solved = false;
   int board[4][4];
   Position pos[16];
   Board
      *up      = nullptr,
      *down    = nullptr,
      *left    = nullptr,
      *right   = nullptr;

   current->getBoard(board);
   current->getPosition(pos);
   cost++;

   // up
   if(current->getMove() != MOVEDOWN && moveUp(board,pos))
   {
      up = new Board(board, pos);
      moveDown(board, pos);

      up->setParent(current);
      up->setCost(cost);
      up->setMove(MOVEUP);

      solved = iDAStar(up, cost, threshold, best);

      if (!solved && best != up)
      {
         up->setParent(nullptr);
         delete up;
         up = nullptr;
      }
   }
         
   // down
   if(!solved && current->getMove() != MOVEUP && moveDown(board,pos))
   {
      down = new Board(board, pos);
      moveUp(board ,pos);

      down->setParent(current);
      down->setCost(cost);
      down->setMove(MOVEDOWN);

      solved = iDAStar(down, cost, threshold, best);

      if (!solved && best != down)
      {
         down->setParent(nullptr);
         delete down;
         down = nullptr;
      }
   }
         
   // left
   if(!solved && current->getMove() != MOVERIGHT && moveLeft(board,pos))
   {
      left = new Board(board, pos);
      moveRight(board, pos);

      left->setParent(current);
      left->setCost(cost);
      left->setMove(MOVELEFT);

      solved = iDAStar(left, cost, threshold, best);

      if (!solved && best != left)
      {
         left->setParent(nullptr);
         delete left;
         left = nullptr;
      }
   }

   // right
   if(!solved && current->getMove() != MOVELEFT && moveRight(board,pos))
   {
      right = new Board(board, pos);
      moveLeft(board, pos);

      right->setParent(current);
      right->setCost(cost);
      right->setMove(MOVERIGHT);

      solved = iDAStar(right, cost, threshold, best);

      if (!solved && best != right)
      {
         right->setParent(nullptr);
         delete right;
         right = nullptr;
      }
   }

   return solved;
}

/*For your assignment, code this function*/
bool Design::iDAStar(int board[][4],Position pos[]){
   Board* start = new Board(board, pos);
   Board* best = new Board();
   int heuristic = start->heuristic();

   while(!::iDAStar(start, 0, heuristic, best))
      heuristic += best->score();

   getMoves(best, movelist_);

   return true;
}
