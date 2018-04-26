
#ifndef STRATEGYTESTING_h
#define STRATEGYTESTING_h

#include <random>
#include "Bot.h"
#include "Tools.h"
#include "Stats.h"
#include "UtilityNode.h"
#include "MAB.h"
#include "UCB1.h"
#include "UCBHybrid.h"
#include "EpsilonGreedy.h"
#include "Evaluator.h"
#include "RatioEvaluator.h"

namespace StrategyTesting {
	int playMatch(Bot bot0, Bot bot1, bool verbose = false);
	void playTournament(Bot bot0, Bot bot1, int rounds = 100, bool verbose = false);
}

int StrategyTesting::playMatch(Bot bot0, Bot bot1, bool verbose) {
	Board board(18, 16);
	board.initiateBoardPositions();
	bot0.SetBoard(&board);
	bot1.SetBoard(&board);
	bot0.SetYourBotId(P0);
	bot1.SetYourBotId(P1);

	float serverRatio = .5;
	int bot0Time = 10000 * serverRatio;
	int bot1Time = 10000 * serverRatio;
	int timePerMove = 100 * serverRatio;
	bot0.SetTimePerMove(timePerMove);
	bot1.SetTimePerMove(timePerMove);

	for (int round = 0; round < 50 && bot0Time >= 0 && bot1Time >= 0; round++) {
		if (verbose) cout << board.toString();
		int P0Count = board.getPlayerCellCount(P0);
		int P1Count = board.getPlayerCellCount(P1);
		if (P0Count == 0 || P1Count == 0) break;
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

		delete boardCopy;
		if (verbose) cout << board.toString();
		P0Count = board.getPlayerCellCount(P0);
		P1Count = board.getPlayerCellCount(P1);
		if (P0Count == 0 || P1Count == 0) break;
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

	bot0.cleanUp();
	bot1.cleanUp();

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

void StrategyTesting::playTournament(Bot bot0, Bot bot1, int rounds, bool verbose) {
	int bot0Wins = 0;
	int bot1Wins = 0;

	for (int currentRound = 1; currentRound <= rounds; currentRound++) {
		int matchResult;
		if (currentRound % 2 == 1) {
			matchResult = StrategyTesting::playMatch(bot0, bot1, verbose);
		}
		else {
			matchResult = 1 - StrategyTesting::playMatch(bot1, bot0, verbose);
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
				if (probabilityBot0IsBetter > .99 || probabilityBot1IsBetter > .99) break; // the tournament is complete at this point
			}
		}
	}
}

#endif