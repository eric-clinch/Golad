
#include "BirthRandSearch.h"

using namespace std;

double BirthRandSearch::max_score = 288.0;

struct BirthRandSearch::MoveAndScore {
	Move move;
	double score;

	MoveAndScore(Move m, double s) {
		move = m;
		score = s;
	}
};

vector<MoveType> BirthRandSearch::GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID) {
	vector<MoveType> availableMoves;

	if (board.getPlayerCellCount(enemyID) > 0) {
		availableMoves.push_back(KILL);
	}

	if (board.getPlayerCellCount(playerID) > 1) {
		availableMoves.push_back(BIRTH);
	}

	return availableMoves;
}

Move BirthRandSearch::getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes,
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

double BirthRandSearch::evaluateBoardMini(Board &board, Player playerID, Player enemyID, int trials, int depth, double alpha, double beta) {
	assert(alpha < beta);
	Board *trialBoard = trialBoards[depth];
	Board *nextRoundBoard = nextRoundBoards[depth];
	board.setNextRoundBoard(*nextRoundBoard);
	assert(*nextRoundBoard == *board.getNextRoundBoard());

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(enemyID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(playerID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, enemyID, playerID);

	for (int i = 0; i < trials; i++) {
		Move trialMove = getRandomMove(board, enemyID, playerID, availableMoveTypes, deadCells, myCells, enemyCells);
		double moveScore = getMoveScoreMini(board, playerID, enemyID, trialMove, *nextRoundBoard, *trialBoard, depth + 1, alpha, beta);
		if (moveScore < beta) {
			beta = moveScore;
			if (beta <= alpha) break; // prune
		}
	}

	return beta;
};

double BirthRandSearch::evaluateBoardMaxi(Board &board, Player playerID, Player enemyID, int trials, int depth, double alpha, double beta) {
	assert(alpha < beta);
	Board *trialBoard = trialBoards[depth];
	Board *nextRoundBoard = nextRoundBoards[depth];
	board.setNextRoundBoard(*nextRoundBoard);
	assert(*nextRoundBoard == *board.getNextRoundBoard());

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, playerID, enemyID);

	for (int i = 0; i < trials; i++) {
		Move trialMove = getRandomMove(board, playerID, enemyID, availableMoveTypes, deadCells, myCells, enemyCells);
		double moveScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, *nextRoundBoard, *trialBoard, depth + 1, alpha, beta);
		if (moveScore > alpha) {
			alpha = moveScore;
			if (beta <= alpha) break; // prune
		}
	}

	return alpha;
};

inline double BirthRandSearch::getMoveScoreMini(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &trialBoard,
	int depth, double alpha, double beta) {
	assert(alpha < beta);
	board.applyMove(move, enemyID, nextRoundBoard, trialBoard);
	assert(trialBoard == *board.makeMove(move, enemyID));
	if (trialBoard.getPlayerCellCount(playerID) == 0) return -max_score;
	else if (trialBoard.getPlayerCellCount(enemyID) == 0) return max_score;

	if (depth == maxDepth) {
		return (double)trialBoard.getPlayerCellCount(playerID) / trialBoard.getPlayerCellCount(enemyID);
	}
	else {
		return evaluateBoardMaxi(trialBoard, playerID, enemyID, adversarialTrials[depth], depth, alpha, beta);
	}
}

inline double BirthRandSearch::getMoveScoreMaxi(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &trialBoard,
	int depth, double alpha, double beta) {
	assert(alpha < beta);
	board.applyMove(move, playerID, nextRoundBoard, trialBoard);
	assert(trialBoard == *board.makeMove(move, playerID));
	if (trialBoard.getPlayerCellCount(playerID) == 0) return -max_score;
	else if (trialBoard.getPlayerCellCount(enemyID) == 0) return max_score;

	if (depth == maxDepth) {
		return (double)trialBoard.getPlayerCellCount(playerID) / trialBoard.getPlayerCellCount(enemyID);
	}
	else {
		return evaluateBoardMini(trialBoard, playerID, enemyID, adversarialTrials[depth], depth, alpha, beta);
	}
}

BirthRandSearch::MoveAndScore BirthRandSearch::getBestKillMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &enemyCells,
	vector<Coordinate> &myCells, Board &nextRoundBoard) {

	Board emptyBoard(board.getWidth(), board.getHeight());

	double alpha = -max_score;
	double beta = max_score;

	Move bestMove = Move();
	double moveScore = getMoveScoreMaxi(board, playerID, enemyID, bestMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
	assert(moveScore >= alpha);
	alpha = moveScore;
	if (beta <= alpha) return MoveAndScore(bestMove, moveScore); // prune

	int numMyCells = myCells.size();
	for (int i = 0; i < numMyCells; i++) {
		Coordinate myCell = myCells[i];
		Move trialMove = Move(myCell);
		double trialMoveScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
		if (trialMoveScore > alpha) {
			alpha = trialMoveScore;
			bestMove = trialMove;
			if (beta <= alpha) return MoveAndScore(bestMove, alpha);
		}
	}

	int numEnemyCells = enemyCells.size();
	for (int i = 0; i < numEnemyCells; i++) {
		Coordinate enemyCell = enemyCells[i];
		Move trialMove = Move(enemyCell);
		double trialMoveScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
		if (trialMoveScore > alpha) {
			alpha = trialMoveScore;
			bestMove = trialMove;
			if (beta <= alpha) return MoveAndScore(bestMove, alpha);
		}
	}

	return MoveAndScore(bestMove, alpha);
}

BirthRandSearch::MoveAndScore BirthRandSearch::getBestBirthMove(Board &board, Player playerID, Player enemyID,
	vector<Coordinate> &deadCellsVect, vector<Coordinate> &myCellsVect, Board &nextRoundBoard, int time) {
	long startTime = Tools::get_time();

	double alpha = -max_score;
	double beta = max_score;

	Board emptyBoard(board.getWidth(), board.getHeight());

	if (board.getPlayerCellCount(playerID) < 2) {
		// not possible to birth, so just pass
		Move passMove = Move();
		double passScore = getMoveScoreMaxi(board, playerID, enemyID, passMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
		return MoveAndScore(passMove, passScore);
	}

	RandomVector<Coordinate> deadCells(deadCellsVect);
	RandomVector<Coordinate> myCells(myCellsVect);

	Coordinate target = deadCells.pop_back();
	Coordinate sacrifice1 = myCells.pop_back();
	Coordinate sacrifice2 = myCells.pop_back();
	Move bestMove = Move(target, sacrifice1, sacrifice2);
	double moveScore = getMoveScoreMaxi(board, playerID, enemyID, bestMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
	assert(moveScore >= alpha);
	alpha = moveScore;
	if (beta <= alpha) return MoveAndScore(bestMove, moveScore); // prune

	int myRemainingCells = myCells.size();
	int trials = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < time; currentTime = Tools::get_time()) {
		assert(deadCells.size() > 0);
		assert(alpha < beta);
		Coordinate newTarget = deadCells.pop_back();
		Move trialMove = Move(newTarget, sacrifice1, sacrifice2);
		double trialScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0, alpha, beta);

		if (trialScore > alpha) {
			alpha = trialScore;
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
			if (trialScore > alpha) {
				alpha = trialScore;
				bestMove = trialMove;
				if (alpha >= beta) break; // prune
				myCells.push_back(sacrifice1);
				sacrifice1 = newSacrifice1;
			}
			else myCells.push_back(newSacrifice1);

			Coordinate newSacrifice2 = myCells.pop_back();
			trialMove = Move(target, sacrifice1, newSacrifice2);
			trialScore = getMoveScoreMaxi(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0, alpha, beta);
			if (trialScore > alpha) {
				alpha = trialScore;
				bestMove = trialMove;
				if (alpha >= beta) break; // prune
				myCells.push_back(sacrifice2);
				sacrifice2 = newSacrifice2;
			}
			else myCells.push_back(newSacrifice2);
		}
	}
	cerr << "BirthRandSearch trials: " << trials << "\n";
	return MoveAndScore(bestMove, alpha);
}

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
//	//cerr << "BirthRandSearch trials: " << trials << "\n";
//	return MoveAndScore(bestMove, bestScore);
//}

BirthRandSearch::BirthRandSearch(int maxDepth, int* adversarialTrials) : maxDepth(maxDepth), adversarialTrials(adversarialTrials),
trialBoards(new Board*[maxDepth]), nextRoundBoards(new Board*[maxDepth]) {
	for (int i = 0; i < maxDepth; i++) {
		trialBoards[i] = NULL;
		nextRoundBoards[i] = NULL;
	}
}

BirthRandSearch::~BirthRandSearch() {
	for (int i = 0; i < maxDepth; i++) {
		if (trialBoards[i] != NULL) delete trialBoards[i];
		if (nextRoundBoards[i] != NULL) delete nextRoundBoards[i];
	}
	delete trialBoards;
	delete nextRoundBoards;
}

Move BirthRandSearch::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round) {
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

	MoveAndScore bestKillMove = getBestKillMove(board, playerID, enemyID, enemyCells, myCells, *nextRoundBoard);

	int dt = Tools::get_time() - startTime;
	MoveAndScore bestBirthMove = getBestBirthMove(board, playerID, enemyID, deadCells, myCells, *nextRoundBoard, timeToUse - dt);

	delete nextRoundBoard;

	return bestBirthMove.score > bestKillMove.score ? bestBirthMove.move : bestKillMove.move;
}