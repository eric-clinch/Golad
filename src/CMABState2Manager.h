#ifndef CMABState2Manager_h
#define CMABState2Manager_h

using namespace std;

class CMABState2;

class CMABState2Manager {
public:

	vector<CMABState2*> *states;
	CMABState2 *rootNode;
	int currentIndex;

	CMABState2Manager() {}

	CMABState2Manager(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *moveMAB, float nonRootGreed,
		Player playerID, Player enemyID) {
		this->rootNode = new CMABState2(board, evaluator, coordinateMAB, moveMAB, nonRootGreed, playerID, enemyID, this);
		this->states = new vector<CMABState2*>();
		this->currentIndex = 0;
	}

	void newRound(Board &board, Player playerID, Player enemyID) {
		rootNode->repurposeNode(board, playerID, enemyID);
		currentIndex = 0;
	}

	CMABState2 *getState(Board &board, CMABState2::SharedData *sharedData, Player playerID, Player enemyID) {
		if (currentIndex < states->size()) {
			CMABState2 *result = (*states)[currentIndex];
			result->repurposeNode(board, playerID, enemyID);
			currentIndex++;
			return result;
		}
		else {
			CMABState2 *result = new CMABState2(board, sharedData, playerID, enemyID);
			states->push_back(result);
			currentIndex++;
			return result;
		}
	}

	~CMABState2Manager() {
		for (int i = 0; i < states->size(); i++) {
			delete (*states)[i];
		}
		delete states;
		rootNode->freeShared();
		delete rootNode;
	}
};

#endif