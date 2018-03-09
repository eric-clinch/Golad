
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

struct BirthRandSearch2::Data {
	Player playerID;
	Player enemyID;
	int maxDepth;
	int *adversarialTrials;
	Board **trialBoards;
	Board **nextRoundBoards;
	LinkedList<Move> **killerMovesByRound;

	Data(Player playerID, Player enemyID, int boardWidth, int boardHeight, int maxDepth, int *adversarialTrials) {
		this->playerID = playerID;
		this->enemyID = enemyID;
		this->maxDepth = maxDepth;
		this->adversarialTrials = adversarialTrials;

		trialBoards = new Board*[maxDepth];
		nextRoundBoards = new Board*[maxDepth];
		killerMovesByRound = new LinkedList<Move>*[maxDepth];

		for (int i = 0; i < maxDepth; i++) {
			trialBoards[i] = new Board(boardWidth, boardHeight);
			nextRoundBoards[i] = new Board(boardWidth, boardHeight);
			
			int killerMovesToStore = ceil(sqrt(adversarialTrials[i]));
			killerMovesByRound[i] = new LinkedList<Move>(killerMovesToStore);
		}
	}

	~Data() {
		for (int i = 0; i < maxDepth; i++) {
			delete trialBoards[i];
			delete nextRoundBoards[i];
			delete killerMovesByRound[i];
		}
		delete trialBoards;
		delete nextRoundBoards;
		delete killerMovesByRound;
	}
};

struct BirthRandSearch2::ABMArgs {
	Board *board;
	Player playerID;
	Player enemyID;
	int time;
	MoveAndScore *result;

	int maxDepth;
	int *adversarialTrials;

	ABMArgs(Board *board, Player playerID, Player enemyID, int time, MoveAndScore *result, int maxDepth, int *adversarialTrials) {
		this->board = board;
		this->playerID = playerID;
		this->enemyID = enemyID;
		this->time = time;
		this->result = result;
		this->maxDepth = maxDepth;
		this->adversarialTrials = adversarialTrials;
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

	//if (availableMoves.size() == 0) {
	//	cerr << "playerID: " << playerID << " enemyID: " << enemyID << "\n";
	//	cerr << "player count: " << board.getPlayerCellCount(playerID) << " enemy count: " << board.getPlayerCellCount(enemyID) << "\n";
	//	cerr << board.toString() << "\n";
	//}
	assert(availableMoves.size() > 0);
	return availableMoves;
}

inline Move BirthRandSearch2::getRandomBirth(vector<Coordinate> &deadCells, vector<Coordinate> &myCells) {
	if (myCells.size() < 2) return Move();
	// randomly choose a target dead cell and two of my own cells to sacrifice
	Coordinate target = Tools::GetRandomElementFromVector(deadCells);
	Coordinate sacrifice1 = Tools::PopRandomElementFromVector(myCells); // pop the element so we don't choose the same element twice
	Coordinate sacrifice2 = Tools::GetRandomElementFromVector(myCells);
	myCells.push_back(sacrifice1);
	return Move(target, sacrifice1, sacrifice2);
}

Move BirthRandSearch2::getRandomMove(vector<MoveType> &availableMoveTypes, vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells) {
	assert(availableMoveTypes.size() > 0);
	int moveType = Tools::GetRandomElementFromVector(availableMoveTypes);

	if (moveType == BIRTH) {
		if (myCells.size() < 2) {
			return Move();
		}
		return getRandomBirth(deadCells, myCells);
	}
	else {
		assert(moveType == KILL);
		Coordinate target = Tools::GetRandomElementFromVector(enemyCells);
		return Move(target);
	}
}

double BirthRandSearch2::evaluateBoardMini(Board &board, int depth, Data &data, double alpha, double beta) {
	assert(alpha < beta);
	Board *trialBoard = data.trialBoards[depth];
	Board *nextRoundBoard = data.nextRoundBoards[depth];
	LinkedList<Move> *killerMoves = data.killerMovesByRound[depth];
	board.setNextRoundBoard(*nextRoundBoard);
	//assert(*nextRoundBoard == *board.getNextRoundBoard());

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(data.enemyID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(data.playerID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, data.enemyID, data.playerID);

	int maxTrials = data.adversarialTrials[depth];
	int numTrials = 0;
	for (Node<Move> *currentNode = killerMoves->getFront(); currentNode->next != NULL; currentNode = currentNode->next) {
		Move killerMove = currentNode->element;
		if (board.isLegal(killerMove, data.enemyID)) {
			numTrials++;
			double killerScore = getMoveScoreMini(board, killerMove, *nextRoundBoard, *trialBoard, depth + 1, data, alpha, beta);
			if (killerScore < beta) {
				beta = killerScore;
				if (alpha >= beta) {
					// move the current node to the front;
					killerMoves->moveToFront(currentNode);
					break; // prune
				}
			}
		}
	}

	for (; numTrials < maxTrials && alpha < beta; numTrials++) {
		Move trialMove = getRandomMove(availableMoveTypes, deadCells, myCells, enemyCells);
		double moveScore = getMoveScoreMini(board, trialMove, *nextRoundBoard, *trialBoard, depth + 1, data, alpha, beta);
		if (moveScore < beta) {
			beta = moveScore;
			if (alpha >= beta) {
				killerMoves->push_on(trialMove);
			}
		}
	}

	//cerr << "number of trials: " << numTrials << " best move: " << bestMove.toString() << "\n";
	return beta;
};

double BirthRandSearch2::evaluateBoardMaxi(Board &board, int depth, Data &data, double alpha, double beta) {
	assert(alpha < beta);
	Board *trialBoard = data.trialBoards[depth];
	Board *nextRoundBoard = data.nextRoundBoards[depth];
	LinkedList<Move> *killerMoves = data.killerMovesByRound[depth];
	board.setNextRoundBoard(*nextRoundBoard);
	//assert(*nextRoundBoard == *board.getNextRoundBoard());

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(data.playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(data.enemyID).at(0));
	vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(board, data.playerID, data.enemyID);

	int maxTrials = data.adversarialTrials[depth];
	int numTrials = 0;
	for (Node<Move> *currentNode = killerMoves->getFront(); currentNode->next != NULL; currentNode = currentNode->next) {
		Move killerMove = currentNode->element;
		if (board.isLegal(killerMove, data.playerID)) {
			numTrials++;
			double killerScore = getMoveScoreMaxi(board, killerMove, *nextRoundBoard, *trialBoard, depth + 1, data, alpha, beta);
			if (killerScore > alpha) {
				alpha = killerScore;
				if (alpha >= beta) {
					// move the current node to the front;
					killerMoves->moveToFront(currentNode);
					break; // prune
				}
			}
		}
	}

	for (; numTrials < maxTrials && alpha < beta; numTrials++) {
		//Move trialMove = getTrialMove(deadCells, myCells, enemyCells, i);
		Move trialMove = getRandomMove(availableMoveTypes, deadCells, myCells, enemyCells);
		double moveScore = getMoveScoreMaxi(board, trialMove, *nextRoundBoard, *trialBoard, depth + 1, data, alpha, beta);
		if (moveScore > alpha) {
			alpha = moveScore;
			if (alpha >= beta) {
				killerMoves->push_on(trialMove);
			}
		}
	}

	return alpha;
};

inline double BirthRandSearch2::getMoveScoreMini(Board &board, Move &move, Board &nextRoundBoard, Board &trialBoard,
	int depth, Data &data, double alpha, double beta) {
	assert(alpha < beta);
	board.applyMove(move, data.enemyID, nextRoundBoard, trialBoard);
	//assert(trialBoard == *board.makeMove(move, enemyID));
	if (trialBoard.getPlayerCellCount(data.playerID) == 0) return -max_score;
	else if (trialBoard.getPlayerCellCount(data.enemyID) == 0) return max_score;

	if (depth == data.maxDepth) {
		double score = (double)trialBoard.getPlayerCellCount(data.playerID) / trialBoard.getPlayerCellCount(data.enemyID);
		return score;
	}
	else {
		return evaluateBoardMaxi(trialBoard, depth, data, alpha, beta);
	}
}

inline double BirthRandSearch2::getMoveScoreMaxi(Board &board, Move &move, Board &nextRoundBoard, Board &trialBoard,
	int depth, Data &data, double alpha, double beta) {
	assert(alpha < beta);
	board.applyMove(move, data.playerID, nextRoundBoard, trialBoard);
	//assert(trialBoard == *board.makeMove(move, playerID));
	if (trialBoard.getPlayerCellCount(data.playerID) == 0) return -max_score;
	else if (trialBoard.getPlayerCellCount(data.enemyID) == 0) return max_score;

	if (depth == data.maxDepth) {
		double score = (double)trialBoard.getPlayerCellCount(data.playerID) / trialBoard.getPlayerCellCount(data.enemyID);
		return score;
	}
	else {
		return evaluateBoardMini(trialBoard, depth, data, alpha, beta);
	}
}

BirthRandSearch2::MoveAndScore BirthRandSearch2::getBestKillMove(Board &board, vector<Coordinate> &enemyCells, 
																 vector<Coordinate> &myCells, Board &nextRoundBoard, Data &data) {
	double alpha = -max_score;
	double beta = max_score;
	Board emptyBoard(board.getWidth(), board.getHeight());

	Move bestMove = Move();
	double moveScore = getMoveScoreMaxi(board, bestMove, nextRoundBoard, emptyBoard, 0, data, alpha, beta);
	assert(moveScore >= alpha);
	alpha = moveScore;
	if (beta <= alpha) return MoveAndScore(bestMove, moveScore); // prune

	int numEnemyCells = enemyCells.size();
	for (int i = 0; i < numEnemyCells; i++) {
		Coordinate enemyCell = enemyCells[i];
		Move trialMove = Move(enemyCell);
		double trialMoveScore = getMoveScoreMaxi(board, trialMove, nextRoundBoard, emptyBoard, 0, data, alpha, beta);
		if (trialMoveScore > alpha) {
			alpha = trialMoveScore;
			bestMove = trialMove;
			if (beta <= alpha) return MoveAndScore(bestMove, alpha);
		}
	}

	int numMyCells = myCells.size();
	for (int i = 0; i < numMyCells; i++) {
		Coordinate myCell = myCells[i];
		Move trialMove = Move(myCell);
		double trialMoveScore = getMoveScoreMaxi(board, trialMove, nextRoundBoard, emptyBoard, 0, data, alpha, beta);
		if (trialMoveScore > alpha) {
			alpha = trialMoveScore;
			bestMove = trialMove;
			if (beta <= alpha) return MoveAndScore(bestMove, alpha);
		}
	}

	return MoveAndScore(bestMove, alpha);
}

BirthRandSearch2::MoveAndScore BirthRandSearch2::getBestBirthMove(Board &board, vector<Coordinate> &deadCellsVect, 
																  vector<Coordinate> &myCellsVect, Board &nextRoundBoard, Data &data, int time) {
	long startTime = Tools::get_time();

	double alpha = -max_score;
	double beta = max_score;
	Board emptyBoard(board.getWidth(), board.getHeight());

	if (board.getPlayerCellCount(data.playerID) < 2) {
		// not possible to birth, so just pass
		Move passMove = Move();
		double passScore = getMoveScoreMaxi(board, passMove, nextRoundBoard, emptyBoard, 0, data, alpha, beta);
		return MoveAndScore(passMove, passScore);
	}

	RandomVector<Coordinate> deadCells(deadCellsVect);
	RandomVector<Coordinate> myCells(myCellsVect);

	Coordinate target = deadCells.pop_back();
	Coordinate sacrifice1 = myCells.pop_back();
	Coordinate sacrifice2 = myCells.pop_back();
	Move bestMove = Move(target, sacrifice1, sacrifice2);
	double moveScore = getMoveScoreMaxi(board, bestMove, nextRoundBoard, emptyBoard, 0, data, alpha, beta);
	alpha = moveScore;

	int myRemainingCells = myCells.size();
	int trials = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < time; currentTime = Tools::get_time()) {
		assert(deadCells.size() > 0);
		assert(alpha < beta);
		Coordinate newTarget = deadCells.pop_back();
		Move trialMove = Move(newTarget, sacrifice1, sacrifice2);
		double trialScore = getMoveScoreMaxi(board, trialMove, nextRoundBoard, emptyBoard, 0, data, alpha, beta);

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
			trialScore = getMoveScoreMaxi(board, trialMove, nextRoundBoard, emptyBoard, 0, data, alpha, beta);
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
			trialScore = getMoveScoreMaxi(board, trialMove, nextRoundBoard, emptyBoard, 0, data, alpha, beta);
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
	cerr << "BirthRandSearch2 trials: " << trials << "\n";
	return MoveAndScore(bestMove, alpha);
}

void *BirthRandSearch2::approximateBestMove(void *voidArg) {
	long startTime = Tools::get_time();

	ABMArgs arg = *(ABMArgs*) voidArg;

	Board *board = arg.board;
	Player playerID = arg.playerID;
	Player enemyID = arg.enemyID;
	int time = arg.time;
	MoveAndScore *result = arg.result;

	vector<Coordinate> deadCells = board->GetCells('.');
	vector<Coordinate> myCells = board->GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board->GetCells(to_string(enemyID).at(0));
	Board *nextRoundBoard = board->getNextRoundBoard();

	Data data(playerID, enemyID, board->getWidth(), board->getHeight(), arg.maxDepth, arg.adversarialTrials);

	MoveAndScore bestKillMove = getBestKillMove(*board, enemyCells, myCells, *nextRoundBoard, data);
	if (bestKillMove.score >= max_score) {
		*result = bestKillMove;
		return NULL; // prune
	}

	int timeUsed = Tools::get_time() - startTime;
	MoveAndScore bestBirthMove = getBestBirthMove(*board, deadCells, myCells, *nextRoundBoard, data, time - timeUsed);

	delete nextRoundBoard;

	*result = bestBirthMove.score >= bestKillMove.score ? bestBirthMove : bestKillMove;
	return NULL;
}

BirthRandSearch2::BirthRandSearch2(int maxDepth, int* adversarialTrials) : maxDepth(maxDepth), adversarialTrials(adversarialTrials) {}

BirthRandSearch2::~BirthRandSearch2() {}

Move BirthRandSearch2::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round) {
	int roundsRemaining = 100 - round; // the maximum number of rounds that will be played after this round
	int extraTimeGiven = roundsRemaining * timePerMove;
	int timeToUse = (time + extraTimeGiven) / (roundsRemaining + 1);
	timeToUse -= 10;

	cerr << "round: " << round << "\n";

	MoveAndScore *bestMoveAndScore1 = new MoveAndScore(Move(), 0);
	MoveAndScore *bestMoveAndScore2 = new MoveAndScore(Move(), 0);

	Board *copied = board.getCopy();
	ABMArgs thread1Arg(copied, playerID, enemyID, timeToUse, bestMoveAndScore1, maxDepth, adversarialTrials);
	pthread_t thread1;
	pthread_create(&thread1, NULL, &approximateBestMove, (void*) &thread1Arg);

	ABMArgs thread2Arg(&board, playerID, enemyID, timeToUse, bestMoveAndScore2, maxDepth, adversarialTrials);
	approximateBestMove((void*) &thread2Arg);
	pthread_join(thread1, NULL);
	
	Move bestMove = bestMoveAndScore1->score > bestMoveAndScore2->score ? bestMoveAndScore1->move : bestMoveAndScore2->move;
	delete bestMoveAndScore1;
	delete bestMoveAndScore2;
	return bestMove;
}