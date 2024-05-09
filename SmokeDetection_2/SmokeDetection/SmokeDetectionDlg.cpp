
// SmokeDetectionDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "SmokeDetection.h"
#include "SmokeDetectionDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CSmokeDetectionDlg ��ȭ ����




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


// CSmokeDetectionDlg �޽��� ó����

BOOL CSmokeDetectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	m_stopButton.EnableWindow (FALSE);
	m_pCapture = 0;
	m_pImage = 0;

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CSmokeDetectionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
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
			// ??? �����򰡿� �ڵ�
			CString resultString;
			resultString.Format (_T("# %d / start: %d / end: %d / smokeNum: %d"), m_smoke->frameSeq, m_smoke->startSmokeSeq, m_smoke->endSmokeSeq, m_smoke->numOfSmokeFrame);
			m_warningList.DeleteString (0);
			m_warningList.AddString (resultString);
			*/

			if (smokeState == 1) {
				m_warningList.AddString (_T("���Ⱑ �߻��߽��ϴ�."));
			} else if (smokeState == -1) {
				m_warningList.AddString (_T("���Ⱑ �Ҹ��߽��ϴ�."));
			}

			ReleaseDC (pDC);
		}

		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
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
		MessageBox (_T("ķ ��ġ�� �����ϴ�."), _T("cvCaptureFromCAM () ����"), MB_ICONERROR);
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
	CFileDialog fileDlg (TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, _T("avi ����(*.avi)|*.avi"));
	if (fileDlg.DoModal () != IDOK) {
		MessageBox (_T("������ �����ϼ���."));
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
		MessageBox (_T("������ �����ϴ�."), _T("cvCaptureFromFile () ����"), MB_ICONERROR);
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
