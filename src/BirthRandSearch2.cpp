
#include "BirthRandSearch2.h"

using namespace std;

double BirthRandSearch2::max_score = 288.0;

struct BirthRandSearch2::MoveAndScore {
	Move move;
	double score;

	MoveAndScore(Move m, double s) {
		move = m;
		score = s;
	}
};

vector<MoveType> BirthRandSearch2::GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID) {
	vector<MoveType> availableMoves;

	if (board.getPlayerCellCount(enemyID) > 0) {
		availableMoves.push_back(KILL);
	}

	if (board.getPlayerCellCount(playerID) > 1) {
		availableMoves.push_back(BIRTH);
	}

	return availableMoves;
}

Move BirthRandSearch2::getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes,
	vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells) {
	assert(availableMoveTypes.size() > 0);
	int moveType = Tools::GetRandomElementFromVector(availableMoveTypes);

	if (moveType == BIRTH) {
		if (myCells.size() < 2) {
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

BirthRandSearch2::MoveAndScore BirthRandSearch2::evaluateBoardMini(Board &board, Player playerID, Player enemyID, int trials, int depth, double alpha, double beta) {
	assert(alpha < beta);
	Board *trialBoard = trialBoards[depth];
	Board *nextRoundBoard = nextRoundBoards[depth];
	board.setNextRoundBoard(*nextRoundBoard);
	assert(*nextRoundBoard == *board.getNextRoundBoard());

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(enemyID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(playerID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, enemyID, playerID);

	Move bestMove = Move();
	for (int i = 0; i < trials; i++) {
		Move trialMove = getRandomMove(board, enemyID, playerID, availableMoveTypes, deadCells, myCells, enemyCells);
		MoveAndScore moveScore = getMoveScoreMini(board, playerID, enemyID, trialMove, *nextRoundBoard, *trialBoard, depth + 1, alpha, beta);
		if (moveScore.score < beta) {
			beta = moveScore.score;
			bestMove = trialMove;
			if (beta <= alpha) break; // prune
		}
	}

	return MoveAndScore(bestMove, beta);
};

BirthRandSearch2::MoveAndScore BirthRandSearch2::evaluateBoardMaxi(Board &board, Player playerID, Player enemyID, int trials, int depth, double alpha, double beta) {
	assert(alpha < beta);
	Board *trialBoard = trialBoards[depth];
	Board *nextRoundBoard = nextRoundBoards[depth];
	board.setNextRoundBoard(*nextRoundBoard);
	assert(*nextRoundBoard == *board.getNextRoundBoard());

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, playerID, enemyID);

	Move bestMove = Move();
	for (int i = 0; i < trials; i++) {
		Move trialMove = getRandomMove(board, playerID, enemyID, availableMoveTypes, deadCells, myCells, enemyCells);
		MoveAndScore moveScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, *nextRoundBoard, *trialBoard, depth + 1, alpha, beta);
		if (moveScore.score > alpha) {
			alpha = moveScore.score;
			bestMove = trialMove;
			if (beta <= alpha) break; // prune
		}
	}

	return MoveAndScore(bestMove, alpha);
};

inline BirthRandSearch2::MoveAndScore BirthRandSearch2::getMoveScoreMini(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &trialBoard,
												 int depth, double alpha, double beta) {
	assert(alpha < beta);
	board.applyMove(move, enemyID, nextRoundBoard, trialBoard);
	assert(trialBoard == *board.makeMove(move, enemyID));
	if (trialBoard.getPlayerCellCount(playerID) == 0) return MoveAndScore(Move(), -max_score);
	else if (trialBoard.getPlayerCellCount(enemyID) == 0) return MoveAndScore(Move(), max_score);

	if (depth == maxDepth) {
		double score = (double)trialBoard.getPlayerCellCount(playerID) / trialBoard.getPlayerCellCount(enemyID);
		return MoveAndScore(Move(), score);
	}
	else {
		return evaluateBoardMaxi(trialBoard, playerID, enemyID, adversarialTrials[depth], depth, alpha, beta);
	}
}

inline BirthRandSearch2::MoveAndScore BirthRandSearch2::getMoveScoreMaxi(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &trialBoard,
												 int depth, double alpha, double beta) {
	assert(alpha < beta);
	board.applyMove(move, playerID, nextRoundBoard, trialBoard);
	assert(trialBoard == *board.makeMove(move, playerID));
	if (trialBoard.getPlayerCellCount(playerID) == 0) return MoveAndScore(Move(), -max_score);
	else if (trialBoard.getPlayerCellCount(enemyID) == 0) return MoveAndScore(Move(), max_score);
	
	if (depth == maxDepth) {
		double score = (double)trialBoard.getPlayerCellCount(playerID) / trialBoard.getPlayerCellCount(enemyID);
		return MoveAndScore(Move(), score);
	}
	else {
		return evaluateBoardMini(trialBoard, playerID, enemyID, adversarialTrials[depth], depth, alpha, beta);
	}
}

BirthRandSearch2::MoveAndScore BirthRandSearch2::getBestKillMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &enemyCells, 
																 vector<Coordinate> &myCells, Board &nextRoundBoard, double alpha, double beta) {
	assert(alpha < beta);
	Board emptyBoard(board.getWidth(), board.getHeight());

	Move bestMove = Move();
	MoveAndScore moveScore = getMoveScoreMaxi(board, playerID, enemyID, bestMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
	assert(moveScore >= alpha);
	alpha = moveScore.score;
	if (beta <= alpha) return MoveAndScore(bestMove, moveScore.score); // prune

	int numMyCells = myCells.size();
	for (int i = 0; i < numMyCells; i++) {
		Coordinate myCell = myCells[i];
		Move trialMove = Move(myCell);
		MoveAndScore trialMoveScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
		if (trialMoveScore.score > alpha) {
			alpha = trialMoveScore.score;
			bestMove = trialMove;
			if (beta <= alpha) return MoveAndScore(bestMove, alpha);
		}
	}

	int numEnemyCells = enemyCells.size();
	for (int i = 0; i < numEnemyCells; i++) {
		Coordinate enemyCell = enemyCells[i];
		Move trialMove = Move(enemyCell);
		MoveAndScore trialMoveScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
		if (trialMoveScore.score > alpha) {
			alpha = trialMoveScore.score;
			bestMove = trialMove;
			if (beta <= alpha) return MoveAndScore(bestMove, alpha);
		}
	}

	return MoveAndScore(bestMove, alpha);
}

BirthRandSearch2::MoveAndScore BirthRandSearch2::getBestBirthMove(Board &board, Player playerID, Player enemyID, 
	vector<Coordinate> &deadCellsVect, vector<Coordinate> &myCellsVect, Board &nextRoundBoard, double alpha, double beta, int time) {
	assert(alpha < beta);
	long startTime = Tools::get_time();

	Board emptyBoard(board.getWidth(), board.getHeight());

	if (board.getPlayerCellCount(playerID) < 2) {
		// not possible to birth, so just pass
		Move passMove = Move();
		MoveAndScore passScore = getMoveScoreMaxi(board, playerID, enemyID, passMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
		return MoveAndScore(passMove, passScore.score);
	}

	RandomVector<Coordinate> deadCells(deadCellsVect);
	RandomVector<Coordinate> myCells(myCellsVect);

	Coordinate target = deadCells.pop_back();
	Coordinate sacrifice1 = myCells.pop_back();
	Coordinate sacrifice2 = myCells.pop_back();
	Move bestMove = Move(target, sacrifice1, sacrifice2);
	MoveAndScore moveScore = getMoveScoreMaxi(board, playerID, enemyID, bestMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
	assert(moveScore >= alpha);
	alpha = moveScore.score;
	if (beta <= alpha) return MoveAndScore(bestMove, alpha); // prune

	int myRemainingCells = myCells.size();
	int trials = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < time; currentTime = Tools::get_time()) {
		assert(deadCells.size() > 0);
		assert(alpha < beta);
		Coordinate newTarget = deadCells.pop_back();
		Move trialMove = Move(newTarget, sacrifice1, sacrifice2);
		MoveAndScore trialScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
		
		if (trialScore.score > alpha) {
			alpha = trialScore.score;
			bestMove = trialMove;
			if (alpha >= beta) break; // prune
			deadCells.push_back(target);
			target = newTarget;
		}
		else deadCells.push_back(newTarget);

		trials++;

		if (myRemainingCells > 0) {
			assert(myCells.size() > 0);
			trials += 2;
			Coordinate newSacrifice1 = myCells.pop_back();
			trialMove = Move(target, newSacrifice1, sacrifice2);
			trialScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
			if (trialScore.score > alpha) {
				alpha = trialScore.score;
				bestMove = trialMove;
				if (alpha >= beta) break; // prune
				myCells.push_back(sacrifice1);
				sacrifice1 = newSacrifice1;
			}
			else myCells.push_back(newSacrifice1);

			Coordinate newSacrifice2 = myCells.pop_back();
			trialMove = Move(target, sacrifice1, newSacrifice2);
			trialScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
			if (trialScore.score > alpha) {
				alpha = trialScore.score;
				bestMove = trialMove;
				if (alpha >= beta) break; // prune
				myCells.push_back(sacrifice2);
				sacrifice2 = newSacrifice2;
			}
			else myCells.push_back(newSacrifice2);
		}
	}
	cerr << "BirthRandSearch2 trials: " << trials << "\n";
	return MoveAndScore(bestMove, alpha);
}

//BirthRandSearch2::MoveAndScore BirthRandSearch2::getBestBirthMove(Board &board, Player playerID, Player enemyID,
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
//		Coordinate target = Tools::GetRandomElementFromVector(deadCells);
//		Coordinate sacrifice1 = Tools::PopRandomElementFromVector(myCells);
//		Coordinate sacrifice2 = Tools::GetRandomElementFromVector(myCells);
//		myCells.push_back(sacrifice1);
//		Move trialMove = Move(target, sacrifice1, sacrifice2);
//		int trialScore = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard);
//
//		if (trialScore > bestScore) {
//			bestScore = trialScore;
//			bestMove = trialMove;
//		}
//	}
//	//cerr << "BirthRandSearch2 trials: " << trials << "\n";
//	return MoveAndScore(bestMove, bestScore);
//}

BirthRandSearch2::BirthRandSearch2(int maxDepth, int* adversarialTrials) : maxDepth(maxDepth), adversarialTrials(adversarialTrials), 
																		   trialBoards(new Board*[maxDepth]), nextRoundBoards(new Board*[maxDepth]) {
	for (int i = 0; i < maxDepth; i++) {
		trialBoards[i] = NULL;
		nextRoundBoards[i] = NULL;
	}
}

Move BirthRandSearch2::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round) {
	long startTime = Tools::get_time();

	int roundsRemaining = 100 - round;
	int timeToUse = min(timePerMove + (time / roundsRemaining), time) - 5;

	for (int i = 0; i < maxDepth; i++) {
		if (trialBoards[i] == NULL) trialBoards[i] = new Board(board.getWidth(), board.getHeight());
		if (nextRoundBoards[i] == NULL) nextRoundBoards[i] = new Board(board.getWidth(), board.getHeight());
	}

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
	Board *nextRoundBoard = board.getNextRoundBoard();

	Board emptyBoard(board.getWidth(), board.getHeight());

	double alpha = -max_score;
	double beta = max_score;

	MoveAndScore bestKillMove = getBestKillMove(board, playerID, enemyID, enemyCells, myCells, *nextRoundBoard, alpha, beta);
	if (bestKillMove.score > alpha) {
		alpha = bestKillMove.score;
		if (alpha >= beta) return bestKillMove.move;
	}

	int timeUsed = Tools::get_time() - startTime;
	MoveAndScore bestBirthMove = getBestBirthMove(board, playerID, enemyID, deadCells, myCells, *nextRoundBoard, alpha, beta, timeToUse - timeUsed);

	delete nextRoundBoard;

	return bestBirthMove.score > bestKillMove.score ? bestBirthMove.move : bestKillMove.move;
}