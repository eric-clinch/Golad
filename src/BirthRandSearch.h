
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define NDEBUG
#endif

#ifndef BirthRandSearch_h
#define BirthRandSearch_h

#include <assert.h>
#include <math.h>
#include <pthread.h>
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"
#include "Tools.h"
#include "RandomVector.h"
#include "LinkedList.h"

class BirthRandSearch : public Strategy
{
private:
	static double max_score;
	int maxDepth;
	// len(adversarial) needs to be equal to maxDepth
	int *adversarialTrials;

	// stores a move and its assigned score from the minimax algorithm
	struct MoveAndScore;
	// stores data that is passed around the minimax algorithm, like the current player and enemy IDs, killer moves, and boards to be used
	struct Data;
	// stores arguments for the approximateBestMove method, so that the method can be called with a single argument and thus can be
	// called in parallel using pthreads
	struct ABMArgs;

	static inline Move getRandomBirth(vector<Coordinate> &deadCells, vector<Coordinate> &myCells);
	static Move getRandomMove(vector<MoveType> &availableMoveTypes, vector<Coordinate> &deadCells,
		vector<Coordinate> &myCells, vector<Coordinate> &enemyCells);
	static vector<MoveType> GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID);

	static double evaluateBoardMini(Board &board, int depth, Data &data, double alpha, double beta);
	static inline double getMoveScoreMini(Board &board, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth, Data &data, double alpha, double beta);

	static double evaluateBoardMaxi(Board &board, int depth, Data &data, double alpha, double beta);
	static inline double getMoveScoreMaxi(Board &board, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth, Data &data, double alpha, double beta);

	static MoveAndScore getBestKillMove(Board &board, vector<Coordinate> &enemyCells, vector<Coordinate> &myCells, Board &nextRoundBoard, Data &data);
	static MoveAndScore getBestBirthMove(Board &board, vector<Coordinate> &deadCells, vector<Coordinate> &myCells, Board &nextRoundBoard, Data &data, int time);
	static void *approximateBestMove(void *arg);

public:
	BirthRandSearch(int maxDepth, int* adversarialTrials);
	~BirthRandSearch();
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round);
};

#endif