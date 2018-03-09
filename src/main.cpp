
#ifdef _DEBUG
	#include <stdlib.h>
	#include <crtdbg.h>
	#define _CRTDBG_MAP_ALLOC
	#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
#else
	#define _CrtDumpMemoryLeaks() {}
	#define NDEBUG
#endif

#include <ctime>
#include <chrono>
#include <stdio.h>
#include <cstdio>
#include <assert.h>
#include <math.h>
#include "Parser.h"
#include "Bot.h"
#include "Strategy.h"
#include "RandomStrategy.h"
#include "RandSearchStrategy.h"
#include "AdversarialStrategy.h"
#include "BirthRandSearch.h"
#include "BirthRandSearch2.h"
#include "BirthRandSearchKiller.h"
#include "MCDSStrategy.h"
#include "Board.h"
#include "Tools.h"
#include "LinkedList.h"
#include "Stats.h"

using namespace std;

void printArray(int **a, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			cout << a[i][j] << " ";
		}
		cout << "\n";
	}
	cout << "\n";
}

void test() {

}

void play() {
	int bot0AdversarialTrials[] = { 600 };
	BirthRandSearch2 strategy = BirthRandSearch2(1, bot0AdversarialTrials);
	Bot myBot = Bot(&strategy);
	Parser parser = Parser(myBot);
	parser.Parse();
}

int playMatch(Bot bot0, Bot bot1, bool verbose = false) {
	Board board(18, 16);
	board.initiateBoardPositions();
	bot0.SetBoard(&board);
	bot1.SetBoard(&board);
	bot0.SetYourBotId(P0);
	bot1.SetYourBotId(P1);

	int bot0Time = 10000;
	int bot1Time = 10000;
	int timePerMove = 170;
	bot0.SetTimePerMove(timePerMove);
	bot1.SetTimePerMove(timePerMove);

	for (int round = 0; round < 50 && bot0Time >= 0 && bot1Time >= 0; round++) {
		if (verbose) cout << board.toString();
		int P0Count = board.getPlayerCellCount(P0);
		int P1Count = board.getPlayerCellCount(P1);
		if (P0Count == 0 && P1Count == 0) return -1; // tie
		else if (P0Count == 0) return 1;
		else if (P1Count == 0) return 0;
		Board *boardCopy = board.getCopy();
		
		// player 0's turn
		bot0Time = min(bot0Time + timePerMove, 10000);
		if (verbose) cout << "bot 0 time: " << bot0Time << "\n";
		long startTime = Tools::get_time();
		Move move = bot0.GetMove(bot0Time, round);
		assert(board == *boardCopy);
		int dt = Tools::get_time() - startTime;
		bot0Time -= dt;
		if (verbose) cout << "Player 0's move: " << move.toString() << " time to compute: " << dt << "\n\n";
		board.makeMoveOnBoard(move, P0);

		if (verbose) cout << board.toString();
		P0Count = board.getPlayerCellCount(P0);
		P1Count = board.getPlayerCellCount(P1);
		if (P0Count == 0 && P1Count == 0) return -1; // tie
		else if (P0Count == 0) return 1;
		else if (P1Count == 0) return 0;
		delete boardCopy;
		boardCopy = board.getCopy();

		// player 1's turn
		bot1Time = min(bot1Time + timePerMove, 10000);
		if (verbose) cout << "bot 1 time: " << bot1Time << "\n";
		startTime = Tools::get_time();
		move = bot1.GetMove(bot1Time, round);
		assert(board == *boardCopy);
		dt = Tools::get_time() - startTime;
		bot1Time -= dt;
		if (verbose) cout << "Player 1's move: " << move.toString() << " time to compute: " << dt << "\n\n";
		board.makeMoveOnBoard(move, P1);

		delete boardCopy;
	}

	if (bot0Time < 0) {
		cout << "bot 0 ran out of time\n";
		return 1;
	}
	else if (bot1Time < 0) {
		cout << "bot 1 ran out of time\n";
		return 0;
	}

	int P0Count = board.getPlayerCellCount(P0);
	int P1Count = board.getPlayerCellCount(P1);
	if (P0Count == P1Count) return -1; // tie
	return P0Count > P1Count ? 0 : 1;
}

void playTournament(Bot bot0, Bot bot1, int rounds = 100, bool verbose = false) {
	int bot0Wins = 0;
	int bot1Wins = 0;

	for (int currentRound = 1; currentRound <= rounds; currentRound++) {
		int matchResult;
		if (currentRound % 2 == 1) {
			matchResult = playMatch(bot0, bot1, verbose);
		}
		else {
			matchResult = 1 - playMatch(bot1, bot0, verbose);
			if (matchResult == 2) matchResult = -1; // ties should be -1
		}
		if (matchResult == 0) bot0Wins++;
		else if (matchResult == 1) bot1Wins++;
		cout << "match winner: " << matchResult << "\n";
		if (currentRound % 10 == 0) {
			int ties = currentRound - bot0Wins - bot1Wins;
			cout << "bot 0 wins: " << bot0Wins << "\nbot 1 wins: " << bot1Wins << "\nties: " << ties << "\n";
			int numSamples = currentRound - ties;
			if (numSamples > 30) { // if the Central Limit Theorem Applies
				double probabilityBot1IsBetter = Stats::getProbabilitySuccessfulBernoulli(bot1Wins, numSamples);
				double probabilityBot0IsBetter = 1 - probabilityBot1IsBetter;
				cout << "probability that bot 0 is better: " << 100 * probabilityBot0IsBetter << "%\n";
				cout << "probability that bot 1 is better: " << 100 * probabilityBot1IsBetter << "%\n";
				if (probabilityBot0IsBetter > .995 || probabilityBot1IsBetter > .995) break; // the tournament is complete at this point
			}
		}
	}
}

void playTest() {
	int bot0AdversarialTrials[] = { 600 };
	BirthRandSearch bot0Strategy = BirthRandSearch(1, bot0AdversarialTrials);
	Bot bot0 = Bot(&bot0Strategy);

	int bot1AdversarialTrials[] = { 600 };
	BirthRandSearch2 bot1Strategy = BirthRandSearch2(1, bot1AdversarialTrials);
	Bot bot1 = Bot(&bot1Strategy);

	playTournament(bot0, bot1, 1000);
	//playMatch(bot0, bot1, true);
}

int main() {
	// Initialize random number generator
	srand(time(NULL));

	//unsigned availableThreads = thread::hardware_concurrency();
	//cerr << "available threads: " << availableThreads << "\n";

	//freopen("cerr_log.txt", "w", stderr);

	//playTest();
	//test();
	play();
	
	delete Board::simulationLookupTable;
	_CrtDumpMemoryLeaks();
}