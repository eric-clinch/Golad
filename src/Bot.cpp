#include "Bot.h"
using namespace std;

Bot::Bot(Strategy *strategy)
{
  this->board = NULL;
  this->strategy = strategy;
  this->timePerMove = 100;
}

template <class T>
T Bot::RandomElementFromVector(vector<T> *input)
{
  random_shuffle(input->begin(), input->end());
  T temp = input->back();
  input->pop_back();
  return temp;
}

void Bot::setStrategy(Strategy *strategy) {
	this->strategy = strategy;
}

Move Bot::GetMove(int time, int round)
{
	assert(board != NULL);
	return strategy->getMove(*board, playerID, enemyID, time, timePerMove, round);
}

void Bot::SetRound(int round){
	this->currentRound = round;
};

int Bot::GetRound() {
	return this->currentRound;
}

Board* Bot::GetBoard()
{
	return board;
}

void Bot::SetBoard(Board *board)
{
	this->board = board;
}

void Bot::SetTimebank(int time){};

void Bot::SetTimePerMove(int time){
	this->timePerMove = time;
};

void Bot::SetYourBotName(string name)
{
	playerName = name;
}

string Bot::GetYourBotName()
{
	return playerName;
}

void Bot::SetYourBotId(Player playerID)
{
	this->playerID = playerID;
	if (playerID == P0) {
		enemyID = P1;
	}
	else {
		enemyID = P0;
	}
}

void Bot::SetFieldWidth(int width)
{
	this->width = width;
}

void Bot::SetFieldHeight(int height)
{
	this->height = height;
}

int Bot::GetFieldWidth()
{
  return width;
}

int Bot::GetFieldHeight()
{
  return height;
}

void Bot::SetPlayerNames(string player1, string player2){};

void Bot::SetMaxRounds(int maxRounds)
{
	this->maxRounds = maxRounds;
}

int Bot::GetMaxRounds()
{
  return this->maxRounds;
}

void Bot::SetMyLivingCellCount(int livingCellCount) {}

int Bot::GetMyLivingCellCount()
{
	return this->board->getPlayerCellCount(playerID);
}

void Bot::SetEnemyLivingCellCount(int livingCellCount) {}

int Bot::GetEnemyLivingCellCount()
{
	return board->getPlayerCellCount(enemyID);
}

vector<MoveType>* Bot::GetAvailableMoveTypes()
{
	  vector<MoveType>* availableMoves = new vector<MoveType>;

	  if (this->GetMyLivingCellCount() + this->GetEnemyLivingCellCount() > 0) {
			availableMoves->push_back(KILL);
	  }

	  if (this->GetMyLivingCellCount() > 1) {
			availableMoves->push_back(BIRTH);
	  }

	  return availableMoves;
}