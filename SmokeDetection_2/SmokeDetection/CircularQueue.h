#pragma once

template <typename GeneralType>
class CircularQueue
{
private:
	int capacity;
	int length;

	int firstElemIndex;
	int lastElemIndex;

	GeneralType* queue;

public:
	CircularQueue<GeneralType> (int newCapacity) {
		queue = new GeneralType[newCapacity];
		capacity = newCapacity;
		length = 0;
		firstElemIndex = 0;
		lastElemIndex = 0;
	}

	~CircularQueue<GeneralType> (void) {
		delete[] queue;
	}

	bool isEmpty () {
		return (length == 0) ? true : false;
	}

	bool isFull () {
		return (length == capacity) ? true : false;
	}

	int queueLength () {
		return length;
	}

	bool pushBack (GeneralType newElem) {
		if (isFull ())
			return false;

		if (isEmpty ()) {
			queue[lastElemIndex] = newElem;
			if (firstElemIndex - 1 < 0)
				firstElemIndex = capacity - 1;
			else
				firstElemIndex--;
			lastElemIndex = (lastElemIndex + 1) % capacity;
			length++;
		} else {
			queue[lastElemIndex] = newElem;
			lastElemIndex = (lastElemIndex + 1) % capacity;
			length++;
		}

		return true;
	}

	bool pushFront (GeneralType newElem) {
		if (isFull ())
			return false;

		if (isEmpty ()) {
			queue[firstElemIndex] = newElem;
			if (firstElemIndex - 1 < 0)
				firstElemIndex = capacity - 1;
			else
				firstElemIndex--;
			lastElemIndex = (lastElemIndex + 1) % capacity;
			length++;
		} else {
			queue[firstElemIndex] = newElem;
			if (firstElemIndex - 1 < 0)
				firstElemIndex = capacity - 1;
			else
				firstElemIndex--;
			length++;
		}

		return true;
	}

	bool removeBack () {
		if (isEmpty ())
			return false;

		if (lastElemIndex - 1 < 0)
			lastElemIndex = capacity - 1;
		else
			lastElemIndex--;
		length--;

		return true;
	}

	bool removeFront () {
		if (isEmpty ())
			return false;

		firstElemIndex = (firstElemIndex + 1) % capacity;
		length--;

		return true;
	}

	GeneralType lookElement (int index) {
		if ( !( (index >= 0) && (index < length) ) )
			return 0;

		int virtualFirstIndex;

		if (firstElemIndex == capacity - 1)
			virtualFirstIndex = 0;
		else
			virtualFirstIndex = firstElemIndex + 1;

		return queue[(virtualFirstIndex + index) % capacity];
	}
};