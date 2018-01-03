
#ifdef _DEBUG
	#include <stdlib.h>
	#include <crtdbg.h>
	#define _CRTDBG_MAP_ALLOC
	#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
#else
	#define _CrtDumpMemoryLeaks() {}
#endif

#include <ctime>
#include <chrono>
#include <assert.h>
#include "Parser.h"
#include "Bot.h"
#include "Strategy.h"
#include "RandomStrategy.h"
#include "RandSearchStrategy.h"
#include "AdversarialStrategy.h"
#include "BirthRandSearch.h"
#include "Board.h"

using namespace std;

// returns the current time, measured in milliseconds
long get_time()
{
	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(
		chrono::system_clock::now().time_since_epoch());
	return ms.count();
}

void printArray(int **a, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			cout << a[i][j] << " ";
		}
		cout << "\n";
	}
	cout << "\n";
}

void test()
{
	int width = 18;
	int height = 16;
	RandomStrategy randomStrategy = RandomStrategy();

	for (int i = 0; i < 500; i++) {
		Board a = Board(width, height);
		a.initiateBoardPositions();
		Board *aCopy = a.getCopy();

		Player playerID = P0;
		Player enemyID = P1;
		if (i % 2 == 1) {
			playerID = P1;
			enemyID = P0;
		}
		Move m = randomStrategy.getMove(a, playerID, enemyID, 0, 0);

		Board *normalMove = a.makeMove(m, playerID);
		Board *nextRoundBoard = a.getNextRoundBoard();
		Board appliedMove = Board(width, height);
		a.applyMove(m, playerID, *nextRoundBoard, appliedMove);

		assert(*normalMove == appliedMove);
		assert(a == *aCopy);
		cout << "passed test " << i << "\n";
	}
}

void play() {
	AdversarialStrategy adversarialStrategy = AdversarialStrategy(10);
	Bot myBot = Bot(&adversarialStrategy);
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
	int timePerMove = 700;
	bot0.SetTimePerMove(timePerMove);
	bot1.SetTimePerMove(timePerMove);

	for (int round = 0; round < 100 && bot0Time >= 0 && bot1Time >= 0; round++) {
		if (verbose) cout << board.toString() << "\n";
		int P0Count = board.getPlayerCellCount(P0);
		int P1Count = board.getPlayerCellCount(P1);
		if (P0Count == 0 && P1Count == 0) return -1; // tie
		else if (P0Count == 0) return 1;
		else if (P1Count == 0) return 0;
		
		if (round % 2 == 0) {
			// player 0's turn
			bot0Time = min(bot0Time + timePerMove, 10000);
			if (verbose) cout << "bot 0 time: " << bot0Time << "\n";

			long startTime = get_time();
			Move move = bot0.GetMove(bot0Time);
			int dt = get_time() - startTime;
			bot0Time -= dt;

			if (verbose) cout << "Player 0's move: " << move.toString() << " time to compute: " << dt << "\n";
			board.makeMoveOnBoard(move, P0);
		}
		else {
			// player 1's turn
			bot1Time = min(bot1Time + timePerMove, 10000);
			if (verbose) cout << "bot 1 time: " << bot1Time << "\n";

			long startTime = get_time();
			Move move = bot1.GetMove(bot1Time);
			int dt = get_time() - startTime;
			bot1Time -= dt;

			if (verbose) cout << "Player 1's move: " << move.toString() << " time to compute: " << dt << "\n";
			board.makeMoveOnBoard(move, P1);
		}
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

void playTournament(Bot bot0, Bot bot1, int rounds = 100) {
	int bot0Wins = 0;
	int bot1Wins = 0;

	for (int currentRound = 1; currentRound <= rounds; currentRound++) {
		int matchResult;
		if (currentRound % 2 == 1) {
			matchResult = playMatch(bot0, bot1);
		}
		else {
			matchResult = 1 - playMatch(bot1, bot0);
			if (matchResult == 2) matchResult = -1; // ties should be -1
		}
		if (matchResult == 0) bot0Wins++;
		else if (matchResult == 1) bot1Wins++;
		cout << "match winner: " << matchResult << "\n";
		if (currentRound % 10 == 0) {
			int ties = currentRound - bot0Wins - bot1Wins;
			cout << "bot 0 wins: " << bot0Wins << "\nbot 1 wins: " << bot1Wins << "\nties: " << ties << "\n";
		}
	}

	int ties = rounds - bot0Wins - bot1Wins;
	cout << "bot 0 wins: " << bot0Wins << "\nbot 1 wins: " << bot1Wins << "\nties: " << ties << "\n";
}

void playTest() {
	AdversarialStrategy bot0Strategy = AdversarialStrategy(1);
	Bot bot0 = Bot(&bot0Strategy);

	AdversarialStrategy bot1Strategy = AdversarialStrategy(3);
	Bot bot1 = Bot(&bot1Strategy);

	playTournament(bot0, bot1, 100);
	//playMatch(bot0, bot1, true);
}

int main()
{
	// Initialize random number generator
	srand(time(NULL));
	  
	playTest();
	//test();
	//play();

	_CrtDumpMemoryLeaks();
}