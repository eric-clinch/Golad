#ifndef CMABStateParallelManager_h
#define CMABStateParallelManager_h

using namespace std;

class CMABStateParallel;

class CMABStateParallelManager {
public:

	vector<CMABStateParallel*> *states;
	CMABStateParallel *rootNode;
	int currentIndex;

	CMABStateParallelManager() {}

	CMABStateParallelManager(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *moveMAB, float nonRootGreed,
		Player playerID, Player enemyID) {
		this->rootNode = new CMABStateParallel(board, evaluator, coordinateMAB, moveMAB, nonRootGreed, playerID, enemyID, this);
		this->states = new vector<CMABStateParallel*>();
		this->currentIndex = 0;
	}

	void newRound(Board &board, Player playerID, Player enemyID) {
		rootNode->repurposeNode(board, playerID, enemyID);
		currentIndex = 0;
	}

	CMABStateParallel *getState(Board &board, CMABStateParallel::SharedData *sharedData, Player playerID, Player enemyID) {
		if (currentIndex < states->size()) {
			CMABStateParallel *result = (*states)[currentIndex];
			result->repurposeNode(board, playerID, enemyID);
			currentIndex++;
			return result;
		}
		else {
			CMABStateParallel *result = new CMABStateParallel(board, sharedData, playerID, enemyID);
			states->push_back(result);
			currentIndex++;
			return result;
		}
	}

	~CMABStateParallelManager() {
		for (int i = 0; i < states->size(); i++) {
			delete (*states)[i];
		}
		delete states;
		rootNode->freeShared();
		delete rootNode;
	}
};

#endif