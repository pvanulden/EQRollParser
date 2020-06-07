// EQRollParserDlg.h : header file
//

#pragma once
#include "afxcmn.h"

typedef struct 
{
	CString strName;
	INT nRoll;
	INT nRangeLow;
	INT nRangeHigh;
	COleDateTime dtTime;
} SRollRecord;

// CEQRollParserDlg dialog
class CEQRollParserDlg : public CDialog
{
// Construction
public:
	CEQRollParserDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EQROLLPARSER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnHelpAbout();
	afx_msg void OnFileExit();
	afx_msg void OnBnClickedOk();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();

	void PrintRolls( CString strTimeStamp, CArray<SRollRecord,SRollRecord>& arrRolls );
	void AddText( INT nIDC, CString strText );
	void EnableWindows( BOOL b );
	void ThreadStartup();
	void ThreadCleanup();

	CString m_strLogPath;
	CProgressCtrl m_ctrlProgress;
	BOOL m_bCancel;
	BOOL m_bExiting;
	CWinThread* m_pReadThread;
	DWORD m_dwFileReadSize;
	int m_nReadWholeFile;
	CString m_strSaveCaption;
	CString m_strSearch;
};
