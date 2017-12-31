#include "Board.h"

using namespace std;

inline char Board::getNextCellStatus(int cellX, int cellY) {
	int player0Neighbors = 0;
	int player1Neighbors = 0;
	for (int dx = -1; dx <= 1; dx++) {
		for (int dy = -1; dy <= 1; dy++) {
			if (dx == 0 && dy == 0) continue;
			int x = cellX + dx;
			int y = cellY + dy;
			if (x < 0 || x >= width || y < 0 || y >= height) continue;
			if (this->board[x][y] == '0') player0Neighbors++;
			else if (this->board[x][y] == '1') player1Neighbors++;
		}
	}

	char currentStatus = board[cellX][cellY];
	int totalNeighbors = player0Neighbors + player1Neighbors;

	if (currentStatus == '.') {
		if (totalNeighbors == 3) {
			// the cell spawns for whoever has the most neighbors
			return player0Neighbors > player1Neighbors ? '0' : '1';
		}
		else {
			// dead cells stay dead unless they have exactly 3 neighbors
			return '.';
		}
	}
	else if (totalNeighbors == 2 || totalNeighbors == 3) {
		// the cell continues to live if it has 2 or 3 neighbors
		return currentStatus;
	}
	else return '.';
}

inline void Board::updateCellStatus(int x, int y, Board &lastRoundBoard) {
	char currentStatus = board[x][y];
	if (currentStatus == '0') P0CellCount--;
	else if (currentStatus == '1') P1CellCount--;

	char updatedStatus = lastRoundBoard.getNextCellStatus(x, y);
	board[x][y] = updatedStatus;
	if (updatedStatus == '0') P0CellCount++;
	else if (updatedStatus == '1') P1CellCount++;
}

inline void Board::updateRegionStatus(int cellX, int cellY, Board &lastRoundBoard) {
	for (int dx = -1; dx <= 1; dx++) {
		for (int dy = -1; dy <= 1; dy++) {
			int x = cellX + dx;
			int y = cellY + dy;
			if (x < 0 || x >= width || y < 0 || y >= height) continue;
			updateCellStatus(x, y, lastRoundBoard);
		}
	}
}

// runs the next round of the Game of Life simulation, updating the board as well
// as the cell counts for each player
inline void Board::nextRound() {
	char **newBoard = new char *[width];
	for (int i = 0; i < width; i++) {
		newBoard[i] = new char[height];
	}

	P0CellCount = P1CellCount = 0;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			char newStatus = getNextCellStatus(x, y);
			if (newStatus == '0') P0CellCount++;
			else if (newStatus == '1') P1CellCount++;
			newBoard[x][y] = newStatus;
		}
	}

	deleteBoard();
	board = newBoard;
}

Board::Board(int width, int height)
{
	this->width = width;
	this->height = height;
	board = new char*[width];
	for (int i = 0; i < width; ++i) {
		board[i] = new char[height];
	}
}

Board::~Board() {
	deleteBoard();
}

bool Board::operator== (Board &other) {
	if (width != other.width || height != other.height) return false;
	if (P0CellCount != other.P0CellCount || P1CellCount != other.P1CellCount) return false;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (board[x][y] != other.board[x][y]) return false;
		}
	}
	return true;
}

void Board::UpdateBoard(stringstream &stream) {
	int x = 0, y = 0;
	string field;
	while (getline(stream, field, ',')) {
		board[x][y] = field.at(0);
		x = (x + 1) % this->width;
		if (x == 0) y++;
	}
}

void Board::initiateBoardPositions(int aliveCells) {
	// set all cells to dead
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			board[x][y] = '.';
		}
	}

	// set cells alive randomly and rotationally symmetrically
	vector<Coordinate> positions;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height / 2; y++) {
			positions.push_back(Coordinate(x, y));
		}
	}

	assert(positions.size() >= (size_t) aliveCells);
	random_shuffle(positions.begin(), positions.end());
	for (int i = 0; i < aliveCells; i++) {
		Coordinate c = positions.back();
		positions.pop_back();

		// place live cells rotationally symmetrically
		board[c.x][c.y] = '0';
		board[width - c.x - 1][height - c.y - 1] = '1';
	}
	P0CellCount = P1CellCount = aliveCells;
}

Board* Board::getCopy() {
	Board *result = new Board(width, height);
	copyBoard(result->board);
	result->P0CellCount = P0CellCount;
	result->P1CellCount = P1CellCount;
	assert(*result == *this);
	return result;
}

void Board::setPlayerCellCount(Player playerID, int cellCount) {
	if (playerID == P0) {
		P0CellCount = cellCount;
	}
	else {
		P1CellCount = cellCount;
	}
}

int Board::getPlayerCellCount(Player playerID) {
	if (playerID == P0) {
		return P0CellCount;
	}
	else {
		return P1CellCount;
	}
}

vector<Coordinate> Board::GetCells(char type) {
	vector<Coordinate> selectedCells;
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			if (board[x][y] == type) {
				selectedCells.push_back(Coordinate(x, y));
			}
		}
	}
	return selectedCells;
}

inline void Board::deleteBoard() {
	for (int i = 0; i < width; i++) {
		delete[] board[i];
	}
	delete[] board;
}

inline void Board::copyBoard(char **blankBoard) {
	for (int x = 0; x < width; x++) {
		copy(board[x], board[x] + height, blankBoard[x]);
	}
}

Board* Board::makeMove(Move &move, Player playerID) {
	Board *result = new Board(width, height);
	copyBoard(result->board);
	result->makeMoveOnBoard(move, playerID);
	return result;
}

void Board::makeMoveOnBoard(Move &move, Player playerID) {
	if (move.MoveType == KILL) {
		assert(board[move.target.x][move.target.y] != '.');
		board[move.target.x][move.target.y] = '.';
	}
	else if (move.MoveType == BIRTH) {
		char playerChar = to_string(playerID).at(0);
		assert(board[move.sacrifice1.x][move.sacrifice1.y] == board[move.sacrifice2.x][move.sacrifice2.y]);
		assert(board[move.sacrifice2.x][move.sacrifice2.y] == playerChar);
		assert(board[move.target.x][move.target.y] == '.');
		assert(move.sacrifice1.x != move.sacrifice2.x || move.sacrifice1.y != move.sacrifice2.y);
		board[move.sacrifice1.x][move.sacrifice1.y] = '.';
		board[move.sacrifice2.x][move.sacrifice2.y] = '.';
		board[move.target.x][move.target.y] = playerChar;
	}

	// this call is necessary to update the cell counts, as well as running the simulation
	nextRound();
}

// applies a move to a board as if the move was made on the last round of play.
// the following expressions are equivalent for any given Board originalBoard,
// Move move, and Player playerID:
// 1. originalBoard.makeMove(move, playerID);
// 2. originalBoard.applyMove(move, playerID, originalBoard.makeMove(Move(), playerID));
// Note that originalBoard.makeMove(Move(), playerID) is making the pass move on the original
// board, which is equivalent to just computing the next round of the Game Of Life simulation.
// this should be used to minimize recomputation when applying different moves
// to the same board.
Board* Board::applyMove(Move &move, Player playerID, Board &nextRoundBoard) {
	Board *result = nextRoundBoard.getCopy();

	if (move.MoveType == KILL) {
		char targetChar = board[move.target.x][move.target.y];
		assert(targetChar != '.');
		board[move.target.x][move.target.y] = '.';

		result->updateRegionStatus(move.target.x, move.target.y, *this);

		board[move.target.x][move.target.y] = targetChar;
	}
	else if (move.MoveType == BIRTH) {
		char playerChar = to_string(playerID).at(0);
		assert(board[move.sacrifice1.x][move.sacrifice1.y] == board[move.sacrifice2.x][move.sacrifice2.y]);
		assert(board[move.sacrifice2.x][move.sacrifice2.y] == playerChar);
		assert(board[move.target.x][move.target.y] == '.');
		assert(move.sacrifice1.x != move.sacrifice2.x || move.sacrifice1.y != move.sacrifice2.y);
		board[move.sacrifice1.x][move.sacrifice1.y] = '.';
		board[move.sacrifice2.x][move.sacrifice2.y] = '.';
		board[move.target.x][move.target.y] = playerChar;

		result->updateRegionStatus(move.sacrifice1.x, move.sacrifice1.y, *this);
		result->updateRegionStatus(move.sacrifice2.x, move.sacrifice2.y, *this);
		result->updateRegionStatus(move.target.x, move.target.y, *this);

		board[move.sacrifice1.x][move.sacrifice1.y] = playerChar;
		board[move.sacrifice2.x][move.sacrifice2.y] = playerChar;
		board[move.target.x][move.target.y] = '.';
	}

	return result;
}

string Board::toString() {
	ostringstream result;
	result << "Player 0: " << P0CellCount << " Player 1: " << P1CellCount << "\n";
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			result << board[x][y] << " ";
		}
		result << "\n";
	}
	return result.str();
}