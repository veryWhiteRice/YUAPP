#include "pch.h"
#include "CListenSocket.h"
#include "framework.h"
#include "JH_TCP_Server.h"
#include "JH_TCP_ServerDlg.h"

CListenSocket::CListenSocket(CJHTCPServerDlg* pDlg)
{
	m_pDlg = pDlg;
}

CListenSocket::~CListenSocket()
{
}


// CListenSocket 멤버 함수


void CListenSocket::OnAccept(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CSocket::OnAccept(nErrorCode);
	m_pDlg->ProcessAccept(nErrorCode);
}