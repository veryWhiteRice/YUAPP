
// JH_TCP_ServerDlg.h: 헤더 파일
//

#pragma once
#include "pch.h"
#include "framework.h"
#include "afxwin.h"
#include "afxcoll.h"
struct ThreadArg
{
	CStringList* pList;//Message 담는 List형 함수
	CDialogEx* pDlg;
	int Thread_run;// Thread_run 실행
};
class CDataSocket;// Client에게 Message 전달하는 용도
class CListenSocket; // Server 여는 Socket
// CJHTCPServerDlg 대화 상자
class CJHTCPServerDlg : public CDialogEx
{
// 생성입니다.
public:
	CJHTCPServerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	CListenSocket* m_pListenSocket; // 서버 Socket
	CDataSocket* m_pDataSocket; // Client Socket
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_JH_TCP_SERVER_DIALOG };
#endif

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
	CEdit m_rx_edit;
	CWinThread* pThread1, * pThread2; // Thread 실행 객체
	ThreadArg arg1, arg2;//쓰레드 함수
	void ProcessAccept(int nErrorCode); // Socket Connect 요청을 받는 메서드
	void ProcessReceive(CDataSocket* pSocket, int nErrorCode); // Socket Message를 받는 메서드
	void ProcessClose(CDataSocket* pSocket, int nErrorCode); // Socket Close 메서드
};
