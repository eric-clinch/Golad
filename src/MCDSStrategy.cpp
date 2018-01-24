
#include "MCDSStrategy.h"

using namespace std;

double MCDSStrategy::max_score = 288.0;

struct MCDSStrategy::MoveAndScore {
	Move move;
	double score;

	MoveAndScore(Move m, double s) {
		move = m;
		score = s;
	}
};

struct MCDSStrategy::CoordinateAndScore {
	Coordinate coordinate;
	double scoreSum;
	int numSimulations;
	
	CoordinateAndScore(Coordinate c) {
		coordinate = c;
		scoreSum = 0;
		numSimulations = 0;
	}

	double getScore(double confidenceNumerator) {
		if (numSimulations == 0) {
			// this value must be strictly greater than the normal return score
			// in order to guarantee that it will be picked before already tested coordinates
			return max_score + confidenceNumerator;
		}
		else return scoreSum / (double)numSimulations + sqrt(confidenceNumerator / numSimulations);
	}

	void updateScore(double score) {
		scoreSum += score;
		numSimulations += 1;
	}
};

vector<MoveType> MCDSStrategy::GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID) {
	vector<MoveType> availableMoves;

	if (board.getPlayerCellCount(enemyID) > 0) {
		availableMoves.push_back(KILL);
	}

	if (board.getPlayerCellCount(playerID) > 1) {
		availableMoves.push_back(BIRTH);
	}

	return availableMoves;
}

Move MCDSStrategy::getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes,
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

inline double MCDSStrategy::getMoveScore(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &trialBoard, int depth) {
	
	Board *boards[] = { new Board(board.getWidth(), board.getHeight()), new Board(board.getWidth(), board.getHeight()) };
	int currentBoardIndex = 0;
	Board *currentBoard = boards[currentBoardIndex];
	Board *nextBoard = boards[1 - currentBoardIndex];

	board.applyMove(move, playerID, nextRoundBoard, *currentBoard);

	Player currentPlayer, otherPlayer;
	for (int round = 0; currentBoard->getPlayerCellCount(playerID) > 0 && currentBoard->getPlayerCellCount(enemyID) > 0 && round < 5; round++) {
		if (round % 2 == 0) {
			currentPlayer = enemyID;
			otherPlayer = playerID;
		}
		else {
			currentPlayer = playerID;
			otherPlayer = enemyID;
		}

		vector<MoveType> availableMoveTypes = GetAvailableMoveTypes(*currentBoard, currentPlayer, otherPlayer);
		vector<Coordinate> currentPlayerCells = currentBoard->GetCells(to_string(currentPlayer).at(0));
		vector<Coordinate> otherPlayerCells = currentBoard->GetCells(to_string(otherPlayer).at(0));
		vector<Coordinate> deadCells = currentBoard->GetCells('.');
		Move m = getRandomMove(*currentBoard, currentPlayer, otherPlayer, availableMoveTypes, deadCells, currentPlayerCells, otherPlayerCells);
		currentBoard->makeMove(m, currentPlayer, nextRoundBoard);
		
		currentBoardIndex = 1 - currentBoardIndex;
		currentBoard = boards[currentBoardIndex];
		nextBoard = boards[1 - currentBoardIndex];
	}

	int playerCellCount = currentBoard->getPlayerCellCount(playerID);
	int enemyCellCount = currentBoard->getPlayerCellCount(enemyID);

	delete currentBoard;
	delete nextBoard;

	if (playerCellCount > enemyCellCount) return 1;
	else return 0;
}

MCDSStrategy::MoveAndScore MCDSStrategy::getBestKillMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &enemyCells,
	vector<Coordinate> &myCells, Board &nextRoundBoard) {

	Board emptyBoard(board.getWidth(), board.getHeight());

	Move bestMove = Move();
	double bestMoveScore = getMoveScore(board, playerID, enemyID, bestMove, nextRoundBoard, emptyBoard, 0);

	int numMyCells = myCells.size();
	for (int i = 0; i < numMyCells; i++) {
		Coordinate myCell = myCells[i];
		Move trialMove = Move(myCell);
		double trialMoveScore = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, emptyBoard, 0);
		if (trialMoveScore > bestMoveScore) {
			bestMoveScore = trialMoveScore;
			bestMove = trialMove;
		}
	}

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

MCDSStrategy::CoordinateAndScore* MCDSStrategy::getBestSacrifices(vector<MCDSStrategy::CoordinateAndScore> &myCells, int totalSimulations) {
	assert(myCells.size() >= 2);
	double confidenceNumerator = confidenceConstant * log(totalSimulations);

	CoordinateAndScore bestSacrifice = myCells[0];
	double bestScore = bestSacrifice.getScore(confidenceNumerator);

	CoordinateAndScore trialSacrifice = myCells[1];
	double trialScore = trialSacrifice.getScore(confidenceNumerator);
	CoordinateAndScore nextBestSacrifice = trialSacrifice;
	double nextBestScore = trialScore;
	if (trialScore > bestScore) {
		nextBestSacrifice = bestSacrifice;
		nextBestScore = bestScore;
		bestSacrifice = trialSacrifice;
		bestScore = trialScore;
	}

	for (int i = 2; i < myCells.size(); i++) {
		CoordinateAndScore trialSacrifice = myCells[i];
		double trialScore = trialSacrifice.getScore(confidenceNumerator);

		if (trialScore > bestScore) {
			nextBestSacrifice = bestSacrifice;
			nextBestScore = bestScore;
			bestSacrifice = trialSacrifice;
			bestScore = trialScore;
		}
		else if (trialScore > nextBestScore) {
			nextBestSacrifice = trialSacrifice;
			nextBestScore = trialScore;
		}
	}

	CoordinateAndScore *result = new CoordinateAndScore[2]{ bestSacrifice, nextBestSacrifice };
	return result;
}

Coordinate* MCDSStrategy::getResultSacrifices(vector <CoordinateAndScore> &myCells) {
	assert(myCells.size() >= 2);
	CoordinateAndScore bestSacrifice = myCells[0];
	double bestScore = bestSacrifice.numSimulations;

	CoordinateAndScore trialSacrifice = myCells[1];
	double trialScore = trialSacrifice.numSimulations;
	CoordinateAndScore nextBestSacrifice = trialSacrifice;
	double nextBestScore = trialScore;
	if (trialScore > bestScore) {
		nextBestSacrifice = bestSacrifice;
		nextBestScore = bestScore;
		bestSacrifice = trialSacrifice;
		bestScore = trialScore;
	}

	for (int i = 2; i < myCells.size(); i++) {
		CoordinateAndScore trialSacrifice = myCells[i];
		double trialScore = trialSacrifice.numSimulations;

		if (trialScore > bestScore) {
			nextBestSacrifice = bestSacrifice;
			nextBestScore = bestScore;
			bestSacrifice = trialSacrifice;
			bestScore = trialScore;
		}
		else if (trialScore > nextBestScore) {
			nextBestSacrifice = trialSacrifice;
			nextBestScore = trialScore;
		}
	}

	Coordinate *result = new Coordinate[2]{ bestSacrifice.coordinate, nextBestSacrifice.coordinate };
	return result;
}

MCDSStrategy::CoordinateAndScore MCDSStrategy::getBestBirthTarget(vector<CoordinateAndScore> &deadCells, int totalSimulations) {
	assert(deadCells.size() > 0);
	double confidenceNumerator = confidenceConstant * log(totalSimulations);

	CoordinateAndScore bestTarget = deadCells[0];
	double bestScore = bestTarget.getScore(confidenceNumerator);
	for (int i = 1; i < deadCells.size(); i++) {
		CoordinateAndScore trialTarget = deadCells[i];
		double trialScore = trialTarget.getScore(confidenceNumerator);
		if (trialScore > bestScore) {
			bestTarget = trialTarget;
			bestScore = trialScore;
		}
	}

	return bestTarget;
}

Coordinate MCDSStrategy::getResultBirthTarget(vector<CoordinateAndScore> &deadCells) {
	assert(deadCells.size() > 0);
	CoordinateAndScore bestTarget = deadCells[0];
	double bestScore = bestTarget.numSimulations;
	for (int i = 1; i < deadCells.size(); i++) {
		CoordinateAndScore trialTarget = deadCells[i];
		double trialScore = trialTarget.numSimulations;
		if (trialScore > bestScore) {
			bestTarget = trialTarget;
			bestScore = trialScore;
		}
	}

	return bestTarget.coordinate;
}

void MCDSStrategy::printScores(vector<CoordinateAndScore> &deadCells, vector<CoordinateAndScore> &myCells, int totalSimulations) {
	for (int i = 0; i < deadCells.size(); i++) {
		CoordinateAndScore c = deadCells[i];
		cerr << c.coordinate.toString() << " " << c.getScore(totalSimulations) << " ";
	}
	for (int i = 0; i < myCells.size(); i++) {
		CoordinateAndScore c = myCells[i];
		cerr << c.coordinate.toString() << " " << c.getScore(totalSimulations) << " ";
	}
	cerr << "\n";
}

MCDSStrategy::MoveAndScore MCDSStrategy::getBestBirthMove(Board &board, Player playerID, Player enemyID,
	vector<Coordinate> &deadCellsVect, vector<Coordinate> &myCellsVect, Board &nextRoundBoard, int time) {
	long startTime = Tools::get_time();
	Board trialBoard(board.getWidth(), board.getHeight());

	if (board.getPlayerCellCount(playerID) < 2) {
		// not possible to birth, so just pass
		Move passMove = Move();
		double passScore = getMoveScore(board, playerID, enemyID, passMove, nextRoundBoard, trialBoard, 0);
		return MoveAndScore(passMove, passScore);
	}
	
	vector<CoordinateAndScore> deadCells = vector<CoordinateAndScore>();
	for (Coordinate c : deadCellsVect) deadCells.push_back(CoordinateAndScore(c));
	random_shuffle(deadCells.begin(), deadCells.end());

	vector<CoordinateAndScore> myCells = vector<CoordinateAndScore>();
	for (Coordinate c : myCellsVect) myCells.push_back(CoordinateAndScore(c));
	random_shuffle(myCells.begin(), myCells.end());

	int totalSimulations = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < time; currentTime = Tools::get_time()) {
		//printScores(deadCells, myCells, totalSimulations);

		CoordinateAndScore *sacrifices = getBestSacrifices(myCells, totalSimulations);
		CoordinateAndScore sacrifice1 = sacrifices[0];
		CoordinateAndScore sacrifice2 = sacrifices[1];
		delete sacrifices;

		CoordinateAndScore target = getBestBirthTarget(deadCells, totalSimulations);
		
		Move trialMove = Move(target.coordinate, sacrifice1.coordinate, sacrifice2.coordinate);
		double score = getMoveScore(board, playerID, enemyID, trialMove, nextRoundBoard, trialBoard, 0);
		target.updateScore(score);
		sacrifice1.updateScore(score);
		sacrifice2.updateScore(score);
		totalSimulations += 1;
	}
	cerr << "total simulations: " << totalSimulations << "\n";

	Coordinate *sacrifices = getResultSacrifices(myCells);
	Coordinate sacrifice1 = sacrifices[0];
	Coordinate sacrifice2 = sacrifices[1];
	Coordinate target = getResultBirthTarget(deadCells);

	Move resultMove(target, sacrifice1, sacrifice2);
	return MoveAndScore(resultMove, max_score);
}

Move MCDSStrategy::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove) {
	long startTime = Tools::get_time();

	vector<Coordinate> deadCells = board.GetCells('.');
	vector<Coordinate> myCells = board.GetCells(to_string(playerID).at(0));
	vector<Coordinate> enemyCells = board.GetCells(to_string(enemyID).at(0));
	Board *nextRoundBoard = board.getNextRoundBoard();

	Board emptyBoard(board.getWidth(), board.getHeight());

	MoveAndScore bestKillMove = getBestKillMove(board, playerID, enemyID, enemyCells, myCells, *nextRoundBoard);

	int dt = Tools::get_time() - startTime;
	MoveAndScore bestBirthMove = getBestBirthMove(board, playerID, enemyID, deadCells, myCells, *nextRoundBoard, timePerMove - dt);

	delete nextRoundBoard;

	Move result = bestBirthMove.score > bestKillMove.score ? bestBirthMove.move : bestKillMove.move;
	return result;
}

MCDSStrategy::MCDSStrategy(int maxDepth, int *adversarialTrials, double confidenceConstant) {
	this->maxDepth = maxDepth;
	this->adversarialTrials = adversarialTrials;
	this->confidenceConstant = confidenceConstant;
}