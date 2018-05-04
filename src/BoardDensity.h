
#ifndef  BOARDDENSITY_H
#define BOARDDENSITY_H

#include "Board.h"

class BoardDensity {
public:
	static float getBoardDensity(Board &board, Player player) {
		vector<Coordinate> playerCells = board.GetCells(player);
		if (playerCells.size() == 0) return 0;
		float densitySum = 0;
		for (int i = 0; i < playerCells.size(); i++) {
			Coordinate c = playerCells[i];
			densitySum += getCoordinateDensity(board, c.x, c.y);
		}
		return densitySum / playerCells.size();
	}

	static float getCoordinateDensity(Board &board, int x, int y) {
		float density = 0;
		int width = board.getWidth();
		int height = board.getHeight();
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				int newX = x + dx;
				int newY = y + dy;
				if (0 <= newX && newX < width && 0 <= newY && newY < height) {
					char cellStatus = board.getCoordinateType(newX, newY);
					if (cellStatus != Board::EMPTY) density += 1;
				}
			}
		}
		return density;
	}

	static float getPlayerCoordinateDensity(Board &board, int x, int y, Player playerID) {
		float density = 0;
		int width = board.getWidth();
		int height = board.getHeight();
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				int newX = x + dx;
				int newY = y + dy;
				if (0 <= newX && newX < width && 0 <= newY && newY < height) {
					char cellStatus = board.getCoordinateType(newX, newY);
					if (cellStatus == playerID) density += 1;
				}
			}
		}
		return density;
	}
};

#endif
