#include "RandomStrategy.h"

using namespace std;

template <class T>
T RandomStrategy::RandomElementFromVector(vector<T> &input) {
	assert(input.size() > 0);
	random_shuffle(input.begin(), input.end());
	T temp = input.back();
	input.pop_back();
	return temp;
}

vector<MoveType> RandomStrategy::GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID) {
	vector<MoveType> availableMoves;

	if (board.getPlayerCellCount(playerID) + board.getPlayerCellCount(enemyID) > 0) {
		availableMoves.push_back(KILL);
	}

	if (board.getPlayerCellCount(playerID) > 1) {
		availableMoves.push_back(BIRTH);
	}

	return availableMoves;
}

RandomStrategy::RandomStrategy() {}

Move RandomStrategy::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove) {
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, playerID, enemyID);
	int moveType = RandomElementFromVector(availableMoveTypes);

	if (moveType == BIRTH) {
		// Get all dead cells
		vector<Coordinate> deadCells = board.GetCells('.');
		vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));

		if (deadCells.size() == 0 || myCells.size() < 2) {
			return Move();
		}

		// randomly choose a target dead cell and two of my own cells to sacrifice
		Coordinate target = RandomElementFromVector(deadCells);
		Coordinate sacrifice1 = RandomElementFromVector(myCells);
		Coordinate sacrifice2 = RandomElementFromVector(myCells);

		return Move(target, sacrifice1, sacrifice2);
	}

	else {
		assert(moveType == KILL);
		vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
		Coordinate target = RandomElementFromVector(enemyCells);

		return Move(target);
	}
}