
#include "RandSearchStrategy.h"

using namespace std;

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
	int moveType = Tools::GetRandomElementFromVector(availableMoveTypes);

	if (moveType == BIRTH) {
		if (deadCells.size() == 0 || myCells.size() < 2) {
			return Move();
		}

		// randomly choose a target dead cell and two of my own cells to sacrifice
		Coordinate target = Tools::GetRandomElementFromVector(deadCells);
		Coordinate sacrifice1 = Tools::PopRandomElementFromVector(myCells); // pop the element so we don't choose the same element twice
		Coordinate sacrifice2 = Tools::GetRandomElementFromVector(myCells);
		myCells.push_back(sacrifice1);

		return Move(target, sacrifice1, sacrifice2);
	}

	else {
		assert(moveType == KILL);
		Coordinate target = Tools::GetRandomElementFromVector(enemyCells);

		return Move(target);
	}
}

inline int RandSearchStrategy::getMoveScore(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &trialBoard) {
	board.applyMove(move, playerID, nextRoundBoard, trialBoard);
	int moveScore = trialBoard.getPlayerCellCount(playerID) - trialBoard.getPlayerCellCount(enemyID);
	return moveScore;
}

RandSearchStrategy::RandSearchStrategy() {}

Move RandSearchStrategy::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove) {
	long startTime = Tools::get_time();

	Board trialBoard(board.getWidth(), board.getHeight());
	Board *nextRoundBoard = board.getNextRoundBoard();

	Move passMove = Move();
	int passMoveScore = getMoveScore(board, playerID, enemyID, passMove, *nextRoundBoard, trialBoard);

	Move bestMove = passMove;
	int bestScore = passMoveScore;
	
	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, playerID, enemyID);

	int trials = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < timePerMove; currentTime = Tools::get_time()) {
		trials++;
		Move testMove = getRandomMove(board, playerID, enemyID, availableMoveTypes, deadCells, myCells, enemyCells);
		int moveScore = getMoveScore(board, playerID, enemyID, testMove, *nextRoundBoard, trialBoard);
		if (moveScore > bestScore) {
			bestScore = moveScore;
			bestMove = testMove;
		}
	}
	//cerr << "RandSearchStrategy  trials: " << trials << "\n";
	delete nextRoundBoard;
	return bestMove;
};