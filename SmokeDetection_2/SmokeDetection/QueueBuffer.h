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

	// ť�� full�� �ƴ� ��� newElem�� ��� pushBack�ϰ� false ����, ť�� full�̸� �տ��� ��� �ϳ��� ���� newElem�� pushBack������ true ����
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

	// ��� ��ҵ��� ���� ����
	GeneralType sumOfElements () {
		GeneralType elemSum = 0;

		for (int i=0 ; i < q->queueLength () ; i++)
			elemSum += q->lookElement (i);

		return elemSum;
	}

	// �� ��� ���� elemLowestThreshold �̻��̸� ó�� ��� ������ ������ ��� ���� ũ�� 1, ������ -1, ������ 0�� ���� (��� ��� ���� elemLowestThreshold �̻��� �ƴϸ� -2 ����)
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

	// ��� ���� �����ϰ� �����ϴ� ������ ��Ÿ������ �˻��Ѵ�.
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