
#include "RandSearchStrategy.h"

using namespace std;

template <class T>
inline T RandSearchStrategy::PopRandomElementFromVector(vector<T> &input) {
	assert(input.size() > 0);
	T result = GetRandomElementFromVector(input);
	input.pop_back();
	return result;
}

template <class T>
inline T RandSearchStrategy::GetRandomElementFromVector(vector<T> &input) {
	assert(input.size() > 0);
	random_shuffle(input.begin(), input.end());
	return input.back();
}

vector<MoveType> RandSearchStrategy::GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID) {
	vector<MoveType> availableMoves;

	if (board.getPlayerCellCount(playerID) + board.getPlayerCellCount(enemyID) > 0) {
		availableMoves.push_back(KILL);
	}

	if (board.getPlayerCellCount(playerID) > 1) {
		availableMoves.push_back(BIRTH);
	}

	return availableMoves;
}

Move RandSearchStrategy::getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes, 
									   vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells) {
	int moveType = GetRandomElementFromVector(availableMoveTypes);

	if (moveType == BIRTH) {
		if (deadCells.size() == 0 || myCells.size() < 2) {
			return Move();
		}

		// randomly choose a target dead cell and two of my own cells to sacrifice
		Coordinate target = GetRandomElementFromVector(deadCells);
		Coordinate sacrifice1 = PopRandomElementFromVector(myCells); // pop the element so we don't choose the same element twice
		Coordinate sacrifice2 = GetRandomElementFromVector(myCells);
		myCells.push_back(sacrifice1);

		return Move(target, sacrifice1, sacrifice2);
	}

	else {
		assert(moveType == KILL);
		Coordinate target = GetRandomElementFromVector(enemyCells);

		return Move(target);
	}
}

RandSearchStrategy::RandSearchStrategy() {}

Move RandSearchStrategy::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove) {
	long startTime = Tools::get_time();

	Move passMove = Move();
	Board *passMoveResult = board.makeMove(passMove, playerID);
	int passMoveScore = passMoveResult->getPlayerCellCount(playerID) - passMoveResult->getPlayerCellCount(enemyID);
	delete passMoveResult;

	Move bestMove = passMove;
	int bestScore = passMoveScore;
	
	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, playerID, enemyID);

	int trials = 0;
	long currentTime = Tools::get_time();
	while (currentTime - startTime < timePerMove) {
		trials++;
		Move testMove = getRandomMove(board, playerID, enemyID, availableMoveTypes, deadCells, myCells, enemyCells);
		Board *moveResult = board.makeMove(testMove, playerID);
		int moveScore = moveResult->getPlayerCellCount(playerID) - moveResult->getPlayerCellCount(enemyID);
		delete moveResult;
		if (moveScore > bestScore) {
			bestScore = moveScore;
			bestMove = testMove;
		}
		currentTime = Tools::get_time();
	}
	cerr << "bot 0 trials: " << trials << "\n";
	return bestMove;
};