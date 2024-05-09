#pragma once
#include "QueueBuffer.h"

// OpenCV 헤더 파일
#include "cv.h"
#include "cvaux.h"
#include "cxcore.h"
#include "highgui.h"

// OpenCV 라이브러리 추가종속성
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "cvaux.lib")
#pragma comment(lib, "cxcore.lib")
#pragma comment(lib, "highgui.lib")


class SmokeDetection
{
private:
	// 환경 파라미터
	int				bnoRow, bnoCol;
	int				blocksize;

	// 기본 영상 변수
	IplImage*		ppFrame;
	IplImage*		ppGFrame;
	IplImage*		pFrame;
	IplImage*		pGFrame;
	IplImage*		cFrame;
	IplImage*		cGFrame;

	// 1. 배경 제거를 위한 변수
	IplImage*		Tn;
	IplImage*		Tnp1;
	IplImage*		Bn;
	IplImage*		Bnp1;
	int**			arrMove;
	IplImage*		movePassFrame;

	// 2. 연기 색 감지를 위한 변수
	int**			arrColor;
	IplImage*		colorPassFrame;

	// 3. 레이블링과 연기 확산, 이동을 보기 위한 변수
	int						smokeBlockNumThreshold;				// 연기 블록이 이 수치값 이상이 나오면 연기가 발생한 것임
	int						smokeBlockNumChangeCountThreshold;	// 연기 블록이 증가/감소하는 연속적인 카운트
	int						subQueueLength;
	QueueBuffer<int>*		smokeBlockNumWindow;				// 연기의 확산를 보기 위한 슬라이딩 윈도우 (최대 레이블 크기를 넣음)

	int						beforeSmokeColorBlockExpanding;		// smokeExistWindow의 요소 합 이전 상태를 보관
	QueueBuffer<int>*		smokeExistWindow;					// 일정 시간 동안 연기의 존재 여부를 보기 위한 슬라이딩 윈도우 (연기가 존재하는 동안에는 1, 아니면 0을 넣음)

	double					smokeCenterPointThreshold;			// 연기 중점이 초기점으로부터 최소한 이 정도 떨어져 있어야 연기가 이동했다고 간주함
	double					initPtX, initPtY;					// 연기의 초기중점 지정 (smokeExistWindow의 요소 합이 0->1이 되는 순간)
	QueueBuffer<double>*	centerPointWindow;					// 연기의 초기중점과 이후의 중점 간의 거리를 보관하기 위한 슬라이딩 윈도우 (연기가 성장할 때에만 값을 넣음)

	char**					arrLabels;							// 최대 레이블만 선택하여 2차원으로 표현함

	// 최종 결과를 담기 위한 결과
	bool			isSmoke;
	int**			arrSmoke;
	IplImage*		smokePassFrame;

	// 연기 발생을 알려 주기 위한 변수
	bool beforeSmokeOccur;
	bool nowSmokeOccur;

	// 처리된 프레임 현황을 알기 위한 플래그 변수
	bool			firstFrameProcessed;
	bool			secondFrameProcessed;

	// 초기화
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
		// ??? 성능평가용 코드
		frameSeq = 0;
		startSmokeSeq = 0;
		endSmokeSeq = 0;
		numOfSmokeFrame = 0;
		*/

		// 배경 제거를 위한 변수 초기화
		Tn = NULL;
		Tnp1 = NULL;
		Bn = NULL;
		Bnp1 = NULL;
		arrMove = NULL;
		movePassFrame = NULL;

		// 연기 색 감지를 위한 변수 초기화
		arrColor = NULL;
		colorPassFrame = NULL;

		// 레이블링과 연기 확산, 이동을 보기 위한 변수 초기화
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

		// 연기 발생/유지/소멸을 알려 주기 위한 변수 초기화
		beforeSmokeOccur = false;
		nowSmokeOccur = false;

		// 프레임 처리 순서 초기화
		firstFrameProcessed = false;
		secondFrameProcessed = false;
	}

	// 메모리 해제
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

	// 그리기 함수
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

	// 알고리즘
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

		// 새로운 배경과 배경 임계치 값을 갱신
		cvSetZero (pxlMove);

		// y는 높이, x는 너비이며, bgrbgr... 식이므로 3*x으로 픽셀의 각 채널에 접근
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

				// 픽셀의 움직임을 결정
				if ((diffnp1_ptr[x] > Tn_ptr[x]) && (diffnp2_ptr[x] > Tn_ptr[x]))
					pxlMove_ptr[x] = 1;
				else
					pxlMove_ptr[x] = 0;

				// 픽셀 배경과 임계치를 갱신
				// 움직이지 않으면,
				if (pxlMove_ptr[x] == 0) {

					Bnp1_ptr[x] = alpha*Bn_ptr[x] + (1-alpha)*cGFrame_ptr[x];
					Tnp1_ptr[x] = alpha*Tn_ptr[x] + (1-alpha)*5*diffImg_ptr[x];

				// 움직이면,
				} else {

					Bnp1_ptr[x] = Bn_ptr[x];
					Tnp1_ptr[x] = Tn_ptr[x];
				}

				// 객체와 배경을 분리, 블록 처리를 위해
				// 배경 픽셀이면,
				if (diffImg_ptr[x] < Gap) {

					bgnSum += diffImg_ptr[x];
					bgnCnt ++;
				} else {

					objSum += diffImg_ptr[x];
					objCnt ++;
				}
			}
		}

		// 객체와 배경의 평균값의 중간
		objAvg = objSum / objCnt;
		bgnAvg = bgnSum / bgnCnt;
		Tb = (objAvg + bgnAvg) / 2;

		// 움직인 불록을 결정한다.
		// 밝기 값을 사용해서 움직인 블록과 움직이지 않은 블록을 결정
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

				// blkAvg가 Tb보다 크면 arrMove(y,x)는 1, 아니면 0
				if (blkAvg > Tb)
					arrMove[y][x] = 1;
				else
					arrMove[y][x] = 0;
			}
		}

		// 메모리 해제
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

		int Tb = blocksize * blocksize / 4 * 3;	// 블록 안에 해당 픽셀의 개수가 Tb를 넘는지 여부를 검사함

		int maxComponent, minComponent, diffOfComponents, avgOfComponents;

		for (y = 0 ; y < bnoRow ; y++) {

			yStart = y*blocksize+1;
			yEnd = (y+1)*blocksize;

			for (x = 0 ; x < bnoCol ; x++) {

				// 움직임 블록이 아니면 제외
				if (arrMove[y][x] == 0)
					continue;

				xStart = x*blocksize+1;
				xEnd = (x+1)*blocksize;

				iNumOfSmokePxl = 0;	// 블록 내 연기 픽셀의 수

				for (j = yStart ; j <= yEnd ; j++) {

					uchar* cFrame_ptr = (uchar*)(cFrame->imageData + j * cFrame->widthStep);

					for (i = xStart ; i <= xEnd ; i++) {

						maxComponent = MaxOfThree (cFrame_ptr[3*i], cFrame_ptr[3*i+1], cFrame_ptr[3*i+2]);
						minComponent = MinOfThree (cFrame_ptr[3*i], cFrame_ptr[3*i+1], cFrame_ptr[3*i+2]);
						diffOfComponents = maxComponent - minComponent;
						avgOfComponents = AvgOfThree (cFrame_ptr[3*i], cFrame_ptr[3*i+1], cFrame_ptr[3*i+2]);

						// R,G,B 중 제일 큰 것과 제일 작은 것의 차이가 15 이내이면서 intensity(R,G,B의 평균)가 100~255 사이인 경우
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
		// 연기 덩어리가 확산하는지 알려줌 (확산은 1, 축소는 -1, 정체는 0)
		int isFilled = smokeBlockNumWindow->fillQueue (maxLabelSize);

		if (isFilled)
			return smokeBlockNumWindow->checkTrend (smokeBlockNumThreshold);
		else
			return 0;
	}
	int isShortSmokeDiffusing (int maxLabelSize) {
		// 연기 덩어리가 확산/축소하는지 알려줌 (확산/축소는 1, 정체는 0)
		int isFilled = smokeBlockNumWindow->fillQueue (maxLabelSize);

		if (isFilled)
			return smokeBlockNumWindow->checkPartTrend (smokeBlockNumThreshold, smokeBlockNumChangeCountThreshold, subQueueLength);
		else
			return 0;
	}
	int isSmokeOccur (int diffuseTrend) {
		// 연기가 발생하고 있는 상황이 아니면 0, 발생하고 있다면 0보다 큰 값을 리턴
		if (diffuseTrend == 1)
			smokeExistWindow->fillQueue (1);
		else
			smokeExistWindow->fillQueue (0);

		return smokeExistWindow->sumOfElements ();
	}
	int isSmokeMassMoving (int smokeColorBlockExpanding, int diffuseTrend) {
		// 연기가 이동하는지 여부를 확인한다. (이동은 1, 복귀는 -1, 정체는 0)
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
			// arrLabels에서 가장 큰 연기 덩어리의 좌우(열) 중점을 찾아냄
			for (int j=0 ; j < bnoRow ; j++) {

				isFoundXLine = false;
				isFoundStartPoint = false;
				isFoundEndPoint = false;

				for (int i=0 ; i < bnoCol ; i++) {
					// 연기 행이 처음 발견되면 연기 행 수를 센 후에
					if (arrLabels[j][i] != 0) {
						if (!isFoundXLine) {
							isFoundXLine = true;
							smokeRow++;
						}

						// 처음 점을 찾고
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

						// 마지막 점을 찾고
						if (!isFoundEndPoint) {
							isFoundEndPoint = true;
							end_x = (bnoCol-1)-i;
						}
					}

					// 두 점의 인덱스 값을 2로 나누어 중점을 구하고 값을 누적함
					if (isFoundStartPoint && isFoundEndPoint) {
						center_x = (start_x + end_x) / 2.0;
						acc_x += center_x;
					}
				}
			}

			// 중점 좌표를 누적한 값을 연기 행 수로 나누면 좌우(열) 평균 중점을 구할 수 있다.
			if (smokeRow != 0)
				avg_center_x = acc_x / (double)smokeRow;

			// arrLabels에서 가장 큰 연기 덩어리의 상하(행) 중점을 찾아냄
			for (int i=0 ; i < bnoCol ; i++) {

				isFoundYLine = false;
				isFoundStartPoint = false;
				isFoundEndPoint = false;

				for (int j=0 ; j < bnoRow ; j++) {
					// 연기 열이 처음 발견되면 연기 행 수를 센 후에
					if (arrLabels[j][i] != 0) {
						if (!isFoundYLine) {
							isFoundYLine = true;
							smokeCol++;
						}

						// 처음 점을 찾고
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

						// 마지막 점을 찾고
						if (!isFoundEndPoint) {
							isFoundEndPoint = true;
							end_y = (bnoRow-1)-j;
						}
					}

					// 두 점의 인덱스 값을 2로 나누어 중점을 구하고 값을 누적함
					if (isFoundStartPoint && isFoundEndPoint) {
						center_y = (start_y + end_y) / 2.0;
						acc_y += center_y;
					}
				}
			}

			// 중심 좌표를 누적한 값을 연기 열 수로 나누면 상하(행) 평균 중점을 구할 수 있다.
			if (smokeCol != 0)
				avg_center_y = acc_y / (double)smokeCol;
		}

		if (smokeRow == 0 || smokeCol == 0)
			return 0;

		// 연기가 처음 등장하게 되면 초기점을 설정함
		if (beforeSmokeColorBlockExpanding == 0 && smokeColorBlockExpanding == 1) {
			initPtX = avg_center_x;
			initPtY = avg_center_y;
		}

		bool isFilled = false;

		// growthTrend가 1일 때만 centerPointWindow에 현재 연기 덩어리의 중점과 초기중점 간의 거리를 넣는다.
		if (diffuseTrend == 1) {
			// 피타고라스의 정리를 이용하여 초기점과 새로운 중점 간의 거리를 구함
			double distance = sqrt( (avg_center_x - initPtX)*(avg_center_x - initPtX) + (avg_center_y - initPtY)*(avg_center_y - initPtY) );
			isFilled = centerPointWindow->fillQueue (distance);
		}

		// 연기의 초기 중점과 나중 중점 간의 거리가 증가하면 중심점이 이동했음을 의미하므로 연기가 확산되었음을 의미함
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
		char* pData = new char[bnoRow * bnoCol];	// 영상을 1차원으로 옮긴 것
		char* labels = new char[bnoRow * bnoCol];	// pData 영상의 각 픽셀에 레이블 번호를 매긴 것 (레이블 번호는 1부터 시작)

		// 레이블링 하기 전에 int** 블록 배열을 IplImage*로 만들어야 함
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

		// 각 레이블의 크기를 측정
		if (sizeOfEachObj == NULL) {
			sizeOfEachObj = new int[numOfObj+1];
			for (int i=0 ; i < numOfObj+1 ; i++)
				sizeOfEachObj[i] = 0;
		}
		for (int i=0 ; i < (bnoRow * bnoCol) ; i++)
			if (labels[i] > 0)
				sizeOfEachObj[labels[i]]++;

		// 가장 큰 레이블의 크기와 번호를 알아낸다
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

		// 가장 큰 크기의 레이블만 남겨서 2차원 공간으로 복사
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

		// labels에서 maxLabelIndex에 해당하는 부분만 추출해서 2차원 이진 이미지로 만든다.
		for (int j=0 ; j < bnoRow ; j++)
			for (int i=0 ; i < bnoCol ; i++)
				if (labels[j*bnoCol + i] == maxLabelIndex)
					arrLabels[j][i] = labels[j*bnoCol + i];

		// 연기의 확산 여부 확인
		if (longSmokeDiffuseTrend > shortSmokeDiffuseTrend)
			isSmoke = (isSmokeMassMoving (smokeColorBlockExpanding, longSmokeDiffuseTrend) == 1) ? true : false;
		else
			isSmoke = (isSmokeMassMoving (smokeColorBlockExpanding, shortSmokeDiffuseTrend) == 1) ? true : false;

		// smokeExistWindow의 요소 합을 이전 상태로 보관
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
			//numOfSmokeFrame++;	// ??? 성능평가용 코드
		} else {
			if (smokePassFrame == NULL)
				smokePassFrame = cvCreateImage (cvGetSize(cFrame), cFrame->depth, cFrame->nChannels);
			cvCopy (cFrame, smokePassFrame);
		}

		release1DArray (pData);
		release1DArray (labels);
		release1DArray (sizeOfEachObj);
	}

	// 프레임 처리
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

		// 움직임 픽셀 블록을 위한 데이터
		if (Tn == NULL)		Tn = cvCreateImage (cvGetSize(ppGFrame), ppGFrame->depth, ppGFrame->nChannels);
		if (Tnp1 == NULL)	Tnp1 = cvCreateImage (cvGetSize(ppGFrame), ppGFrame->depth, ppGFrame->nChannels);
		if (Bn == NULL)		Bn = cvCreateImage (cvGetSize(ppGFrame), ppGFrame->depth, ppGFrame->nChannels);
		if (Bnp1 == NULL)	Bnp1 = cvCreateImage (cvGetSize(ppGFrame), ppGFrame->depth, ppGFrame->nChannels);

		cvSet (Tn, cvScalar(5, 0, 0, 0));	// 임계치 설정

		cvCopy (ppGFrame, Bn);	// 배경 영상 초기화
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

	// 산술 함수
	int MaxOfThree (int x, int y, int z)	{ return (x > y) ? ((x > z) ? x :  z) : ((y > z) ? y : z); }
	int MinOfThree (int x, int y, int z)	{ return (x < y) ? ((x < z) ? x : z) : ((y < z) ? y : z); }
	int AvgOfThree (int x, int y, int z)	{ return ((x+y+z)/3); }

	// 레이블링 알고리즘
	int findNeighboringLabel (int lset[], int x) {
		// 레이블 테이블에서 x번 레이블이 존재하지 않는다면 연결된 레이블 번호를 알려주고, 존재한다면 그대로 출력한다.
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
		A는 중앙 픽셀, connectedness는 3가지 버전이 있다.
		4: A, B, C가 연결되어 있음
		6: A, B, C, D가 연결되어 있음
		8: A, B, C, D, E가 연결되어 있음
		input:
			img -- 그레이스케일 이미지 혹은 바이너리 이미지 (픽셀 값이 0이면 물체가 없는 것으로 인식함, 0이 아니면 물체로 인식)
			labels -- 픽셀의 레이블을 의미하며 최종적으로 해당 픽셀이 속한 레이블 번호를 의미함, 값은 1 이상의 정수, labels[row * col]
			n -- n 연결
		output:
			연결된 영역의 수
		*/
		int imgRow = img->height;
		int imgCol = img->width;
		int imgRowCol = imgRow * imgCol;

		// 레이블 결과, 함수 종료 후에는 결과가 여기에 보관됨
		memset (labels, 0, imgRowCol * sizeof(char));
		int numOfObj = 0;

		// 기타 변수
		int* lset = new int[imgRowCol];   // 레이블 테이블
		memset (lset, 0, imgRowCol * sizeof(int));
		int ntable = 0;	// 레이블의 길이 (레이블 번호는 1번부터 시작함)

		for (int r=0 ; r < imgRow ; r++) {
			for (int c=0 ; c < imgCol ; c++) {
				if (CV_IMAGE_ELEM(img, unsigned char, r, c)) {	// 만약 A가 물체라면
					// 이웃 픽셀 B,C,D,E를 찾아본다.
					int B, C, D, E;
					if (c == 0)					B = 0;	// 첫째 열이면 B는 없음
					else						B = findNeighboringLabel (lset, labels[r*imgCol + (c-1)]);	// (r, c-1) 픽셀의 레이블 번호를 가져옴

					if (r == 0)					C = 0;	// 첫째 줄이면 C는 없음
					else						C = findNeighboringLabel (lset, labels[(r-1)*imgCol + c]);	// (r-1, c) 픽셀의 레이블 번호를 가져옴

					if (r == 0 || c == 0)		D = 0;	// 첫째 줄, 첫째 열이면 D는 없음
					else						D = findNeighboringLabel (lset, labels[(r-1)*imgCol + (c-1)]);	// (r-1, c-1) 픽셀의 레이블 번호를 가져옴

					if (r == 0 || c == imgCol-1)		E = 0;	// 첫째 ㅈ줄, 마지막 열이면 E는 없음
					else						E = findNeighboringLabel (lset, labels[(r-1)*imgCol + (c+1)]);	// (r-1, c+1) 픽셀의 레이블 번호를 가져옴

					if (n == 4) {
						// 4 connectedness 적용
						if (B && C) {	// B와 C가 레이블을 가지고 있다면
							if (B == C)
								labels[r*imgCol + c] = B;	// B, C가 연결된 경우 (r, c) 픽셀, 즉 A의 레이블은 B와 연결된다. (A-B-C 연결)
							else {
								lset[C] = B;			// B, C가 연결되지 않은 경우 C의 레이블은 B와 연결되고
								labels[r*imgCol + c] = B;	// A의 레이블 역시 B와 연결된다. (A-B-C 연결)
							}
						} else if (B)	// B가 물체, C가 물체 아니면
							labels[r*imgCol + c] = B;		// A-B 연결
						else if (C)	// C가 물체, B가 물체 아니면
							labels[r*imgCol + c] = C;		// A-C 연결
						else {		// B, C, D가 물체가 아니라면 - 새로운 물체
							// 레이블을 붙이고 테이블에 넣음
							ntable++;	// 테이블의 번호를 새로 할당하고
							labels[r*imgCol + c] = lset[ntable] = ntable;	// 새로운 번호를 lable set에 배정하고 labels의 (r, c) 픽셀에도 할당한다.
						}
					} else if (n == 6) {
						// 6 connectedness 적용
						if (D)	// D 물체
							labels[r*imgCol + c] = D;		// A-D 연결
						else if (B && C) {		// B와 C가 레이블을 가지고 있다면
							if (B == C)
								labels[r*imgCol + c] = B;	// A-B 연결
							else {
								int tlabel = (((B) < (C)) ? (B) : (C));	// 작은 레이블 번호를 얻어와서 B,C에 같이 할당하고
								lset[B] = tlabel;
								lset[C] = tlabel;
								labels[r*imgCol + c] = tlabel;	// A의 레이블 번호도 B, C와 동일 (A-B-C 연결)
							}
						} else if (B)		// B는 물체, C가 물체 아니면,
							labels[r*imgCol + c] = B;		// A-B 연결
						else if (C)		// C는 물체 B가 물체 아니면,
							labels[r*imgCol + c] = C;		// A-C 연결
						else {			// B, C, D가 물체가 아니라면 - 새로운 물체
							// 레이블을 붙이고 테이블에 넣음
							ntable++;	// 테이블의 번호를 새로 할당하고
							labels[r*imgCol + c] = lset[ntable] = ntable;	// 새로운 번호를 lable set에 배정하고 labels의 (r, c) 픽셀에도 할당한다.
						}
					} else if (n == 8) {
						// 8 connectedness 적용
						if (B || C || D || E) {
							int tlabel = B;

							// B, C, D, E 순서대로 돌면서 가장 작은 레이블 번호를 얻어냄
							if (B)			tlabel = B;
							else if (C)		tlabel = C;
							else if (D)		tlabel = D;
							else if (E)		tlabel = E;

							labels[r*imgCol + c] = tlabel;	// A의 레이블 번호를 할당

							// A와 연결된 B, C, D, E를 모두 동일한 레이블 번호로 통합
							if (B && B != tlabel)	lset[B] = tlabel;
							if (C && C != tlabel)	lset[C] = tlabel;
							if (D && D != tlabel)	lset[D] = tlabel;
							if (E && E != tlabel)	lset[E] = tlabel;
						} else {
							// 레이블을 붙이고 테이블에 넣음
							ntable++;	// 테이블의 번호를 새로 할당하고
							labels[r*imgCol + c] = lset[ntable] = ntable;	// 새로운 번호를 lable set에 배정하고 labels의 (r, c) 픽셀에도 할당한다.
						}
					}
				} else
					labels[r*imgCol + c] = 0;	// A는 물체가 아니므로 넘어감
			}
		}

		// 요소 테이블 정리하기
		for (int i=0 ; i <= ntable ; i++)
			lset[i] = findNeighboringLabel (lset, i);	// i번 레이블 번호가 i번이면 그대로 유지되고, 그렇지 않으면 연결된 레이블 번호를 저장함 (초기화 전에는 레이블 번호들이 보관되어 있음)

		// 룩업 테이블을 통해 이미지 실행하기 (1차 스캔)
		for (int r=0 ; r < imgRow ; r++)
			for (int c=0 ; c < imgCol ; c++)
				labels[r*imgCol + c] = lset[labels[r*imgCol + c]];	// (r, c) 픽셀의 레이블 번호가 레이블 테이블에 있으면 (r, c) 픽셀의 레이블 번호를 레이블 테이블의 것으로 할당함 (작은 번호일 수도 있음)

		// 이미지 속 물체 수 세어보기
		for (int i=0 ; i <= ntable ; i++)
			lset[i] = 0;	// 레이블 테이블 초기화
		for (int r=0 ; r < imgRow ; r++)
			for (int c=0 ; c < imgCol ; c++)
				lset[labels[r*imgCol + c]]++;	// 레이블 테이블의 각 번호마다 각 레이블 번호의 픽셀 개수를 보관함 (초기화 후에는 각 인덱스에 해당하는 레이블 번호의 픽셀 개수를 의미함)

		// number the objects from 1 through n objects
		lset[0] = 0;
		for (int i=1 ; i <= ntable ; i++)
			if (lset[i] > 0)
				lset[i] = ++numOfObj;	// 만약 i번 레이블에 해당하는 픽셀이 0개보다 많다면 i번부터 순서대로 레이블 별로 물체 개수를 할당함 (레이블 테이블은 결과적으로 0, 1, 2, 3, ...가 된다)

		// 룩업 테이블을 통해 이미지 실행하기 (2차 스캔)
		for (int r=0 ; r < imgRow ; r++)
			for (int c=0 ; c < imgCol ; c++)
				labels[r*imgCol + c] = lset[labels[r*imgCol + c]];	// 최종적으로 레이블의 각 픽셀은 자신이 속한 레이블 번호가 할당됨

		delete lset;
		return numOfObj;
	}

public:
	/*
	// ??? 성능평가용 코드
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

		//frameSeq++;	// ??? 성능평가용 코드

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
		// 연기가 나타났을 때 1, 소멸했을 때 -1, 유지 중에는 0 리턴
		if (isSmoke == true)
			nowSmokeOccur = true;

		if (smokeExistWindow->sumOfElements () == 0)
			nowSmokeOccur = false;

		bool smokeAppeared = (!beforeSmokeOccur && nowSmokeOccur);
		bool smokeDisappeared = (beforeSmokeOccur && !nowSmokeOccur);

		beforeSmokeOccur = nowSmokeOccur;

		if (smokeAppeared) {
			//startSmokeSeq = frameSeq;		// ??? 성능평가용 코드
			return 1;
		}
		if (smokeDisappeared) {
			//endSmokeSeq = frameSeq;		// ??? 성능평가용 코드
			return -1;
		}

		return 0;
	}
};