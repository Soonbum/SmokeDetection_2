#pragma once
#include "QueueBuffer.h"

// OpenCV ��� ����
#include "cv.h"
#include "cvaux.h"
#include "cxcore.h"
#include "highgui.h"

// OpenCV ���̺귯�� �߰����Ӽ�
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "cvaux.lib")
#pragma comment(lib, "cxcore.lib")
#pragma comment(lib, "highgui.lib")


class SmokeDetection
{
private:
	// ȯ�� �Ķ����
	int				bnoRow, bnoCol;
	int				blocksize;

	// �⺻ ���� ����
	IplImage*		ppFrame;
	IplImage*		ppGFrame;
	IplImage*		pFrame;
	IplImage*		pGFrame;
	IplImage*		cFrame;
	IplImage*		cGFrame;

	// 1. ��� ���Ÿ� ���� ����
	IplImage*		Tn;
	IplImage*		Tnp1;
	IplImage*		Bn;
	IplImage*		Bnp1;
	int**			arrMove;
	IplImage*		movePassFrame;

	// 2. ���� �� ������ ���� ����
	int**			arrColor;
	IplImage*		colorPassFrame;

	// 3. ���̺��� ���� Ȯ��, �̵��� ���� ���� ����
	int						smokeBlockNumThreshold;				// ���� ����� �� ��ġ�� �̻��� ������ ���Ⱑ �߻��� ����
	int						smokeBlockNumChangeCountThreshold;	// ���� ����� ����/�����ϴ� �������� ī��Ʈ
	int						subQueueLength;
	QueueBuffer<int>*		smokeBlockNumWindow;				// ������ Ȯ�긦 ���� ���� �����̵� ������ (�ִ� ���̺� ũ�⸦ ����)

	int						beforeSmokeColorBlockExpanding;		// smokeExistWindow�� ��� �� ���� ���¸� ����
	QueueBuffer<int>*		smokeExistWindow;					// ���� �ð� ���� ������ ���� ���θ� ���� ���� �����̵� ������ (���Ⱑ �����ϴ� ���ȿ��� 1, �ƴϸ� 0�� ����)

	double					smokeCenterPointThreshold;			// ���� ������ �ʱ������κ��� �ּ��� �� ���� ������ �־�� ���Ⱑ �̵��ߴٰ� ������
	double					initPtX, initPtY;					// ������ �ʱ����� ���� (smokeExistWindow�� ��� ���� 0->1�� �Ǵ� ����)
	QueueBuffer<double>*	centerPointWindow;					// ������ �ʱ������� ������ ���� ���� �Ÿ��� �����ϱ� ���� �����̵� ������ (���Ⱑ ������ ������ ���� ����)

	char**					arrLabels;							// �ִ� ���̺� �����Ͽ� 2�������� ǥ����

	// ���� ����� ��� ���� ���
	bool			isSmoke;
	int**			arrSmoke;
	IplImage*		smokePassFrame;

	// ���� �߻��� �˷� �ֱ� ���� ����
	bool beforeSmokeOccur;
	bool nowSmokeOccur;

	// ó���� ������ ��Ȳ�� �˱� ���� �÷��� ����
	bool			firstFrameProcessed;
	bool			secondFrameProcessed;

	// �ʱ�ȭ
	void initParameters () {
		ppFrame = NULL;
		ppGFrame = NULL;
		pFrame = NULL;
		pGFrame = NULL;
		cFrame = NULL;
		cGFrame = NULL;

		blocksize = 8;
	}
	void initAlgorithms () {
		/*
		// ??? �����򰡿� �ڵ�
		frameSeq = 0;
		startSmokeSeq = 0;
		endSmokeSeq = 0;
		numOfSmokeFrame = 0;
		*/

		// ��� ���Ÿ� ���� ���� �ʱ�ȭ
		Tn = NULL;
		Tnp1 = NULL;
		Bn = NULL;
		Bnp1 = NULL;
		arrMove = NULL;
		movePassFrame = NULL;

		// ���� �� ������ ���� ���� �ʱ�ȭ
		arrColor = NULL;
		colorPassFrame = NULL;

		// ���̺��� ���� Ȯ��, �̵��� ���� ���� ���� �ʱ�ȭ
		smokeBlockNumThreshold = 3;
		smokeBlockNumChangeCountThreshold = 3;
		subQueueLength = 4;
		smokeBlockNumWindow = new QueueBuffer<int> (150);
		beforeSmokeColorBlockExpanding = 0;
		smokeExistWindow = new QueueBuffer<int> (150);
		smokeCenterPointThreshold = 2.0;
		centerPointWindow = new QueueBuffer<double> (150);
		arrLabels = NULL;
		arrSmoke = NULL;
		smokePassFrame = NULL;

		// ���� �߻�/����/�Ҹ��� �˷� �ֱ� ���� ���� �ʱ�ȭ
		beforeSmokeOccur = false;
		nowSmokeOccur = false;

		// ������ ó�� ���� �ʱ�ȭ
		firstFrameProcessed = false;
		secondFrameProcessed = false;
	}

	// �޸� ����
	template <typename GeneralType>
	void release1DArray (GeneralType* arr) {
		if (arr != NULL)
			delete arr;
		arr = NULL;
	}
	template <typename GeneralType>
	void release2DArray (GeneralType** arr, int row) {
		if (arr != NULL) {
			for (int i=0 ; i < row ; i++)
				delete arr[i];
			delete arr;
		}
		arr = NULL;
	}
	void releaseMemory () {
		cvReleaseImage (&ppFrame);
		cvReleaseImage (&ppGFrame);
		cvReleaseImage (&pFrame);
		cvReleaseImage (&pGFrame);
		cvReleaseImage (&cFrame);
		cvReleaseImage (&cGFrame);

		ppFrame = NULL;
		ppGFrame = NULL;
		pFrame = NULL;
		pGFrame = NULL;
		cFrame = NULL;
		cGFrame = NULL;

		cvReleaseImage (&Tn);
		cvReleaseImage (&Tnp1);
		cvReleaseImage (&Bn);
		cvReleaseImage (&Bnp1);

		delete smokeBlockNumWindow;
		delete smokeExistWindow;
		delete centerPointWindow;

		smokeBlockNumWindow = NULL;
		smokeExistWindow = NULL;
		centerPointWindow = NULL;

		cvReleaseImage (&movePassFrame);
		cvReleaseImage (&colorPassFrame);
		cvReleaseImage (&smokePassFrame);

		movePassFrame = NULL;
		colorPassFrame = NULL;
		smokePassFrame = NULL;

		release2DArray (arrMove, bnoRow);
		release2DArray (arrColor, bnoRow);
		release2DArray (arrLabels, bnoRow);
		release2DArray (arrSmoke, bnoRow);

		arrMove = NULL;
		arrColor = NULL;
		arrLabels = NULL;
		arrSmoke = NULL;
	}

	// �׸��� �Լ�
	void drawRectToImage (IplImage* img, int blocksize, int** binArr, CvScalar color) {
		int i, j;
		CvPoint pt1, pt2;

		for (j=0 ; j < bnoRow ; j++) {
			for (i=0 ; i < bnoCol ; i++) {

				if (binArr[j][i] == 1) {
					pt1.y = j*blocksize+1;	pt2.y = (j+1)*blocksize;
					pt1.x = i*blocksize+1;	pt2.x = (i+1)*blocksize;
					cvDrawRect (img, pt1, pt2, color);
				}
			}
		}
	}

	// �˰���
	void detectBlockMove (int** arrMove, int blocksize, IplImage* cGFrame, IplImage* pGFrame, IplImage* ppGFrame, IplImage* Bn, IplImage* Bnp1, IplImage* Tn, IplImage* Tnp1) {
		int i, j;
		int x, y;
		int alpha = 1;
		IplImage* pxlMove = cvCreateImage (cvGetSize(cGFrame), cGFrame->depth, cGFrame->nChannels);
		IplImage* diffnp1 = cvCreateImage (cvGetSize(cGFrame), cGFrame->depth, cGFrame->nChannels);
		IplImage* diffnp2 = cvCreateImage (cvGetSize(cGFrame), cGFrame->depth, cGFrame->nChannels);

		IplImage* diffImg = cvCreateImage (cvGetSize(cGFrame), cGFrame->depth, cGFrame->nChannels);
		cvAbsDiff (cGFrame, Bn, diffImg);

		double bgnSum = 0.0;
		double bgnAvg = 0.0;
		int bgnCnt = 0;
		double objSum = 0.0;
		double objAvg = 0.0;
		int objCnt = 0;
		int Gap = 6;
		double Tb = 0.0;

		int yStart, yEnd, xStart, xEnd;
		double blkSum = 0.0;
		double blkAvg = 0.0;

		// ���ο� ���� ��� �Ӱ�ġ ���� ����
		cvSetZero (pxlMove);

		// y�� ����, x�� �ʺ��̸�, bgrbgr... ���̹Ƿ� 3*x���� �ȼ��� �� ä�ο� ����
		for (y=0 ; y < cGFrame->height ; y++) {

			cvAbsDiff (cGFrame, pGFrame, diffnp1);
			cvAbsDiff (cGFrame, ppGFrame, diffnp2);

			uchar* cGFrame_ptr = (uchar*)(cGFrame->imageData + y * cGFrame->widthStep);

			for (x = 0 ; x < cGFrame->width ; x++) {

				uchar* diffImg_ptr = (uchar*)(diffImg->imageData + y * diffImg->widthStep);
				uchar* cGFrame_ptr = (uchar*)(diffImg->imageData + y * diffImg->widthStep);

				uchar* diffnp1_ptr = (uchar*)(diffnp1->imageData + y * diffnp1->widthStep);
				uchar* diffnp2_ptr = (uchar*)(diffnp2->imageData + y * diffnp2->widthStep);
				uchar* Tn_ptr = (uchar*)(Tn->imageData + y * Tn->widthStep);
				uchar* Tnp1_ptr = (uchar*)(Tnp1->imageData + y * Tnp1->widthStep);
				uchar* Bn_ptr = (uchar*)(Bn->imageData + y * Bn->widthStep);
				uchar* Bnp1_ptr = (uchar*)(Bnp1->imageData + y * Bnp1->widthStep);
				uchar* pxlMove_ptr = (uchar*)(pxlMove->imageData + y * pxlMove->widthStep);

				// �ȼ��� �������� ����
				if ((diffnp1_ptr[x] > Tn_ptr[x]) && (diffnp2_ptr[x] > Tn_ptr[x]))
					pxlMove_ptr[x] = 1;
				else
					pxlMove_ptr[x] = 0;

				// �ȼ� ���� �Ӱ�ġ�� ����
				// �������� ������,
				if (pxlMove_ptr[x] == 0) {

					Bnp1_ptr[x] = alpha*Bn_ptr[x] + (1-alpha)*cGFrame_ptr[x];
					Tnp1_ptr[x] = alpha*Tn_ptr[x] + (1-alpha)*5*diffImg_ptr[x];

				// �����̸�,
				} else {

					Bnp1_ptr[x] = Bn_ptr[x];
					Tnp1_ptr[x] = Tn_ptr[x];
				}

				// ��ü�� ����� �и�, ��� ó���� ����
				// ��� �ȼ��̸�,
				if (diffImg_ptr[x] < Gap) {

					bgnSum += diffImg_ptr[x];
					bgnCnt ++;
				} else {

					objSum += diffImg_ptr[x];
					objCnt ++;
				}
			}
		}

		// ��ü�� ����� ��հ��� �߰�
		objAvg = objSum / objCnt;
		bgnAvg = bgnSum / bgnCnt;
		Tb = (objAvg + bgnAvg) / 2;

		// ������ �ҷ��� �����Ѵ�.
		// ��� ���� ����ؼ� ������ ��ϰ� �������� ���� ����� ����
		for (y = 0 ; y < bnoRow ; y++) {

			yStart = y*blocksize+1;
			yEnd = (y+1)*blocksize;

			for (x = 0 ; x < bnoCol ; x++) {

				xStart = x*blocksize+1;
				xEnd = (x+1)*blocksize;

				blkSum = 0.0;
				blkAvg = 0.0;

				for (j = yStart ; j <= yEnd ; j++) {

					uchar* diffImg_ptr = (uchar*)(diffImg->imageData + j * diffImg->widthStep);

					for (i = xStart ; i <= xEnd ; i++) {

						blkSum += diffImg_ptr[i];
					}
				}

				blkAvg = blkSum / (blocksize * blocksize);

				// blkAvg�� Tb���� ũ�� arrMove(y,x)�� 1, �ƴϸ� 0
				if (blkAvg > Tb)
					arrMove[y][x] = 1;
				else
					arrMove[y][x] = 0;
			}
		}

		// �޸� ����
		cvReleaseImage (&pxlMove);
		cvReleaseImage (&diffnp1);
		cvReleaseImage (&diffnp2);
		cvReleaseImage (&diffImg);
	}
	void detectSmokeColor (int** arrMove, int** arrColor, int blocksize, IplImage* cFrame) {
		int x, y;
		int i, j;
		int iNumOfSmokePxl;
		int yStart, yEnd, xStart, xEnd;

		int Tb = blocksize * blocksize / 4 * 3;	// ��� �ȿ� �ش� �ȼ��� ������ Tb�� �Ѵ��� ���θ� �˻���

		int maxComponent, minComponent, diffOfComponents, avgOfComponents;

		for (y = 0 ; y < bnoRow ; y++) {

			yStart = y*blocksize+1;
			yEnd = (y+1)*blocksize;

			for (x = 0 ; x < bnoCol ; x++) {

				// ������ ����� �ƴϸ� ����
				if (arrMove[y][x] == 0)
					continue;

				xStart = x*blocksize+1;
				xEnd = (x+1)*blocksize;

				iNumOfSmokePxl = 0;	// ��� �� ���� �ȼ��� ��

				for (j = yStart ; j <= yEnd ; j++) {

					uchar* cFrame_ptr = (uchar*)(cFrame->imageData + j * cFrame->widthStep);

					for (i = xStart ; i <= xEnd ; i++) {

						maxComponent = MaxOfThree (cFrame_ptr[3*i], cFrame_ptr[3*i+1], cFrame_ptr[3*i+2]);
						minComponent = MinOfThree (cFrame_ptr[3*i], cFrame_ptr[3*i+1], cFrame_ptr[3*i+2]);
						diffOfComponents = maxComponent - minComponent;
						avgOfComponents = AvgOfThree (cFrame_ptr[3*i], cFrame_ptr[3*i+1], cFrame_ptr[3*i+2]);

						// R,G,B �� ���� ū �Ͱ� ���� ���� ���� ���̰� 15 �̳��̸鼭 intensity(R,G,B�� ���)�� 100~255 ������ ���
						if ((diffOfComponents < 15) && (avgOfComponents > 100) && (avgOfComponents <= 255))
							iNumOfSmokePxl ++;
					}
				}

				if (iNumOfSmokePxl > Tb)
					arrColor[y][x] = 1;
				else
					arrColor[y][x] = 0;
			}
		}
	}
	int isLongSmokeDiffusing (int maxLabelSize) {
		// ���� ����� Ȯ���ϴ��� �˷��� (Ȯ���� 1, ��Ҵ� -1, ��ü�� 0)
		int isFilled = smokeBlockNumWindow->fillQueue (maxLabelSize);

		if (isFilled)
			return smokeBlockNumWindow->checkTrend (smokeBlockNumThreshold);
		else
			return 0;
	}
	int isShortSmokeDiffusing (int maxLabelSize) {
		// ���� ����� Ȯ��/����ϴ��� �˷��� (Ȯ��/��Ҵ� 1, ��ü�� 0)
		int isFilled = smokeBlockNumWindow->fillQueue (maxLabelSize);

		if (isFilled)
			return smokeBlockNumWindow->checkPartTrend (smokeBlockNumThreshold, smokeBlockNumChangeCountThreshold, subQueueLength);
		else
			return 0;
	}
	int isSmokeOccur (int diffuseTrend) {
		// ���Ⱑ �߻��ϰ� �ִ� ��Ȳ�� �ƴϸ� 0, �߻��ϰ� �ִٸ� 0���� ū ���� ����
		if (diffuseTrend == 1)
			smokeExistWindow->fillQueue (1);
		else
			smokeExistWindow->fillQueue (0);

		return smokeExistWindow->sumOfElements ();
	}
	int isSmokeMassMoving (int smokeColorBlockExpanding, int diffuseTrend) {
		// ���Ⱑ �̵��ϴ��� ���θ� Ȯ���Ѵ�. (�̵��� 1, ���ʹ� -1, ��ü�� 0)
		if (smokeColorBlockExpanding == 0)
			return 0;

		int smokeRow = 0, smokeCol = 0;
		int start_x = 0, end_x = 0;
		int start_y = 0, end_y = 0;
		double center_x = 0.0;	double acc_x = 0.0;	double avg_center_x = 0.0;
		double center_y = 0.0;	double acc_y = 0.0;	double avg_center_y = 0.0;
		bool isFoundXLine = false;
		bool isFoundYLine = false;
		bool isFoundStartPoint = false;
		bool isFoundEndPoint = false;

		if (diffuseTrend == 1) {
			// arrLabels���� ���� ū ���� ����� �¿�(��) ������ ã�Ƴ�
			for (int j=0 ; j < bnoRow ; j++) {

				isFoundXLine = false;
				isFoundStartPoint = false;
				isFoundEndPoint = false;

				for (int i=0 ; i < bnoCol ; i++) {
					// ���� ���� ó�� �߰ߵǸ� ���� �� ���� �� �Ŀ�
					if (arrLabels[j][i] != 0) {
						if (!isFoundXLine) {
							isFoundXLine = true;
							smokeRow++;
						}

						// ó�� ���� ã��
						if (!isFoundStartPoint) {
							isFoundStartPoint = true;
							start_x = i;
						}
					}

					if (arrLabels[j][(bnoCol-1)-i] != 0) {
						if (!isFoundXLine) {
							isFoundXLine = true;
							smokeRow++;
						}

						// ������ ���� ã��
						if (!isFoundEndPoint) {
							isFoundEndPoint = true;
							end_x = (bnoCol-1)-i;
						}
					}

					// �� ���� �ε��� ���� 2�� ������ ������ ���ϰ� ���� ������
					if (isFoundStartPoint && isFoundEndPoint) {
						center_x = (start_x + end_x) / 2.0;
						acc_x += center_x;
					}
				}
			}

			// ���� ��ǥ�� ������ ���� ���� �� ���� ������ �¿�(��) ��� ������ ���� �� �ִ�.
			if (smokeRow != 0)
				avg_center_x = acc_x / (double)smokeRow;

			// arrLabels���� ���� ū ���� ����� ����(��) ������ ã�Ƴ�
			for (int i=0 ; i < bnoCol ; i++) {

				isFoundYLine = false;
				isFoundStartPoint = false;
				isFoundEndPoint = false;

				for (int j=0 ; j < bnoRow ; j++) {
					// ���� ���� ó�� �߰ߵǸ� ���� �� ���� �� �Ŀ�
					if (arrLabels[j][i] != 0) {
						if (!isFoundYLine) {
							isFoundYLine = true;
							smokeCol++;
						}

						// ó�� ���� ã��
						if (!isFoundStartPoint) {
							isFoundStartPoint = true;
							start_y = j;
						}
					}

					if (arrLabels[(bnoRow-1)-j][i] != 0) {
						if (!isFoundYLine) {
							isFoundYLine = true;
							smokeCol++;
						}

						// ������ ���� ã��
						if (!isFoundEndPoint) {
							isFoundEndPoint = true;
							end_y = (bnoRow-1)-j;
						}
					}

					// �� ���� �ε��� ���� 2�� ������ ������ ���ϰ� ���� ������
					if (isFoundStartPoint && isFoundEndPoint) {
						center_y = (start_y + end_y) / 2.0;
						acc_y += center_y;
					}
				}
			}

			// �߽� ��ǥ�� ������ ���� ���� �� ���� ������ ����(��) ��� ������ ���� �� �ִ�.
			if (smokeCol != 0)
				avg_center_y = acc_y / (double)smokeCol;
		}

		if (smokeRow == 0 || smokeCol == 0)
			return 0;

		// ���Ⱑ ó�� �����ϰ� �Ǹ� �ʱ����� ������
		if (beforeSmokeColorBlockExpanding == 0 && smokeColorBlockExpanding == 1) {
			initPtX = avg_center_x;
			initPtY = avg_center_y;
		}

		bool isFilled = false;

		// growthTrend�� 1�� ���� centerPointWindow�� ���� ���� ����� ������ �ʱ����� ���� �Ÿ��� �ִ´�.
		if (diffuseTrend == 1) {
			// ��Ÿ����� ������ �̿��Ͽ� �ʱ����� ���ο� ���� ���� �Ÿ��� ����
			double distance = sqrt( (avg_center_x - initPtX)*(avg_center_x - initPtX) + (avg_center_y - initPtY)*(avg_center_y - initPtY) );
			isFilled = centerPointWindow->fillQueue (distance);
		}

		// ������ �ʱ� ������ ���� ���� ���� �Ÿ��� �����ϸ� �߽����� �̵������� �ǹ��ϹǷ� ���Ⱑ Ȯ��Ǿ����� �ǹ���
		if (isFilled)
			return centerPointWindow->checkTrend (smokeCenterPointThreshold);

		return 0;
	}
	void chooseMovingObject () {
		if (arrMove == NULL) {
			arrMove = new int*[bnoRow];
			for (int i=0 ; i < bnoRow ; i++) {
				arrMove[i] = new int[bnoCol];
				memset (arrMove[i], 0, sizeof(int)*bnoCol);
			}
		} else {
			for (int i=0 ; i < bnoRow ; i++) {
				memset (arrMove[i], 0, sizeof(int)*bnoCol);
			}
		}
		detectBlockMove (arrMove, blocksize, cGFrame, pGFrame, ppGFrame, Bn, Bnp1, Tn, Tnp1);

		if (movePassFrame == NULL)
			movePassFrame = cvCreateImage (cvGetSize(cFrame), cFrame->depth, cFrame->nChannels);
		cvCopy (cFrame, movePassFrame);
		CvScalar color = CV_RGB(0,255,0);
		drawRectToImage (movePassFrame, blocksize, arrMove, color);
	}
	void chooseSmokeColorObject () {
		if (arrColor == NULL) {
			arrColor = new int*[bnoRow];
			for (int i=0 ; i < bnoRow ; i++) {
				arrColor[i] = new int[bnoCol];
				memset (arrColor[i], 0, sizeof(int)*bnoCol);
			}
		} else {
			for (int i=0 ; i < bnoRow ; i++) {
				memset (arrColor[i], 0, sizeof(int)*bnoCol);
			}
		}
		detectSmokeColor (arrMove, arrColor, blocksize, cFrame);

		if (colorPassFrame == NULL)
			colorPassFrame = cvCreateImage (cvGetSize(cFrame), cFrame->depth, cFrame->nChannels);
		cvCopy (cFrame, colorPassFrame);
		CvScalar color = CV_RGB(255,0,0);
		drawRectToImage (colorPassFrame, blocksize, arrColor, color);
	}
	void chooseSmokeObject () {
		char* pData = new char[bnoRow * bnoCol];	// ������ 1�������� �ű� ��
		char* labels = new char[bnoRow * bnoCol];	// pData ������ �� �ȼ��� ���̺� ��ȣ�� �ű� �� (���̺� ��ȣ�� 1���� ����)

		// ���̺� �ϱ� ���� int** ��� �迭�� IplImage*�� ������ ��
		for (int j=0 ; j < bnoRow ; j++)
			for (int i=0 ; i < bnoCol ; i++)
				pData[j*bnoCol + i] = arrColor[j][i];

		IplImage blockImg;
		memset (&blockImg, 0, sizeof(IplImage));
		blockImg.height = bnoRow;
		blockImg.width = bnoCol;
		blockImg.widthStep = bnoCol;
		blockImg.imageData = pData;

		int numOfObj = bwlabel (&blockImg, labels);

		int* sizeOfEachObj = NULL;

		// �� ���̺��� ũ�⸦ ����
		if (sizeOfEachObj == NULL) {
			sizeOfEachObj = new int[numOfObj+1];
			for (int i=0 ; i < numOfObj+1 ; i++)
				sizeOfEachObj[i] = 0;
		}
		for (int i=0 ; i < (bnoRow * bnoCol) ; i++)
			if (labels[i] > 0)
				sizeOfEachObj[labels[i]]++;

		// ���� ū ���̺��� ũ��� ��ȣ�� �˾Ƴ���
		int maxLabelSize = 0;
		int maxLabelIndex = 0;

		for (int i=1 ; i <= numOfObj ; i++) {
			if (sizeOfEachObj[i] > maxLabelSize) {
				maxLabelSize = sizeOfEachObj[i];
				maxLabelIndex = i;
			}
		}

		int longSmokeDiffuseTrend = isLongSmokeDiffusing (maxLabelSize);
		int shortSmokeDiffuseTrend = isShortSmokeDiffusing (maxLabelSize);

		int smokeColorBlockExpanding;

		if (longSmokeDiffuseTrend > shortSmokeDiffuseTrend)
			smokeColorBlockExpanding = isSmokeOccur (longSmokeDiffuseTrend);
		else
			smokeColorBlockExpanding = isSmokeOccur (shortSmokeDiffuseTrend);

		// ���� ū ũ���� ���̺� ���ܼ� 2���� �������� ����
		if (arrLabels == NULL) {
			arrLabels = new char*[bnoRow];
			for (int i=0 ; i < bnoRow ; i++) {
				arrLabels[i] = new char[bnoCol];
				memset (arrLabels[i], 0, sizeof(char)*bnoCol);
			}
		} else {
			for (int i=0 ; i < bnoRow ; i++) {
				memset (arrLabels[i], 0, sizeof(char)*bnoCol);
			}
		}

		// labels���� maxLabelIndex�� �ش��ϴ� �κи� �����ؼ� 2���� ���� �̹����� �����.
		for (int j=0 ; j < bnoRow ; j++)
			for (int i=0 ; i < bnoCol ; i++)
				if (labels[j*bnoCol + i] == maxLabelIndex)
					arrLabels[j][i] = labels[j*bnoCol + i];

		// ������ Ȯ�� ���� Ȯ��
		if (longSmokeDiffuseTrend > shortSmokeDiffuseTrend)
			isSmoke = (isSmokeMassMoving (smokeColorBlockExpanding, longSmokeDiffuseTrend) == 1) ? true : false;
		else
			isSmoke = (isSmokeMassMoving (smokeColorBlockExpanding, shortSmokeDiffuseTrend) == 1) ? true : false;

		// smokeExistWindow�� ��� ���� ���� ���·� ����
		beforeSmokeColorBlockExpanding = smokeColorBlockExpanding;

		if (isSmoke == true) {
			if (arrSmoke == NULL) {
				arrSmoke = new int*[bnoRow];
				for (int i=0 ; i < bnoRow ; i++) {
					arrSmoke[i] = new int[bnoCol];
					memset (arrSmoke[i], 0, sizeof(int)*bnoCol);
				}
			} else {
				for (int i=0 ; i < bnoRow ; i++) {
					memset (arrSmoke[i], 0, sizeof(int)*bnoCol);
				}
			}

			for (int j=0 ; j < bnoRow ; j++) {
				for (int i=0 ; i < bnoCol ; i++) {
					if (labels[j*bnoCol + i] == maxLabelIndex)
						arrSmoke[j][i] = 1;
					else
						arrSmoke[j][i] = 0;
				}
			}

			if (smokePassFrame == NULL)
				smokePassFrame = cvCreateImage (cvGetSize(cFrame), cFrame->depth, cFrame->nChannels);
			cvCopy (cFrame, smokePassFrame);
			CvScalar color = CV_RGB(255,0,0);
			drawRectToImage (smokePassFrame, blocksize, arrSmoke, color);

			CvFont font;
			cvInitFont (&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1, 8);
			cvPutText (smokePassFrame, "Smoke detect!", cvPoint(10, 20), &font, CV_RGB(255,0,0));
			//numOfSmokeFrame++;	// ??? �����򰡿� �ڵ�
		} else {
			if (smokePassFrame == NULL)
				smokePassFrame = cvCreateImage (cvGetSize(cFrame), cFrame->depth, cFrame->nChannels);
			cvCopy (cFrame, smokePassFrame);
		}

		release1DArray (pData);
		release1DArray (labels);
		release1DArray (sizeOfEachObj);
	}

	// ������ ó��
	void updateBackground () {
		cvCopy (cFrame, pFrame);
		cvCopy (pGFrame, ppGFrame);
		cvCopy (cGFrame, pGFrame);

		cvCopy (Tnp1, Tn);
		cvCopy (Bnp1, Bn);
	}
	void processFirstFrame (IplImage *m_pImage) {
		if (ppFrame == NULL)
			ppFrame = cvCreateImage (cvGetSize(m_pImage), m_pImage->depth, m_pImage->nChannels);
		cvCopy (m_pImage, ppFrame);
		if (ppGFrame == NULL)
			ppGFrame = cvCreateImage (cvGetSize(ppFrame), ppFrame->depth, 1);
		cvCvtColor (ppFrame, ppGFrame, CV_BGR2GRAY);

		bnoRow = ppFrame->height / blocksize;
		bnoCol = ppFrame->width / blocksize;

		// ������ �ȼ� ����� ���� ������
		if (Tn == NULL)		Tn = cvCreateImage (cvGetSize(ppGFrame), ppGFrame->depth, ppGFrame->nChannels);
		if (Tnp1 == NULL)	Tnp1 = cvCreateImage (cvGetSize(ppGFrame), ppGFrame->depth, ppGFrame->nChannels);
		if (Bn == NULL)		Bn = cvCreateImage (cvGetSize(ppGFrame), ppGFrame->depth, ppGFrame->nChannels);
		if (Bnp1 == NULL)	Bnp1 = cvCreateImage (cvGetSize(ppGFrame), ppGFrame->depth, ppGFrame->nChannels);

		cvSet (Tn, cvScalar(5, 0, 0, 0));	// �Ӱ�ġ ����

		cvCopy (ppGFrame, Bn);	// ��� ���� �ʱ�ȭ
	}
	void processSecondFrame (IplImage *m_pImage) {
		if (pFrame == NULL)
			pFrame = cvCreateImage (cvGetSize(m_pImage), m_pImage->depth, m_pImage->nChannels);
		cvCopy (m_pImage, pFrame);
		if (pGFrame == NULL)
			pGFrame = cvCreateImage (cvGetSize(pFrame), pFrame->depth, 1);
		cvCvtColor (pFrame, pGFrame, CV_BGR2GRAY);
	}
	void processAfterSecondFrame (IplImage *m_pImage) {
		if (cFrame == NULL)
			cFrame = cvCreateImage (cvGetSize(m_pImage), m_pImage->depth, m_pImage->nChannels);
		cvCopy (m_pImage, cFrame);

		if (cGFrame == NULL)
			cGFrame = cvCreateImage (cvGetSize(cFrame), cFrame->depth, 1);
		cvCvtColor (cFrame, cGFrame, CV_BGR2GRAY);

		chooseMovingObject ();
		chooseSmokeColorObject ();
		chooseSmokeObject ();
	}

	// ��� �Լ�
	int MaxOfThree (int x, int y, int z)	{ return (x > y) ? ((x > z) ? x :  z) : ((y > z) ? y : z); }
	int MinOfThree (int x, int y, int z)	{ return (x < y) ? ((x < z) ? x : z) : ((y < z) ? y : z); }
	int AvgOfThree (int x, int y, int z)	{ return ((x+y+z)/3); }

	// ���̺� �˰���
	int findNeighboringLabel (int lset[], int x) {
		// ���̺� ���̺��� x�� ���̺��� �������� �ʴ´ٸ� ����� ���̺� ��ȣ�� �˷��ְ�, �����Ѵٸ� �״�� ����Ѵ�.
		if (lset[x] != x)	x = lset[x];
		return x;
	}
	int bwlabel (IplImage* img, char* labels, int n=4) {
		/*
		labeling scheme
		+-+-+-+
		|D|C|E|
		+-+-+-+
		|B|A| |
		+-+-+-+
		| | | |
		+-+-+-+
		A�� �߾� �ȼ�, connectedness�� 3���� ������ �ִ�.
		4: A, B, C�� ����Ǿ� ����
		6: A, B, C, D�� ����Ǿ� ����
		8: A, B, C, D, E�� ����Ǿ� ����
		input:
			img -- �׷��̽����� �̹��� Ȥ�� ���̳ʸ� �̹��� (�ȼ� ���� 0�̸� ��ü�� ���� ������ �ν���, 0�� �ƴϸ� ��ü�� �ν�)
			labels -- �ȼ��� ���̺��� �ǹ��ϸ� ���������� �ش� �ȼ��� ���� ���̺� ��ȣ�� �ǹ���, ���� 1 �̻��� ����, labels[row * col]
			n -- n ����
		output:
			����� ������ ��
		*/
		int imgRow = img->height;
		int imgCol = img->width;
		int imgRowCol = imgRow * imgCol;

		// ���̺� ���, �Լ� ���� �Ŀ��� ����� ���⿡ ������
		memset (labels, 0, imgRowCol * sizeof(char));
		int numOfObj = 0;

		// ��Ÿ ����
		int* lset = new int[imgRowCol];   // ���̺� ���̺�
		memset (lset, 0, imgRowCol * sizeof(int));
		int ntable = 0;	// ���̺��� ���� (���̺� ��ȣ�� 1������ ������)

		for (int r=0 ; r < imgRow ; r++) {
			for (int c=0 ; c < imgCol ; c++) {
				if (CV_IMAGE_ELEM(img, unsigned char, r, c)) {	// ���� A�� ��ü���
					// �̿� �ȼ� B,C,D,E�� ã�ƺ���.
					int B, C, D, E;
					if (c == 0)					B = 0;	// ù° ���̸� B�� ����
					else						B = findNeighboringLabel (lset, labels[r*imgCol + (c-1)]);	// (r, c-1) �ȼ��� ���̺� ��ȣ�� ������

					if (r == 0)					C = 0;	// ù° ���̸� C�� ����
					else						C = findNeighboringLabel (lset, labels[(r-1)*imgCol + c]);	// (r-1, c) �ȼ��� ���̺� ��ȣ�� ������

					if (r == 0 || c == 0)		D = 0;	// ù° ��, ù° ���̸� D�� ����
					else						D = findNeighboringLabel (lset, labels[(r-1)*imgCol + (c-1)]);	// (r-1, c-1) �ȼ��� ���̺� ��ȣ�� ������

					if (r == 0 || c == imgCol-1)		E = 0;	// ù° ����, ������ ���̸� E�� ����
					else						E = findNeighboringLabel (lset, labels[(r-1)*imgCol + (c+1)]);	// (r-1, c+1) �ȼ��� ���̺� ��ȣ�� ������

					if (n == 4) {
						// 4 connectedness ����
						if (B && C) {	// B�� C�� ���̺��� ������ �ִٸ�
							if (B == C)
								labels[r*imgCol + c] = B;	// B, C�� ����� ��� (r, c) �ȼ�, �� A�� ���̺��� B�� ����ȴ�. (A-B-C ����)
							else {
								lset[C] = B;			// B, C�� ������� ���� ��� C�� ���̺��� B�� ����ǰ�
								labels[r*imgCol + c] = B;	// A�� ���̺� ���� B�� ����ȴ�. (A-B-C ����)
							}
						} else if (B)	// B�� ��ü, C�� ��ü �ƴϸ�
							labels[r*imgCol + c] = B;		// A-B ����
						else if (C)	// C�� ��ü, B�� ��ü �ƴϸ�
							labels[r*imgCol + c] = C;		// A-C ����
						else {		// B, C, D�� ��ü�� �ƴ϶�� - ���ο� ��ü
							// ���̺��� ���̰� ���̺� ����
							ntable++;	// ���̺��� ��ȣ�� ���� �Ҵ��ϰ�
							labels[r*imgCol + c] = lset[ntable] = ntable;	// ���ο� ��ȣ�� lable set�� �����ϰ� labels�� (r, c) �ȼ����� �Ҵ��Ѵ�.
						}
					} else if (n == 6) {
						// 6 connectedness ����
						if (D)	// D ��ü
							labels[r*imgCol + c] = D;		// A-D ����
						else if (B && C) {		// B�� C�� ���̺��� ������ �ִٸ�
							if (B == C)
								labels[r*imgCol + c] = B;	// A-B ����
							else {
								int tlabel = (((B) < (C)) ? (B) : (C));	// ���� ���̺� ��ȣ�� ���ͼ� B,C�� ���� �Ҵ��ϰ�
								lset[B] = tlabel;
								lset[C] = tlabel;
								labels[r*imgCol + c] = tlabel;	// A�� ���̺� ��ȣ�� B, C�� ���� (A-B-C ����)
							}
						} else if (B)		// B�� ��ü, C�� ��ü �ƴϸ�,
							labels[r*imgCol + c] = B;		// A-B ����
						else if (C)		// C�� ��ü B�� ��ü �ƴϸ�,
							labels[r*imgCol + c] = C;		// A-C ����
						else {			// B, C, D�� ��ü�� �ƴ϶�� - ���ο� ��ü
							// ���̺��� ���̰� ���̺� ����
							ntable++;	// ���̺��� ��ȣ�� ���� �Ҵ��ϰ�
							labels[r*imgCol + c] = lset[ntable] = ntable;	// ���ο� ��ȣ�� lable set�� �����ϰ� labels�� (r, c) �ȼ����� �Ҵ��Ѵ�.
						}
					} else if (n == 8) {
						// 8 connectedness ����
						if (B || C || D || E) {
							int tlabel = B;

							// B, C, D, E ������� ���鼭 ���� ���� ���̺� ��ȣ�� ��
							if (B)			tlabel = B;
							else if (C)		tlabel = C;
							else if (D)		tlabel = D;
							else if (E)		tlabel = E;

							labels[r*imgCol + c] = tlabel;	// A�� ���̺� ��ȣ�� �Ҵ�

							// A�� ����� B, C, D, E�� ��� ������ ���̺� ��ȣ�� ����
							if (B && B != tlabel)	lset[B] = tlabel;
							if (C && C != tlabel)	lset[C] = tlabel;
							if (D && D != tlabel)	lset[D] = tlabel;
							if (E && E != tlabel)	lset[E] = tlabel;
						} else {
							// ���̺��� ���̰� ���̺� ����
							ntable++;	// ���̺��� ��ȣ�� ���� �Ҵ��ϰ�
							labels[r*imgCol + c] = lset[ntable] = ntable;	// ���ο� ��ȣ�� lable set�� �����ϰ� labels�� (r, c) �ȼ����� �Ҵ��Ѵ�.
						}
					}
				} else
					labels[r*imgCol + c] = 0;	// A�� ��ü�� �ƴϹǷ� �Ѿ
			}
		}

		// ��� ���̺� �����ϱ�
		for (int i=0 ; i <= ntable ; i++)
			lset[i] = findNeighboringLabel (lset, i);	// i�� ���̺� ��ȣ�� i���̸� �״�� �����ǰ�, �׷��� ������ ����� ���̺� ��ȣ�� ������ (�ʱ�ȭ ������ ���̺� ��ȣ���� �����Ǿ� ����)

		// ��� ���̺��� ���� �̹��� �����ϱ� (1�� ��ĵ)
		for (int r=0 ; r < imgRow ; r++)
			for (int c=0 ; c < imgCol ; c++)
				labels[r*imgCol + c] = lset[labels[r*imgCol + c]];	// (r, c) �ȼ��� ���̺� ��ȣ�� ���̺� ���̺� ������ (r, c) �ȼ��� ���̺� ��ȣ�� ���̺� ���̺��� ������ �Ҵ��� (���� ��ȣ�� ���� ����)

		// �̹��� �� ��ü �� �����
		for (int i=0 ; i <= ntable ; i++)
			lset[i] = 0;	// ���̺� ���̺� �ʱ�ȭ
		for (int r=0 ; r < imgRow ; r++)
			for (int c=0 ; c < imgCol ; c++)
				lset[labels[r*imgCol + c]]++;	// ���̺� ���̺��� �� ��ȣ���� �� ���̺� ��ȣ�� �ȼ� ������ ������ (�ʱ�ȭ �Ŀ��� �� �ε����� �ش��ϴ� ���̺� ��ȣ�� �ȼ� ������ �ǹ���)

		// number the objects from 1 through n objects
		lset[0] = 0;
		for (int i=1 ; i <= ntable ; i++)
			if (lset[i] > 0)
				lset[i] = ++numOfObj;	// ���� i�� ���̺� �ش��ϴ� �ȼ��� 0������ ���ٸ� i������ ������� ���̺� ���� ��ü ������ �Ҵ��� (���̺� ���̺��� ��������� 0, 1, 2, 3, ...�� �ȴ�)

		// ��� ���̺��� ���� �̹��� �����ϱ� (2�� ��ĵ)
		for (int r=0 ; r < imgRow ; r++)
			for (int c=0 ; c < imgCol ; c++)
				labels[r*imgCol + c] = lset[labels[r*imgCol + c]];	// ���������� ���̺��� �� �ȼ��� �ڽ��� ���� ���̺� ��ȣ�� �Ҵ��

		delete lset;
		return numOfObj;
	}

public:
	/*
	// ??? �����򰡿� �ڵ�
	int				frameSeq;
	int				startSmokeSeq;
	int				endSmokeSeq;
	int				numOfSmokeFrame;
	*/

	SmokeDetection () {
		initParameters ();
		initAlgorithms ();
	}

	~SmokeDetection () {
		releaseMemory ();
	}

	int detectSmoke (IplImage *m_pImage) {

		//frameSeq++;	// ??? �����򰡿� �ڵ�

		if (firstFrameProcessed == false) {
			processFirstFrame (m_pImage);
			firstFrameProcessed = true;
			return 1;
		} else if (secondFrameProcessed == false) {
			processSecondFrame (m_pImage);
			secondFrameProcessed = true;
			return 2;
		} else {
			processAfterSecondFrame (m_pImage);
			updateBackground ();
			return 3;
		}
	}

	IplImage* getMovePassFrame () {
		return movePassFrame;
	}

	IplImage* getColorPassFrame () {
		return colorPassFrame;
	}

	IplImage* getSmokePassFrame () {
		return smokePassFrame;
	}

	int isSmokeEvent () {
		// ���Ⱑ ��Ÿ���� �� 1, �Ҹ����� �� -1, ���� �߿��� 0 ����
		if (isSmoke == true)
			nowSmokeOccur = true;

		if (smokeExistWindow->sumOfElements () == 0)
			nowSmokeOccur = false;

		bool smokeAppeared = (!beforeSmokeOccur && nowSmokeOccur);
		bool smokeDisappeared = (beforeSmokeOccur && !nowSmokeOccur);

		beforeSmokeOccur = nowSmokeOccur;

		if (smokeAppeared) {
			//startSmokeSeq = frameSeq;		// ??? �����򰡿� �ڵ�
			return 1;
		}
		if (smokeDisappeared) {
			//endSmokeSeq = frameSeq;		// ??? �����򰡿� �ڵ�
			return -1;
		}

		return 0;
	}
};