#include "Board.h"

// ideas for efficient computation of Conway's Game Of Life taken from here:
// https://codereview.stackexchange.com/questions/42718/optimize-conways-game-of-life
// note that because cells can either be dead, player 0's, or player 1's, base 3 is needed
// rather than base 2.

using namespace std;

// produces a unique index for each configuration of the
// given grid, assuming the grid is 3x3 and its only elements
// are '0', '1', and ' '. Uses a base 3 system in order to 
// encode the status of each cell in the grid into the index
// and produce a unique index for each grid state.
// neighbor position significance:
// 0 3 6
// 1 4 7
// 2 5 8
inline int getLookupTableIndex(char **grid) {
	int significance = 1;
	int result = 0;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			if (grid[x][y] == '0') {
				result += significance;
			}
			else if (grid[x][y] == '1') {
				result += 2 * significance;
			}

			significance *= 3;
		}
	}
	return result;
}

void getLookupTableHelper(char *lookupTable, char **grid, int x, int y, int bot0Neighbors, int bot1Neighbors) {
	if (y > 2) {
		// the grid is full, make a corresponding entry into the lookup table
		int index = getLookupTableIndex(grid);
		assert(lookupTable[index] == ' ');
		assert(0 <= index);
		assert(index < 19683);

		// make sure not to account for the middle cell in the neighbor count
		char currentCellStatus = grid[1][1];
		if (currentCellStatus == '0') bot0Neighbors--;
		else if (currentCellStatus == '1') bot1Neighbors--;

		char nextCellStatus = '.';
		int totalNeighbors = bot0Neighbors + bot1Neighbors;
		if (currentCellStatus == '.') {
			if (totalNeighbors == 3) {
				nextCellStatus = bot0Neighbors > bot1Neighbors ? '0' : '1';
			}
		}
		else if (totalNeighbors == 2 || totalNeighbors == 3) {
			nextCellStatus = currentCellStatus;
		}

		lookupTable[index] = nextCellStatus;
	}

	else {
		// fill the grid
		int nextX = x + 1;
		int nextY = y;
		if (nextX > 2) {
			nextX = 0;
			nextY = y + 1;
		}
		grid[x][y] = '.';
		getLookupTableHelper(lookupTable, grid, nextX, nextY, bot0Neighbors, bot1Neighbors);
		grid[x][y] = '0';
		getLookupTableHelper(lookupTable, grid, nextX, nextY, bot0Neighbors + 1, bot1Neighbors);
		grid[x][y] = '1';
		getLookupTableHelper(lookupTable, grid, nextX, nextY, bot0Neighbors, bot1Neighbors + 1);
	}
}

char* getLookupTable() {
	int size = pow(3, 9);
	char *lookupTable = new char[size];
	for (int i = 0; i < size; i++) lookupTable[i] = ' ';

	char **grid = new char*[3];
	for (int i = 0; i < 3; i++) {
		grid[i] = new char[3];
		for (int j = 0; j < 3; j++) {
			grid[i][j] = '.';
		}
	}

	getLookupTableHelper(lookupTable, grid, 0, 0, 0, 0);

	for (int i = 0; i < 3; i++) delete grid[i];
	delete grid;

	return lookupTable;
}

char* Board::simulationLookupTable = getLookupTable();

// significance of each cell:
// 1  27  729
// 3  81  2187
// 9  243 6561
int Board::getCellIndex(int cellX, int cellY) {
	assert(0 < cellX && cellX <= width);
	assert(0 < cellY && cellY <= height);

	int index = 0;
	char *column = board[cellX - 1];

	char c = column[cellY - 1];
	if (c == '0') index += 1;
	else if (c == '1') index += 2;

	c = column[cellY];
	if (c == '0') index += 3;
	else if (c == '1') index += 6;

	c = column[cellY + 1];
	if (c == '0') index += 9;
	else if (c == '1') index += 18;

	column = board[cellX];

	c = column[cellY - 1];
	if (c == '0') index += 27;
	else if (c == '1') index += 54;

	c = column[cellY];
	if (c == '0') index += 81;
	else if (c == '1') index += 162;

	c = column[cellY + 1];
	if (c == '0') index += 243;
	else if (c == '1') index += 486;

	column = board[cellX + 1];

	c = column[cellY - 1];
	if (c == '0') index += 729;
	else if (c == '1') index += 1458;

	c = column[cellY];
	if (c == '0') index += 2187;
	else if (c == '1') index += 4374;

	c = column[cellY + 1];
	if (c == '0') index += 6561;
	else if (c == '1') index += 13122;

	return index;
}

inline char Board::getNextCellStatus(int cellX, int cellY) {
	assert(0 < cellX && cellX <= width);
	assert(0 < cellY && cellY <= height);
	return simulationLookupTable[getCellIndex(cellX, cellY)];
}

inline void Board::updateCellStatus(int x, int y, int index) {
	assert(0 < x && x <= width);
	assert(0 < y && y <= height);

	char currentStatus = board[x][y];
	if (currentStatus == '0') P0CellCount--;
	else if (currentStatus == '1') P1CellCount--;

	char updatedStatus = simulationLookupTable[index];
	board[x][y] = updatedStatus;
	if (updatedStatus == '0') P0CellCount++;
	else if (updatedStatus == '1') P1CellCount++;
}

inline void Board::updateRegionStatus(int cellX, int cellY, Board &lastRoundBoard) {
	assert(0 < cellX && cellX <= width);
	assert(0 < cellY && cellY <= height);

	int minX = max(cellX - 1, 1);
	int maxX = min(cellX + 1, width);
	int minY = max(cellY - 1, 1);
	int maxY = min(cellY + 1, height);

	for (int y = minY; y <= maxY; y++) {
		int cellIndex = lastRoundBoard.getCellIndex(minX, y);
		updateCellStatus(minX, y, cellIndex);
		for (int x = minX + 1; x <= maxX; x++) {
			cellIndex /= 27;

			char c = lastRoundBoard.board[x + 1][y - 1];
			if (c == '0') cellIndex += 729;
			else if (c == '1') cellIndex += 1458;

			c = lastRoundBoard.board[x + 1][y];
			if (c == '0') cellIndex += 2187;
			else if (c == '1') cellIndex += 4374;

			c = lastRoundBoard.board[x + 1][y + 1];
			if (c == '0') cellIndex += 6561;
			else if (c == '1') cellIndex += 13122;

			assert(cellIndex == lastRoundBoard.getCellIndex(x, y));

			updateCellStatus(x, y, cellIndex);
		}
	}
}

// runs the next round of the Game of Life simulation, updating the board as well
// as the cell counts for each player
inline void Board::nextRound() {
	char **newBoard = new char *[__width__];
	for (int i = 0; i < __width__; i++) {
		newBoard[i] = new char[__height__];
	}

	// fill in the outer edge
	for (int x = 0; x < __width__; x++) {
		newBoard[x][0] = '.';
		newBoard[x][__height__ - 1] = '.';
	}
	for (int y = 1; y <= height; y++) {
		newBoard[0][y] = '.';
		newBoard[__width__ - 1][y] = '.';
	}

	P0CellCount = P1CellCount = 0;
	for (int y = 1; y <= height; y++) {
		int cellIndex = getCellIndex(1, y);
		char newStatus = simulationLookupTable[cellIndex];
		if (newStatus == '0') P0CellCount++;
		else if (newStatus == '1') P1CellCount++;
		newBoard[1][y] = newStatus;

		for (int x = 2; x <= width; x++) {
			cellIndex /= 27;

			char c = board[x + 1][y - 1];
			if (c == '0') cellIndex += 729;
			else if (c == '1') cellIndex += 1458;

			c = board[x + 1][y];
			if (c == '0') cellIndex += 2187;
			else if (c == '1') cellIndex += 4374;

			c = board[x + 1][y + 1];
			if (c == '0') cellIndex += 6561;
			else if (c == '1') cellIndex += 13122;

			assert(cellIndex == getCellIndex(x, y));

			char newStatus = simulationLookupTable[cellIndex];
			if (newStatus == '0') P0CellCount++;
			else if (newStatus == '1') P1CellCount++;
			newBoard[x][y] = newStatus;
		}
	}

	deleteBoard();
	board = newBoard;
}

void Board::setNextRoundBoard(Board &result) {
	result.P0CellCount = result.P1CellCount = 0;
	for (int y = 1; y <= height; y++) {
		int cellIndex = getCellIndex(1, y);
		char newStatus = simulationLookupTable[cellIndex];
		if (newStatus == '0') result.P0CellCount++;
		else if (newStatus == '1') result.P1CellCount++;
		result.board[1][y] = newStatus;

		for (int x = 2; x <= width; x++) {
			cellIndex /= 27;

			char c = board[x + 1][y - 1];
			if (c == '0') cellIndex += 729;
			else if (c == '1') cellIndex += 1458;

			c = board[x + 1][y];
			if (c == '0') cellIndex += 2187;
			else if (c == '1') cellIndex += 4374;

			c = board[x + 1][y + 1];
			if (c == '0') cellIndex += 6561;
			else if (c == '1') cellIndex += 13122;

			assert(cellIndex == getCellIndex(x, y));

			char newStatus = simulationLookupTable[cellIndex];
			if (newStatus == '0') result.P0CellCount++;
			else if (newStatus == '1') result.P1CellCount++;
			result.board[x][y] = newStatus;
		}
	}
}

Board* Board::getNextRoundBoard() {
	Board *result = new Board(width, height);

	result->P0CellCount = result->P1CellCount = 0;
	for (int y = 1; y <= height; y++) {
		int cellIndex = getCellIndex(1, y);
		char newStatus = simulationLookupTable[cellIndex];
		if (newStatus == '0') result->P0CellCount++;
		else if (newStatus == '1') result->P1CellCount++;
		result->board[1][y] = newStatus;

		for (int x = 2; x <= width; x++) {
			cellIndex /= 27;

			char c = board[x + 1][y - 1];
			if (c == '0') cellIndex += 729;
			else if (c == '1') cellIndex += 1458;

			c = board[x + 1][y];
			if (c == '0') cellIndex += 2187;
			else if (c == '1') cellIndex += 4374;

			c = board[x + 1][y + 1];
			if (c == '0') cellIndex += 6561;
			else if (c == '1') cellIndex += 13122;

			assert(cellIndex == getCellIndex(x, y));

			char newStatus = simulationLookupTable[cellIndex];
			if (newStatus == '0') result->P0CellCount++;
			else if (newStatus == '1') result->P1CellCount++;
			result->board[x][y] = newStatus;
		}
	}

	return result;
}

Board::Board() {}

Board::Board(int width, int height) : width(width), height(height), __width__(width + 2), __height__(height + 2), board(new char*[this->__width__]),
									  P0CellCount(0), P1CellCount(0)
{
	for (int i = 0; i < __width__; i++) {
		board[i] = new char[__height__];
	}

	// fill in the outer edge
	for (int x = 0; x < __width__; x++) {
		board[x][0] = '.';
		board[x][__height__ - 1] = '.';
	}
	for (int y = 1; y <= height; y++) {
		board[0][y] = '.';
		board[__width__ - 1][y] = '.';
	}
}

Board::~Board() {
	deleteBoard();
}

bool Board::operator== (Board &other) {
	if (width != other.width || height != other.height) {
		return false;
	}
	if (__width__ != other.__width__ || __height__ != other.__height__) { 
		return false; 
	}
	if (P0CellCount != other.P0CellCount || P1CellCount != other.P1CellCount) {
		return false;
	}
	for (int x = 0; x < __width__; x++) {
		for (int y = 0; y < __height__; y++) {
			if (board[x][y] != other.board[x][y]) {
				return false;
			}
		}
	}
	return true;
}

void Board::UpdateBoard(stringstream &stream) {
	int x = 1, y = 1;
	string field;
	while (getline(stream, field, ',')) {
		board[x][y] = field.at(0);
		x++;
		if (x == width + 1) {
			x = 1;
			y++;
		}
	}
}

void Board::initiateBoardPositions(int aliveCells) {
	// set all cells to dead
	for (int x = 0; x < __width__; x++) {
		for (int y = 0; y < __height__; y++) {
			board[x][y] = '.';
		}
	}

	// set cells alive randomly and rotationally symmetrically
	// don't set any of the cells on the outer edge as alive
	vector<Coordinate> positions;
	for (int x = 1; x <= width; x++) {
		for (int y = 1; y <= height / 2; y++) {
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
		board[__width__ - c.x - 1][__height__ - c.y - 1] = '1';
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

int Board::getWidth() {
	return width;
}

int Board::getHeight() {
	return height;
}

// returns the coordinates of every cell that has the given type.
// note that the board is internally represented as a 2D array with
// dimensions (width+2) by (height+2), with the outer edge of the array
// being ignored. This function will not consider coordinates in the
// outer edge and will return coordinates that are not offset by this
// outer edge (ie the coordinates will be one less than their actual
// indices on the internally represented board, but will look correct
// to an outside observer who doesn't know about this outer edge).
vector<Coordinate> Board::GetCells(char type) {
	vector<Coordinate> selectedCells;
	for (int x = 1; x <= width; ++x) {
		for (int y = 1; y <= height; ++y) {
			if (board[x][y] == type) {
				selectedCells.push_back(Coordinate(x - 1, y - 1));
			}
		}
	}
	return selectedCells;
}

inline void Board::deleteBoard() {
	for (int i = 0; i < __width__; i++) {
		delete[] board[i];
	}
	delete[] board;
}

inline void Board::copyBoard(char **blankBoard) {
	for (int x = 0; x < __width__; x++) {
		copy(board[x], board[x] + __height__, blankBoard[x]);
	}
}

inline void Board::copyBoard(Board &blankBoard) {
	assert(blankBoard.width == width && blankBoard.height == height);
	assert(blankBoard.__width__ == __width__ && blankBoard.__height__ == __height__);
	copyBoard(blankBoard.board);
	blankBoard.P0CellCount = P0CellCount;
	blankBoard.P1CellCount = P1CellCount;
	assert(*this == blankBoard);
}

bool Board::isLegal(Move &move, Player playerID) {
	if (move.MoveType == KILL) {
		return board[move.target.x + 1][move.target.y + 1] != '.';
	}
	else if (move.MoveType == BIRTH) {
		char playerChar = to_string(playerID).at(0);
		return (board[move.sacrifice1.x + 1][move.sacrifice1.y + 1] == board[move.sacrifice2.x + 1][move.sacrifice2.y + 1] &&
		        board[move.sacrifice2.x + 1][move.sacrifice2.y + 1] == playerChar &&
				board[move.target.x + 1][move.target.y + 1] == '.' &&
				(move.sacrifice1.x != move.sacrifice2.x || move.sacrifice1.y != move.sacrifice2.y));
	}
	else return true;
}

Board* Board::makeMove(Move &move, Player playerID) {
	Board *result = new Board(width, height);
	copyBoard(result->board);
	result->makeMoveOnBoard(move, playerID);
	return result;
}

void Board::makeMove(Move &move, Player playerID, Board &result) {
	assert(isLegal(move, playerID));

	if (move.MoveType == KILL) {
		int targetX = move.target.x + 1;
		int targetY = move.target.y + 1;

		char targetChar = board[targetX][targetY];
		board[targetX][targetY] = '.';

		setNextRoundBoard(result);

		board[targetX][targetY] = targetChar;
	}
	else if (move.MoveType == BIRTH) {
		int sacrifice1X = move.sacrifice1.x + 1;
		int sacrifice1Y = move.sacrifice1.y + 1;

		int sacrifice2X = move.sacrifice2.x + 1;
		int sacrifice2Y = move.sacrifice2.y + 1;

		int targetX = move.target.x + 1;
		int targetY = move.target.y + 1;

		char playerChar = to_string(playerID).at(0);
		board[sacrifice1X][sacrifice1Y] = '.';
		board[sacrifice2X][sacrifice2Y] = '.';
		board[targetX][targetY] = playerChar;

		setNextRoundBoard(result);

		board[sacrifice1X][sacrifice1Y] = playerChar;
		board[sacrifice2X][sacrifice2Y] = playerChar;
		board[targetX][targetY] = '.';
	}
	else {
		setNextRoundBoard(result);
	}
}

void Board::makeMoveOnBoard(Move &move, Player playerID) {
	assert(isLegal(move, playerID));
	if (move.MoveType == KILL) {
		board[move.target.x + 1][move.target.y + 1] = '.';
	}
	else if (move.MoveType == BIRTH) {
		char playerChar = to_string(playerID).at(0);
		board[move.sacrifice1.x + 1][move.sacrifice1.y + 1] = '.';
		board[move.sacrifice2.x + 1][move.sacrifice2.y + 1] = '.';
		board[move.target.x + 1][move.target.y + 1] = playerChar;
	}

	// this call is necessary to update the cell counts, as well as running the simulation
	nextRound();
}

// applies a move to a board as if the move was made on the last round of play, and copies the result
// into the given Board result so that following expression would be equal to the Board result for any
// given Move move, and Player playerID:
//     originalBoard.makeMove(move, playerID);
// Note that originalBoard.makeMove(Move(), playerID) is making the pass move on the original
// board, which is equivalent to just computing the next round of the Game Of Life simulation. So it
// should be used as the nextRoundBoard parameter.
// this should be used to minimize recomputation when applying different moves
// to the same board.
void Board::applyMove(Move &move, Player playerID, Board &nextRoundBoard, Board &result) {
	assert(isLegal(move, playerID));
	nextRoundBoard.copyBoard(result);

	if (move.MoveType == KILL) {
		int targetX = move.target.x + 1;
		int targetY = move.target.y + 1;

		char targetChar = board[targetX][targetY];
		board[targetX][targetY] = '.';

		result.updateRegionStatus(targetX, targetY, *this);

		board[targetX][targetY] = targetChar;
	}
	else if (move.MoveType == BIRTH) {
		int sacrifice1X = move.sacrifice1.x + 1;
		int sacrifice1Y = move.sacrifice1.y + 1;

		int sacrifice2X = move.sacrifice2.x + 1;
		int sacrifice2Y = move.sacrifice2.y + 1;

		int targetX = move.target.x + 1;
		int targetY = move.target.y + 1;

		char playerChar = to_string(playerID).at(0);
		board[sacrifice1X][sacrifice1Y] = '.';
		board[sacrifice2X][sacrifice2Y] = '.';
		board[targetX][targetY] = playerChar;

		result.updateRegionStatus(sacrifice1X, sacrifice1Y, *this);
		result.updateRegionStatus(sacrifice2X, sacrifice2Y, *this);
		result.updateRegionStatus(targetX, targetY, *this);

		board[sacrifice1X][sacrifice1Y] = playerChar;
		board[sacrifice2X][sacrifice2Y] = playerChar;
		board[targetX][targetY] = '.';
	}
}

string Board::toString(bool showBorder) {
	ostringstream result;
	result << "Player 0: " << P0CellCount << " Player 1: " << P1CellCount << "\n";
	if (showBorder) {
		for (int y = 0; y < __height__; y++) {
			for (int x = 0; x < __width__; x++) {
				result << board[x][y] << " ";
			}
			result << "\n";
		}
	}
	else {
		for (int y = 1; y <= height; y++) {
			for (int x = 1; x <= width; x++) {
				result << board[x][y] << " ";
			}
			result << "\n";
		}
	}
	return result.str();
}