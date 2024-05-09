
// SmokeDetectionDlg.h : ��� ����
//

#pragma once
#include "afxwin.h"
#include "SmokeDetectionCore.h"


// CSmokeDetectionDlg ��ȭ ����
class CSmokeDetectionDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CSmokeDetectionDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_SMOKEDETECTION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
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
