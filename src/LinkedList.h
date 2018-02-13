#ifndef _DEBUG
	#define NDEBUG
#endif

#ifndef LINKEDLIST
#define LINKEDLIST

#include <assert.h>

using namespace std;

template <class T> struct Node {
	Node<T> *previous;
	Node<T> *next;
	T element;
};

template <class T> class LinkedList {
private:
	int currentSize;
	int maxSize;
	Node<T> *head;
	Node<T> *tail;
	bool isValid();

public:
	LinkedList(int size);
	~LinkedList();
	inline void push_on(T element);
	inline void remove(Node<T> *node);
	inline void moveToFront(Node<T> *node);
	inline Node<T> *getFront();
	inline int size();
};

template<class T> bool LinkedList<T>::isValid() {
	if (head->previous != NULL) return false;
	if (tail->next != NULL) return false;
	if (currentSize > maxSize) return false;
	if (currentSize < 0) return false;
	Node<T> *currentNode = head;
	for (int i = 0; i <= currentSize; i++) {
		if (currentNode->next == NULL) return false;
		currentNode = currentNode->next;
	}
	if (currentNode != tail) return false;
	return true;
}

template <class T> LinkedList<T>::LinkedList(int size) {
	maxSize = size;
	head = new Node<T>();
	tail = new Node<T>();
	currentSize = 0;

	head->previous = NULL;
	head->next = tail;
	tail->previous = head;
	tail->next = NULL;
	assert(isValid());
}

template <class T> LinkedList<T>::~LinkedList() {
	assert(isValid());
	Node<T> *currentNode = head;
	while (currentNode != NULL) {
		Node<T> *temp = currentNode;
		currentNode = currentNode->next;
		delete temp;
	}
}

template<class T> inline void LinkedList<T>::push_on(T element) {
	assert(isValid());
	if (currentSize == maxSize) remove(tail->previous); // remove the last element
	Node<T> *newNode = new Node<T>;
	newNode->element = element;
	newNode->previous = head;
	newNode->next = head->next;
	head->next = newNode;
	newNode->next->previous = newNode;
	currentSize++;
	assert(isValid());
}

template<class T>
inline void LinkedList<T>::remove(Node<T> *node) {
	assert(isValid());
	node->previous->next = node->next;
	node->next->previous = node->previous;
	currentSize--;
	delete node;
	assert(isValid());
}

template<class T>
inline void LinkedList<T>::moveToFront(Node<T>* node) {
	assert(isValid());

	// connect the nodes on either side of the node we are moving
	node->next->previous = node->previous;
	node->previous->next = node->next;

	// connect the node with the ones at the front
	Node<T> *currentFront = head->next;
	head->next = node;
	currentFront->previous = node;
	node->previous = head;
	node->next = currentFront;
	assert(isValid());
}

template<class T> inline Node<T> *LinkedList<T>::getFront() {
	return head->next;
}

template<class T> inline int LinkedList<T>::size() {
	return currentSize;
}

#endif // !LINKEDLIST