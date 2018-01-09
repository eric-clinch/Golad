
#include "BirthRandSearch.h"

using namespace std;

struct BirthRandSearch::MoveAndScore {
	Move move;
	double score;

	MoveAndScore(Move m, double s) {
		move = m;
		score = s;
	}
};

template <class T> inline T BirthRandSearch::PopRandomElementFromVector(vector<T> &input) {
	assert(input.size() > 0);
	T result = GetRandomElementFromVector(input);
	input.pop_back();
	return result;
}

template <class T> inline T BirthRandSearch::GetRandomElementFromVector(vector<T> &input) {
	assert(input.size() > 0);
	random_shuffle(input.begin(), input.end());
	return input.back();
}

vector<MoveType> BirthRandSearch::GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID) {
	vector<MoveType> availableMoves;

	if (board.getPlayerCellCount(playerID) + board.getPlayerCellCount(enemyID) > 0) {
		availableMoves.push_back(KILL);
	}

	if (board.getPlayerCellCount(playerID) > 1) {
		availableMoves.push_back(BIRTH);
	}

	return availableMoves;
}

Move BirthRandSearch::getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes,
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

double BirthRandSearch::getBestMoveScore(Board &board, Player playerID, Player enemyID, int trials, int depth) {
	Board trialBoard(board.getWidth(), board.getHeight());
	Board *nextRoundBoard = board.getNextRoundBoard();

	Move passMove = Move();
	double passMoveScore = getMoveScore(board, playerID, enemyID, passMove, *nextRoundBoard, trialBoard, depth + 1);

	double bestScore = passMoveScore;

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, playerID, enemyID);

	for (int i = 0; i < trials; i++) {
		Move testMove = getRandomMove(board, playerID, enemyID, availableMoveTypes, deadCells, myCells, enemyCells);
		double moveScore = getMoveScore(board, playerID, enemyID, testMove, *nextRoundBoard, trialBoard, depth + 1);
		if (moveScore > bestScore) {
			bestScore = moveScore;
		}
	}

	delete nextRoundBoard;
	return bestScore;
};

inline double BirthRandSearch::getMoveScore(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &trialBoard, int depth) {
	board.applyMove(move, playerID, nextRoundBoard, trialBoard);

	if (depth == maxDepth) {
		double moveScore = (double)trialBoard.getPlayerCellCount(playerID) / trialBoard.getPlayerCellCount(enemyID);
		return moveScore;
	}
	else {
		if (trialBoard.getPlayerCellCount(playerID) == 0) return 0;
		else if (trialBoard.getPlayerCellCount(enemyID) == 0) return INT_MAX;
		double moveScore = -getBestMoveScore(trialBoard, enemyID, playerID, adversarialTrials[depth], depth);
		return moveScore;
	}
}

BirthRandSearch::MoveAndScore BirthRandSearch::getBestKillMove(Board &board, Player playerID, Player enemyID,
	vector<Coordinate> &enemyCells, Board &nextRoundBoard) {

	Board emptyBoard(board.getWidth(), board.getHeight());

	Move bestMove = Move();
	double bestMoveScore = getMoveScore(board, playerID, enemyID, bestMove, nextRoundBoard, emptyBoard, 0);

	int numEnemyCells = enemyCells.size();
	for (int i = 0; i < numEnemyCells; i++) {
		Coordinate enemyCell = enemyCells[i];
		Move trialMove = Move(enemyCell);
		double trialMoveScore = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0);
		if (trialMoveScore > bestMoveScore) {
			bestMoveScore = trialMoveScore;
			bestMove = trialMove;
		}
	}

	return MoveAndScore(bestMove, bestMoveScore);
}

BirthRandSearch::MoveAndScore BirthRandSearch::getBestBirthMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &deadCells,
	vector<Coordinate> &myCells, Board &nextRoundBoard, int time) {
	int startTime = Tools::get_time();

	Board emptyBoard(board.getWidth(), board.getHeight());

	if (board.getPlayerCellCount(playerID) < 2) {
		// not possible to birth, so just pass
		Move passMove = Move();
		double passScore = getMoveScore(board, playerID, enemyID, passMove, nextRoundBoard, emptyBoard, 0);
		return MoveAndScore(passMove, passScore);
	}

	Coordinate target = GetRandomElementFromVector(deadCells);
	Coordinate sacrifice1 = PopRandomElementFromVector(myCells);
	Coordinate sacrifice2 = PopRandomElementFromVector(myCells);
	Move bestMove = Move(target, sacrifice1, sacrifice2);
	double bestScore = getMoveScore(board, playerID, enemyID, bestMove, nextRoundBoard, emptyBoard, 0);

	int myRemainingCells = myCells.size();
	int trials = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < time; currentTime = Tools::get_time()) {
		Coordinate newTarget = GetRandomElementFromVector(deadCells);
		Move trialMove = Move(newTarget, sacrifice1, sacrifice2);
		double trialScore = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0);
		if (trialScore > bestScore) {
			bestScore = trialScore;
			bestMove = trialMove;
			target = newTarget;
		}

		trials++;
		if (myRemainingCells > 0) {
			trials += 2;
			Coordinate newSacrifice1 = PopRandomElementFromVector(myCells);
			trialMove = Move(target, newSacrifice1, sacrifice2);
			trialScore = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0);
			if (trialScore > bestScore) {
				bestScore = trialScore;
				bestMove = trialMove;
				myCells.push_back(sacrifice1);
				sacrifice1 = newSacrifice1;
			}
			else myCells.push_back(newSacrifice1);

			Coordinate newSacrifice2 = PopRandomElementFromVector(myCells);
			trialMove = Move(target, sacrifice1, newSacrifice2);
			trialScore = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0);
			if (trialScore > bestScore) {
				bestScore = trialScore;
				bestMove = trialMove;
				myCells.push_back(sacrifice2);
				sacrifice2 = newSacrifice2;
			}
			else myCells.push_back(newSacrifice2);
		}
		currentTime = Tools::get_time();
	}
	//cerr << "BirthRandSearch trials: " << trials << "\n";
	return MoveAndScore(bestMove, bestScore);
}

//BirthRandSearch::MoveAndScore BirthRandSearch::getBestBirthMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &deadCells, 
//																  vector<Coordinate> &myCells, Board &nextRoundBoard, int time) {
//	int startTime = Tools::get_time();
//
//	Board emptyBoard(board.getWidth(), board.getHeight());
//	
//	if (board.getPlayerCellCount(playerID) < 2) {
//		// not possible to birth, so just pass
//		Move passMove = Move();
//		double passScore = getMoveScore(board, playerID, enemyID, passMove, nextRoundBoard, emptyBoard);
//		return MoveAndScore(passMove, passScore);
//	}
//
//	Coordinate target = PopRandomElementFromVector(deadCells);
//	Coordinate sacrifice1 = PopRandomElementFromVector(myCells);
//	Coordinate sacrifice2 = PopRandomElementFromVector(myCells);
//	Move trialMove = Move(target, sacrifice1, sacrifice2);
//	int trialScore = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard);
//
//	Move bestMove = trialMove;
//	int bestScore = trialScore;
//
//	while (deadCells.size() > 0 || myCells.size() > 0) {
//		if (deadCells.size() > 0) {
//			Coordinate newTarget = PopRandomElementFromVector(deadCells);
//			trialMove = Move(newTarget, sacrifice1, sacrifice2);
//			trialScore = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard);
//			if (trialScore > bestScore) {
//				bestScore = trialScore;
//				bestMove = trialMove;
//				target = newTarget;
//			}
//		}
//
//		if (myCells.size() > 0) {
//			Coordinate newSacrifice = PopRandomElementFromVector(myCells);
//
//			Move trialMove1 = Move(target, newSacrifice, sacrifice2);
//			int trial1Score = getMoveScore(board, playerID, enemyID, trialMove1, nextRoundBoard, emptyBoard);
//			Move trialMove2 = Move(target, sacrifice1, newSacrifice);
//			int trial2Score = getMoveScore(board, playerID, enemyID, trialMove2, nextRoundBoard, emptyBoard);
//
//			if (trial1Score > bestScore || trial2Score > bestScore) {
//				if (trial1Score > trial2Score) {
//					bestScore = trial1Score;
//					bestMove = trialMove1;
//					sacrifice1 = newSacrifice;
//				}
//				else {
//					bestScore = trial2Score;
//					bestMove = trialMove2;
//					sacrifice2 = newSacrifice;
//				}
//			}
//		}
//	}
//	return MoveAndScore(bestMove, bestScore);
//}

//BirthRandSearch::MoveAndScore BirthRandSearch::getBestBirthMove(Board &board, Player playerID, Player enemyID,
//	vector<Coordinate> &deadCells, vector<Coordinate> &myCells, Board &nextRoundBoard, int time) {
//	int startTime = Tools::get_time();
//
//	Board emptyBoard(board.getWidth(), board.getHeight());
//	Move bestMove = Move();
//	int bestScore = getMoveScore(board, playerID, enemyID, bestMove, nextRoundBoard, emptyBoard);
//
//	if (board.getPlayerCellCount(playerID) < 2) return MoveAndScore(bestMove, bestScore); // not possible to birth, so just pass
//
//	int trials = 0;
//	for (long currentTime = Tools::get_time(); currentTime - startTime < time; currentTime = Tools::get_time()) {
//		trials++;
//		Coordinate target = GetRandomElementFromVector(deadCells);
//		Coordinate sacrifice1 = PopRandomElementFromVector(myCells);
//		Coordinate sacrifice2 = GetRandomElementFromVector(myCells);
//		myCells.push_back(sacrifice1);
//		Move trialMove = Move(target, sacrifice1, sacrifice2);
//		int trialScore = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard);
//
//		if (trialScore > bestScore) {
//			bestScore = trialScore;
//			bestMove = trialMove;
//		}
//	}
//	//cerr << "BirthRandSearch trials: " << trials << "\n";
//	return MoveAndScore(bestMove, bestScore);
//}

BirthRandSearch::BirthRandSearch(int maxDepth, int* adversarialTrials) {
	this->adversarialTrials = adversarialTrials;
	this->maxDepth = maxDepth;
}

Move BirthRandSearch::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove) {
	long startTime = Tools::get_time();

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
	Board *nextRoundBoard = board.getNextRoundBoard();

	Board emptyBoard(board.getWidth(), board.getHeight());

	MoveAndScore bestKillMove = getBestKillMove(board, playerID, enemyID, enemyCells, *nextRoundBoard);

	int dt = Tools::get_time() - startTime;
	MoveAndScore bestBirthMove = getBestBirthMove(board, playerID, enemyID, deadCells, myCells, *nextRoundBoard, timePerMove - dt);

	delete nextRoundBoard;
	return bestKillMove.score > bestBirthMove.score ? bestKillMove.move : bestBirthMove.move;
}