#ifndef BOT_h
#define BOT_h

#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "Board.h"
#include "Enums.h"
#include "Coordinate.h"
#include "Move.h"
#include "Strategy.h"

using namespace std;

class Bot
{
private:
	Player playerID;
	Player enemyID;
	int width, height;
	int maxRounds;
	int currentRound;
	int timePerMove;
	Board *board;
	string playerName;
	Strategy *strategy;

protected:

public:
	Bot(Strategy *strategy);

	virtual void setStrategy(Strategy *strategy);

	// Action
	virtual Move GetMove(int time, int round);

	// Update
	virtual void SetRound(int round);
	virtual int GetRound();
	virtual Board* GetBoard();
	virtual void SetBoard(Board *board);

	// Settings
	virtual void SetTimebank(int time);
	virtual void SetTimePerMove(int time);

	virtual void SetYourBotName(string name);
	virtual string GetYourBotName();

	virtual void SetYourBotId(Player playerId);

	virtual int GetFieldWidth();
	virtual void SetFieldWidth(int width);

	virtual int GetFieldHeight();
	virtual void SetFieldHeight(int height);

	virtual void SetPlayerNames(string player1, string player2);

	virtual int GetMaxRounds();
	virtual void SetMaxRounds(int maxRounds);

	virtual void SetMyLivingCellCount(int livingCellCount);
	virtual int GetMyLivingCellCount();

	virtual void SetEnemyLivingCellCount(int livingCellCount);
	virtual int GetEnemyLivingCellCount();

	virtual vector<MoveType>* GetAvailableMoveTypes();

	template <class T>
	T RandomElementFromVector(vector<T>* input);
};

#endif
