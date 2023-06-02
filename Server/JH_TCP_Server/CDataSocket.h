#pragma once
#include <afxsock.h>
class CJHTCPServerDlg;
class CDataSocket :
    public CSocket
{
public:
	CDataSocket(CJHTCPServerDlg* pDlg);
	virtual ~CDataSocket();
	CJHTCPServerDlg* m_pDlg;
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
};

