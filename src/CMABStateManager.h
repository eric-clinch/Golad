#ifndef CMABStateManager_h
#define CMABStateManager_h

using namespace std;

class CMABState;

class CMABStateManager {
public:

	vector<CMABState*> *states;
	CMABState *rootNode;
	int currentIndex;

	CMABStateManager() {}

	CMABStateManager(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *moveMAB, float nonRootGreed,
		Player playerID, Player enemyID) {
		this->rootNode = new CMABState(board, evaluator, coordinateMAB, moveMAB, nonRootGreed, playerID, enemyID, this);
		this->states = new vector<CMABState*>();
		this->currentIndex = 0;
	}

	void newRound(Board &board, Player playerID, Player enemyID) {
		rootNode->repurposeNode(board, playerID, enemyID);
		currentIndex = 0;
	}

	CMABState *getState(Board &board, CMABState::SharedData *sharedData, Player playerID, Player enemyID) {
		if (currentIndex < states->size()) {
			CMABState *result = (*states)[currentIndex];
			result->repurposeNode(board, playerID, enemyID);
			currentIndex++;
			return result;
		}
		else {
			CMABState *result = new CMABState(board, sharedData, playerID, enemyID);
			states->push_back(result);
			currentIndex++;
			return result;
		}
	}

	~CMABStateManager() {
		for (int i = 0; i < states->size(); i++) {
			delete (*states)[i];
		}
		delete states;
		rootNode->freeShared();
		delete rootNode;
	}
};

#endif