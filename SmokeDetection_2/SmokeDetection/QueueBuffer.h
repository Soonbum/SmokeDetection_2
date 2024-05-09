#pragma once
#include "CircularQueue.h"

template <typename GeneralType>
class QueueBuffer
{
private:
	CircularQueue<GeneralType>* q;
	int queueCapacity;

public:
	QueueBuffer<GeneralType> (int newCapacity) {
		q = new CircularQueue<GeneralType> (newCapacity);
		queueCapacity = newCapacity;
	}

	~QueueBuffer<GeneralType> (void) {
		delete q;
	}

	bool isEmpty () {
		return q->isEmpty ();
	}

	bool isFull () {
		return q->isFull ();
	}

	// 큐가 full이 아닌 경우 newElem를 계속 pushBack하고 false 리턴, 큐가 full이면 앞에서 요소 하나를 빼고 newElem을 pushBack하으고 true 리턴
	bool fillQueue (GeneralType newElem) {
		if (q->isFull ()) {
			q->removeFront ();
			q->pushBack (newElem);
			return true;
		} else {
			q->pushBack (newElem);
			return false;
		}
	}

	// 모든 요소들의 합을 구함
	GeneralType sumOfElements () {
		GeneralType elemSum = 0;

		for (int i=0 ; i < q->queueLength () ; i++)
			elemSum += q->lookElement (i);

		return elemSum;
	}

	// 각 요소 값이 elemLowestThreshold 이상이며 처음 요소 값보다 마지막 요소 값이 크면 1, 작으면 -1, 같으면 0을 리턴 (모든 요소 값이 elemLowestThreshold 이상이 아니면 -2 리턴)
	int checkTrend (GeneralType elemLowestThreshold) {
		for (int i=0 ; i < q->queueLength () ; i++)
			if ( !(q->lookElement (i) >= elemLowestThreshold) )
				return -2;

		GeneralType firstElem = q->lookElement (0);
		GeneralType lastElem = q->lookElement (q->queueLength () - 1);

		if (firstElem < lastElem)
			return 1;
		else if (firstElem > lastElem)
			return -1;
		else
			return 0;
	}

	// 요소 값이 증가하고 감소하는 패턴이 나타나는지 검사한다.
	int checkPartTrend (GeneralType elemLowestThreshold, GeneralType countLowestThreshold, int subQueueLength) {
		QueueBuffer<GeneralType> subQueue (subQueueLength);

		int beforeTrend = 0;
		int currentTrend = 0;
		int increaseSeriesCount = 0;
		int decreaseSeriesCount = 0;
		int maxIncreaseSeriesCount = 0;
		int maxDecreaseSeriesCount = 0;

		for (int i=0 ; i < (q->queueLength () - subQueueLength) ; i++) {
			for (int j=i ; j < i + subQueueLength ; j++)
				subQueue.fillQueue (q->lookElement (j));
			currentTrend = subQueue.checkTrend (elemLowestThreshold);

			if (currentTrend == 1 && beforeTrend == 1) {
				increaseSeriesCount++;
				maxIncreaseSeriesCount = (maxIncreaseSeriesCount < increaseSeriesCount) ? increaseSeriesCount : maxIncreaseSeriesCount;
			} else if (currentTrend == -1 && beforeTrend == -1) {
				decreaseSeriesCount++;
				maxDecreaseSeriesCount = (maxDecreaseSeriesCount < decreaseSeriesCount) ? decreaseSeriesCount : maxDecreaseSeriesCount;
			}

			beforeTrend = currentTrend;
		}

		if (maxIncreaseSeriesCount >= countLowestThreshold && maxDecreaseSeriesCount >= countLowestThreshold)
			return 1;
		else
			return 0;
	}
};