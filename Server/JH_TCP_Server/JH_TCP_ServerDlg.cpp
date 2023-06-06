
// JH_TCP_ServerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "JH_TCP_Server.h"
#include "JH_TCP_ServerDlg.h"
#include "afxdialogex.h"
#include "CListenSocket.h"
#include "CDataSocket.h"
#include "windows.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define MAX_BUF 1000
CCriticalSection tx_cs;
CCriticalSection rx_cs;
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CJHTCPServerDlg 대화 상자



CJHTCPServerDlg::CJHTCPServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_JH_TCP_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJHTCPServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_rx_edit);
}

BEGIN_MESSAGE_MAP(CJHTCPServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

UINT RXThread(LPVOID arg)	//Receive Thread 
{
	ThreadArg* pArg = (ThreadArg*)arg; // Thread 실행하는 arg를 가지고 온다.(arg2를 가르킴)
	CStringList* plist = pArg->pList; // arg2의 pList를 가지고 온다.
	CJHTCPServerDlg* pDlg = (CJHTCPServerDlg*)pArg->pDlg;
	while (pArg->Thread_run) //Thread가 돌 때
	{
		POSITION pos = plist->GetHeadPosition(); //가장 최상단을 가르킴
		POSITION current_pos;
		while (pos != NULL) //현재 가르키는 곳에 문자열이 있다면
		{
			current_pos = pos; // 현재 가르키는 곳으로 Update
			rx_cs.Lock();	//SenderTextBox의 임계구역을 Lock
			CString str = plist->GetNext(pos); // pList 안에서 해당 부분에 있는 문자열을 가지고 온다.
			rx_cs.Unlock();
			char* txbuf = LPSTR(LPCTSTR(str)); // CString -> String 형 변환
			CString message;
			pDlg->m_rx_edit.GetWindowText(message);	//메세지 받음
			message += str; // 문자열 복사
			message += "\r\n";

			pDlg->m_rx_edit.SetWindowText(message);	//SenderTextBox에 메세지 출력
			pDlg->m_rx_edit.LineScroll(pDlg->m_rx_edit.GetLineCount());

			plist->RemoveAt(current_pos);
			char* str2 = (char*)malloc(sizeof(char) * MAX_BUF);
			str2 = "119: Emergency\n";
			pDlg->m_pDataSocket->Send(str2, (int)strlen(str2)); // 해당 Socket과 연결된 클라이언트에게 Message 전송
		}
		Sleep(10);
	}
	return 0;
}
// CJHTCPServerDlg 메시지 처리기

BOOL CJHTCPServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	CStringList* newlist = new CStringList;// newlist 초기화
	arg1.pList = newlist; // newlist를 arg1 구조체 List로 사용
	arg1.Thread_run = 1;	//스레드 현재 상태
	arg1.pDlg = this;

	CStringList* newlist2 = new CStringList;// newlist2 초기화
	arg2.pList = newlist2; // newlist2를 arg2 구조체 List로 사용
	arg2.Thread_run = 1;	//스레드 현재 상태
	arg2.pDlg = this;
	WSADATA wsa;
	int error_code;
	if ((error_code = WSAStartup(MAKEWORD(2, 2), &wsa)) != 0) {
		TCHAR buffer[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 256, NULL);
		AfxMessageBox(buffer, MB_ICONERROR);
	}
	m_pListenSocket = NULL;

	ASSERT(m_pListenSocket == NULL);
	m_pListenSocket = new CListenSocket(this); // Socket 초기화
	if (m_pListenSocket->Create(51000)) {	//소켓 포트번호 설정(51000)
		if (m_pListenSocket->Listen()) //상대 Socket이 Connect할 때까지 대기
		{
			AfxMessageBox(_T("서버를 시작합니다."), MB_ICONINFORMATION);
			pThread2 = AfxBeginThread(RXThread, (LPVOID)&arg2); // Thread 실행
			return TRUE;
		}
	}
	else {
		int err = m_pListenSocket->GetLastError();
		TCHAR buffer[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 256, NULL);
		AfxMessageBox(buffer, MB_ICONERROR);
	}
	AfxMessageBox(_T("이미 실행 중인 서버가 있습니다.") _T("\n프로그램을 종료합니다."), MB_ICONERROR);
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CJHTCPServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CJHTCPServerDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CJHTCPServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CJHTCPServerDlg::ProcessAccept(int nErrorCode) // Socket 연결 요청 수락
{
	CString PeerAddr; // 접근자 주소
	UINT PeerPort; // 접근자 포트

	m_pDataSocket = NULL;

	ASSERT(nErrorCode == 0);
	if (m_pDataSocket == NULL) {
		m_pDataSocket = new CDataSocket(this);// DataSocket 초기화
		if (m_pListenSocket->Accept(*m_pDataSocket)) // DataSocket을 이용해 접근자와 연결
		{
			m_pDataSocket->GetPeerName(PeerAddr, PeerPort);
		}
		else {
			delete m_pDataSocket;
			m_pDataSocket = NULL;
		}
	}
}

void CJHTCPServerDlg::ProcessReceive(CDataSocket* pSocket, int nErrorCode) // 메세지를 받을 경우
{
	char pBuf[MAX_BUF];
	CString strData = _T("");
	int nbytes;

	nbytes = pSocket->Receive(pBuf, 1024); // Socket을 통해 Message를 받음
	pBuf[nbytes] = NULL; // Buf 끝 부분에 NULL 값 추가
	strData = (LPCSTR)(LPSTR)pBuf; // CString으로 형 변환

	rx_cs.Lock(); // 임계구역 설정
	arg2.pList->AddTail(strData); // arg2 List에 Receive Message 추가
	rx_cs.Unlock(); // 임계구역 해제
}
//프로세스를 종료하였을 경우
void CJHTCPServerDlg::ProcessClose(CDataSocket* pSocket, int nErrorCode) {
	pSocket->Close();
	delete m_pDataSocket;
	m_pDataSocket = NULL;

	int len = m_rx_edit.GetWindowTextLengthW();
	CString message = _T("### 접속 종료 ###\r\n");
	m_rx_edit.SetSel(len, len);
	m_rx_edit.ReplaceSel(message);
}
