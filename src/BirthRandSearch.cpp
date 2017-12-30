
#include "BirthRandSearch.h"

using namespace std;

template <class T>
inline T BirthRandSearch::PopRandomElementFromVector(vector<T> &input) {
	assert(input.size() > 0);
	T result = GetRandomElementFromVector(input);
	input.pop_back();
	return result;
}

template <class T>
inline T BirthRandSearch::GetRandomElementFromVector(vector<T> &input) {
	assert(input.size() > 0);
	random_shuffle(input.begin(), input.end());
	return input.back();
}

int BirthRandSearch::getMoveScore(Board &board, Player playerID, Player enemyID, Move &move) {
	Board *moveResult = board.makeMove(move, playerID);
	int moveScore = moveResult->getPlayerCellCount(playerID) - moveResult->getPlayerCellCount(enemyID);
	delete moveResult;
	return moveScore;
}

Move BirthRandSearch::getBestKillMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &enemyCells) {
	Move bestMove = Move();
	int bestMoveScore = getMoveScore(board, playerID, enemyID, bestMove);

	int numEnemyCells = enemyCells.size();
	for (int i = 0; i < numEnemyCells; i++) {
		Coordinate enemyCell = enemyCells[i];
		Move trialMove = Move(enemyCell);
		int trialMoveScore = getMoveScore(board, playerID, enemyID, trialMove);
		if (trialMoveScore > bestMoveScore) {
			bestMoveScore = trialMoveScore;
			bestMove = trialMove;
		}
	}
	return bestMove;
}

//Move BirthRandSearch::getBestBirthMove(Board &board, Player playerID, Player enemyID, 
//									   vector<Coordinate> &deadCells, vector<Coordinate> &myCells, int time) {
//	int startTime = Tools::get_time();
//
//	if (board.getPlayerCellCount(playerID) < 2) {
//		return Move(); // not possible to birth, so just pass
//	}
//
//	Coordinate target = GetRandomElementFromVector(deadCells);
//	Coordinate sacrifice1 = PopRandomElementFromVector(myCells);
//	Coordinate sacrifice2 = PopRandomElementFromVector(myCells);
//	Move trialMove = Move(target, sacrifice1, sacrifice2);
//	int trialScore = getMoveScore(board, playerID, enemyID, trialMove);
//
//	Move bestMove = trialMove;
//	int bestScore = trialScore;
//
//	int myRemainingCells = myCells.size();
//	long currentTime = Tools::get_time();
//
//	int trials = 0;
//	while (currentTime - startTime < time) {
//		Coordinate newTarget = GetRandomElementFromVector(deadCells);
//		trialMove = Move(newTarget, sacrifice1, sacrifice2);
//		trialScore = getMoveScore(board, playerID, enemyID, trialMove);
//		if (trialScore > bestScore) {
//			bestScore = trialScore;
//			bestMove = trialMove;
//			target = newTarget;
//		}
//
//		trials++;
//		if (myRemainingCells > 0) {
//			trials += 2;
//			Coordinate newSacrifice1 = PopRandomElementFromVector(myCells);
//			trialMove = Move(target, newSacrifice1, sacrifice2);
//			trialScore = getMoveScore(board, playerID, enemyID, trialMove);
//			if (trialScore > bestScore) {
//				bestScore = trialScore;
//				bestMove = trialMove;
//				myCells.push_back(sacrifice1);
//				sacrifice1 = newSacrifice1;
//			}
//			else myCells.push_back(newSacrifice1);
//
//			Coordinate newSacrifice2 = PopRandomElementFromVector(myCells);
//			trialMove = Move(target, sacrifice1, newSacrifice2);
//			trialScore = getMoveScore(board, playerID, enemyID, trialMove);
//			if (trialScore > bestScore) {
//				bestScore = trialScore;
//				bestMove = trialMove;
//				myCells.push_back(sacrifice2);
//				sacrifice2 = newSacrifice2;
//			}
//			else myCells.push_back(newSacrifice2);
//		}
//		currentTime = Tools::get_time();
//	}
//	return bestMove;
//}

Move BirthRandSearch::getBestBirthMove(Board &board, Player playerID, Player enemyID, 
									   vector<Coordinate> &deadCells, vector<Coordinate> &myCells, int time) {
	int startTime = Tools::get_time();

	if (board.getPlayerCellCount(playerID) < 2) {
		return Move(); // not possible to birth, so just pass
	}

	Coordinate target = PopRandomElementFromVector(deadCells);
	Coordinate sacrifice1 = PopRandomElementFromVector(myCells);
	Coordinate sacrifice2 = PopRandomElementFromVector(myCells);
	Move trialMove = Move(target, sacrifice1, sacrifice2);
	int trialScore = getMoveScore(board, playerID, enemyID, trialMove);

	Move bestMove = trialMove;
	int bestScore = trialScore;

	long currentTime = Tools::get_time();

	while (deadCells.size() > 0 || myCells.size() > 0) {
		if (deadCells.size() > 0) {
			Coordinate newTarget = PopRandomElementFromVector(deadCells);
			trialMove = Move(newTarget, sacrifice1, sacrifice2);
			trialScore = getMoveScore(board, playerID, enemyID, trialMove);
			if (trialScore > bestScore) {
				bestScore = trialScore;
				bestMove = trialMove;
				target = newTarget;
			}
		}

		if (myCells.size() > 0) {
			Coordinate newSacrifice = PopRandomElementFromVector(myCells);

			Move trialMove1 = Move(target, newSacrifice, sacrifice2);
			int trial1Score = getMoveScore(board, playerID, enemyID, trialMove1);
			Move trialMove2 = Move(target, sacrifice1, newSacrifice);
			int trial2Score = getMoveScore(board, playerID, enemyID, trialMove2);

			if (trial1Score > bestScore || trial2Score > bestScore) {
				if (trial1Score > trial2Score) {
					bestScore = trial1Score;
					bestMove = trialMove1;
					sacrifice1 = newSacrifice;
				}
				else {
					bestScore = trial2Score;
					bestMove = trialMove2;
					sacrifice2 = newSacrifice;
				}
			}
		}
	}
	return bestMove;
}

BirthRandSearch::BirthRandSearch() {}

Move BirthRandSearch::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove) {
	long startTime = Tools::get_time();

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));

	Move bestKillMove = getBestKillMove(board, playerID, enemyID, enemyCells);
	int bestKillMoveScore = getMoveScore(board, playerID, enemyID, bestKillMove);

	int dt = Tools::get_time() - startTime;
	Move bestBirthMove = getBestBirthMove(board, playerID, enemyID, deadCells, myCells, timePerMove - dt);
	int bestBirthMoveScore = getMoveScore(board, playerID, enemyID, bestBirthMove);

	return bestKillMoveScore > bestBirthMoveScore ? bestKillMove : bestBirthMove;
}