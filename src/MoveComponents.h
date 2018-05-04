#ifndef MOVECOMPONENTS_h
#define MOVECOMPONENTS_h

#include "UtilityNode.h"
#include "Coordinate.h"
#include "Move.h"

class MoveComponents {
public:
	UtilityNode<Coordinate> *targetNode;
	UtilityNode<Coordinate> *sacrificeNode1;
	UtilityNode<Coordinate> *sacrificeNode2;
	Move move;

	MoveComponents() {}

	// constructor for killer moves
	MoveComponents(UtilityNode<Coordinate> *targetNode) {
		assert(targetNode != NULL);
		this->targetNode = targetNode;
		this->sacrificeNode1 = NULL;
		this->sacrificeNode2 = NULL;
		this->move = Move(targetNode->object);
	}

	// constructor for birth moves
	MoveComponents(UtilityNode<Coordinate> *targetNode, UtilityNode<Coordinate> *sacrificeNode1, UtilityNode<Coordinate> *sacrificeNode2) {
		assert(targetNode != NULL && sacrificeNode1 != NULL && sacrificeNode2 != NULL);
		this->targetNode = targetNode;
		this->sacrificeNode1 = sacrificeNode1;
		this->sacrificeNode2 = sacrificeNode2;
		this->move = Move(targetNode->object, sacrificeNode1->object, sacrificeNode2->object);
	}

	void updateUtilities(float newUtility) {
		targetNode->updateUtility(newUtility);
		if (sacrificeNode1 != NULL) {
			assert(sacrificeNode2 != NULL);
			sacrificeNode1->updateUtility(newUtility);
			sacrificeNode2->updateUtility(newUtility);
		}
	}
};


#endif // !MOVECOMPONENTS_h