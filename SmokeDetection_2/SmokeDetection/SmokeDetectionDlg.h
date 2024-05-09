
// SmokeDetectionDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "SmokeDetectionCore.h"


// CSmokeDetectionDlg 대화 상자
class CSmokeDetectionDlg : public CDialogEx
{
// 생성입니다.
public:
	CSmokeDetectionDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SMOKEDETECTION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_stScreen;
	CButton m_stopButton;
	CButton m_camButton;
	CButton m_fileButton;
	CListBox m_warningList;

	CvCapture* m_pCapture;
	IplImage* m_pImage;
	IplImage* m_rImage;
	CvvImage m_cImage;
	SmokeDetection* m_smoke;
	int frame;
	int smoke_frame;

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnClickedButtonInputCam();
	afx_msg void OnClickedButtonInputFile();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonListClear();
};
