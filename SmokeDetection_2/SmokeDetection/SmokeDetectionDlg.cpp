
// SmokeDetectionDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "SmokeDetection.h"
#include "SmokeDetectionDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSmokeDetectionDlg 대화 상자




CSmokeDetectionDlg::CSmokeDetectionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSmokeDetectionDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSmokeDetectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_SCREEN, m_stScreen);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_stopButton);
	DDX_Control(pDX, IDC_BUTTON_INPUT_CAM, m_camButton);
	DDX_Control(pDX, IDC_BUTTON_INPUT_FILE, m_fileButton);
	DDX_Control(pDX, IDC_LIST_WARNING, m_warningList);
}

BEGIN_MESSAGE_MAP(CSmokeDetectionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_INPUT_CAM, &CSmokeDetectionDlg::OnClickedButtonInputCam)
	ON_BN_CLICKED(IDC_BUTTON_INPUT_FILE, &CSmokeDetectionDlg::OnClickedButtonInputFile)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CSmokeDetectionDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_LIST_CLEAR, &CSmokeDetectionDlg::OnBnClickedButtonListClear)
END_MESSAGE_MAP()


// CSmokeDetectionDlg 메시지 처리기

BOOL CSmokeDetectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_stopButton.EnableWindow (FALSE);
	m_pCapture = 0;
	m_pImage = 0;

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CSmokeDetectionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CSmokeDetectionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		if (m_pImage) {
			CDC* pDC;
			CRect rect;

			pDC = m_stScreen.GetDC ();
			m_stScreen.GetClientRect (&rect);

			m_smoke->detectSmoke (m_pImage);

			m_rImage = m_smoke->getSmokePassFrame ();
			m_cImage.CopyOf (m_rImage);
			m_cImage.DrawToHDC (pDC->m_hDC, rect);

			int smokeState = m_smoke->isSmokeEvent ();

			/*
			// ??? 성능평가용 코드
			CString resultString;
			resultString.Format (_T("# %d / start: %d / end: %d / smokeNum: %d"), m_smoke->frameSeq, m_smoke->startSmokeSeq, m_smoke->endSmokeSeq, m_smoke->numOfSmokeFrame);
			m_warningList.DeleteString (0);
			m_warningList.AddString (resultString);
			*/

			if (smokeState == 1) {
				m_warningList.AddString (_T("연기가 발생했습니다."));
			} else if (smokeState == -1) {
				m_warningList.AddString (_T("연기가 소멸했습니다."));
			}

			ReleaseDC (pDC);
		}

		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CSmokeDetectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSmokeDetectionDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pCapture) {
		m_pImage = cvQueryFrame (m_pCapture);
		Invalidate (FALSE);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CSmokeDetectionDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
}


void CSmokeDetectionDlg::OnClickedButtonInputCam()
{
	// TODO: Add your control notification handler code here
	m_pCapture = cvCaptureFromCAM (0);
	if (!m_pCapture) {
		MessageBox (_T("캠 장치가 없습니다."), _T("cvCaptureFromCAM () 오류"), MB_ICONERROR);
		return;
	}

	m_camButton.EnableWindow (FALSE);
	m_fileButton.EnableWindow (FALSE);
	m_stopButton.EnableWindow (TRUE);
	m_smoke = new SmokeDetection ();
	SetTimer (10, 30, NULL);
}


void CSmokeDetectionDlg::OnClickedButtonInputFile()
{
	// TODO: Add your control notification handler code here
	int i;
	CFileDialog fileDlg (TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, _T("avi 파일(*.avi)|*.avi"));
	if (fileDlg.DoModal () != IDOK) {
		MessageBox (_T("파일을 선택하세요."));
		return;
	}
	CString cFilename = fileDlg.GetPathName ();
	char filename[255];
	for (i=0 ; i < cFilename.GetLength() ; i++) {
		filename[i] = cFilename[i];
	}
	filename[i] = '\0';

	m_pCapture = cvCaptureFromFile (filename);
	if (!m_pCapture) {
		MessageBox (_T("파일이 없습니다."), _T("cvCaptureFromFile () 오류"), MB_ICONERROR);
		return;
	}

	m_camButton.EnableWindow (FALSE);
	m_fileButton.EnableWindow (FALSE);
	m_stopButton.EnableWindow (TRUE);
	m_smoke = new SmokeDetection ();
	SetTimer (10, 30, NULL);
}


void CSmokeDetectionDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	m_camButton.EnableWindow (TRUE);
	m_fileButton.EnableWindow (TRUE);
	m_stopButton.EnableWindow (FALSE);

	KillTimer (10);

	delete m_smoke;

	if (m_pCapture) {
		cvReleaseCapture (&m_pCapture);
		m_pCapture = 0;
	}
}


void CSmokeDetectionDlg::OnBnClickedButtonListClear()
{
	// TODO: Add your control notification handler code here
	m_warningList.ResetContent ();
}
