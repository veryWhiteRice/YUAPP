#pragma once
#include <afxsock.h>
class CJHTCPServerDlg;
class CListenSocket :
    public CSocket
{
public:
	CListenSocket(CJHTCPServerDlg* pDlg);
	virtual ~CListenSocket();
	CJHTCPServerDlg* m_pDlg;
	virtual void OnAccept(int nErrorCode);
};

