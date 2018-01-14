
#include "AdversarialStrategy.h"

using namespace std;

vector<MoveType> AdversarialStrategy::GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID) {
	vector<MoveType> availableMoves;

	if (board.getPlayerCellCount(playerID) + board.getPlayerCellCount(enemyID) > 0) {
		availableMoves.push_back(KILL);
	}

	if (board.getPlayerCellCount(playerID) > 1) {
		availableMoves.push_back(BIRTH);
	}

	return availableMoves;
}

Move AdversarialStrategy::getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes,
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

inline int AdversarialStrategy::getSimpleMoveScore(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &trialBoard) {
	board.applyMove(move, playerID, nextRoundBoard, trialBoard);
	int moveScore = trialBoard.getPlayerCellCount(playerID) - trialBoard.getPlayerCellCount(enemyID);
	return moveScore;
}

int AdversarialStrategy::getBestMoveScore(Board &board, Player playerID, Player enemyID, int trials) {
	Board trialBoard(board.getWidth(), board.getHeight());
	Board *nextRoundBoard = board.getNextRoundBoard();

	Move passMove = Move();
	int passMoveScore = getSimpleMoveScore(board, playerID, enemyID, passMove, *nextRoundBoard, trialBoard);

	int bestScore = passMoveScore;

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, playerID, enemyID);

	for (int i = 0; i < trials; i++) {
		Move testMove = getRandomMove(board, playerID, enemyID, availableMoveTypes, deadCells, myCells, enemyCells);
		int moveScore = getSimpleMoveScore(board, playerID, enemyID, testMove, *nextRoundBoard, trialBoard);
		if (moveScore > bestScore) {
			bestScore = moveScore;
		}
	}

	delete nextRoundBoard;
	return bestScore;
};

inline int AdversarialStrategy::getMoveScore(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &trialBoard) {
	board.applyMove(move, playerID, nextRoundBoard, trialBoard);
	// the value could be negated later on, so don't return INT_MIN because negating INT_MIN will cause an overflow error
	if (trialBoard.getPlayerCellCount(playerID) == 0) return -INT_MAX;
	else if (trialBoard.getPlayerCellCount(enemyID) == 0) return INT_MAX;
	int moveScore = -getBestMoveScore(trialBoard, enemyID, playerID, adversarialTrials);
	return moveScore;
}

AdversarialStrategy::AdversarialStrategy(int adversarialTrials) {
	this->adversarialTrials = adversarialTrials;
}

Move AdversarialStrategy::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove) {
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
	//cerr << "AdversarialStrategy trials: " << trials << "\n";
	delete nextRoundBoard;
	return bestMove;
};