// EQRollParserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EQRollParser.h"
#include "EQRollParserDlg.h"
#include "atlbase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define REG_ROOT_PATH		_T("Software\\Bruenorg\\EQRollParser")
#define REG_KEY_LOGPATH		_T("LogPath")
#define REG_KEY_READSIZE	_T("ReadSize")
#define REG_KEY_READALL		_T("ReadAll")
#define REG_KEY_SEARCH		_T("Search")

#define ROLL_START_PATTERN	_T("----------------")
#define ROLL_START			_T("**A Magic Die is rolled by")
#define ROLL_ACTUAL			_T("**It could have been any number from")
#define LOOT_PATTERN		_T(" looted a ")

UINT LogFileReadThread( LPVOID pParam );


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CEQRollParserDlg dialog




CEQRollParserDlg::CEQRollParserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEQRollParserDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pReadThread = NULL;
	m_strLogPath = _T("");
	m_strSearch = _T("");
	m_bCancel = FALSE;
	m_dwFileReadSize = 0;
	m_nReadWholeFile = 1;
	m_bExiting = FALSE;
}

void CEQRollParserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOGPATH, m_strLogPath);
	DDX_Control(pDX, IDC_READPROGRESS, m_ctrlProgress);
	DDX_Text(pDX, IDC_READSIZE, m_dwFileReadSize);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_strSearch);
}

BEGIN_MESSAGE_MAP(CEQRollParserDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP_ABOUT, &CEQRollParserDlg::OnHelpAbout)
	ON_COMMAND(ID_FILE_EXIT, &CEQRollParserDlg::OnFileExit)
	ON_BN_CLICKED(IDC_BROWSE, &CEQRollParserDlg::OnBnClickedBrowse)
	ON_BN_CLICKED(IDOK, &CEQRollParserDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, &CEQRollParserDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CEQRollParserDlg::OnBnClickedRadio2)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CEQRollParserDlg message handlers

BOOL CEQRollParserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CRegKey reg;
	if( reg.Open( HKEY_CURRENT_USER, REG_ROOT_PATH, KEY_READ ) == ERROR_SUCCESS )
	{
		ULONG ulChars;
	    if( reg.QueryStringValue( REG_KEY_LOGPATH, NULL, &ulChars ) == ERROR_SUCCESS )
	    {
			CString strLogPath;
			if( reg.QueryStringValue( REG_KEY_LOGPATH, strLogPath.GetBuffer( ulChars ), &ulChars ) == ERROR_SUCCESS )
			{
				strLogPath.ReleaseBuffer();
				m_strLogPath = strLogPath;
			}

			DWORD dwReadSize = 0;
			if( reg.QueryDWORDValue( REG_KEY_READSIZE, dwReadSize ) == ERROR_SUCCESS )
			{
				m_dwFileReadSize = dwReadSize;
			}

			DWORD dwReadWholeFile = 0;
			if( reg.QueryDWORDValue( REG_KEY_READALL, dwReadWholeFile ) == ERROR_SUCCESS )
			{
				m_nReadWholeFile = dwReadWholeFile;
			}

			if( reg.QueryStringValue( REG_KEY_SEARCH, NULL, &ulChars ) == ERROR_SUCCESS )
			{
				CString strSearch;
				if( reg.QueryStringValue( REG_KEY_SEARCH, strSearch.GetBuffer( ulChars ), &ulChars ) == ERROR_SUCCESS )
				{
					strSearch.ReleaseBuffer();
					m_strSearch = strSearch;
				}
			}

			UpdateData( FALSE );
		}

		reg.Close();
	}
	else
	{
		reg.Create( HKEY_CURRENT_USER, REG_ROOT_PATH );
	}

	CButton* pRadio1 = ( CButton* )GetDlgItem( IDC_RADIO1 );
	CButton* pRadio2 = ( CButton* )GetDlgItem( IDC_RADIO2 );
	CEdit* pReadSize = ( CEdit* )GetDlgItem( IDC_READSIZE );

	if( m_nReadWholeFile == 1 )
	{
		pRadio1->SetCheck( BST_CHECKED );
		pReadSize->EnableWindow( FALSE );
	}
	else
	{
		pRadio2->SetCheck( BST_CHECKED );
		pReadSize->EnableWindow( TRUE );
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEQRollParserDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEQRollParserDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEQRollParserDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CEQRollParserDlg::OnHelpAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CEQRollParserDlg::OnFileExit()
{
	EndDialog( 0 );
}

void CEQRollParserDlg::OnBnClickedBrowse()
{
	CFileDialog dlgBrowse( TRUE, _T("log"), _T("*.txt") );

	if( dlgBrowse.DoModal() == IDOK )
	{
		m_strLogPath = dlgBrowse.GetPathName();

		CRegKey reg;
		if( reg.Open( HKEY_CURRENT_USER, REG_ROOT_PATH, KEY_WRITE ) == ERROR_SUCCESS )
		{
			reg.SetStringValue( REG_KEY_LOGPATH, m_strLogPath );
			reg.Close();
		}

		UpdateData( FALSE );
	}
}

void CEQRollParserDlg::OnBnClickedOk()
{
	if( m_pReadThread != NULL )
	{
		m_bCancel = TRUE;
	}
	else
	{
		UpdateData();

		CRegKey reg;
		if( reg.Open( HKEY_CURRENT_USER, REG_ROOT_PATH, KEY_WRITE ) == ERROR_SUCCESS )
		{
			reg.SetDWORDValue( REG_KEY_READSIZE, m_dwFileReadSize );
			reg.SetStringValue( REG_KEY_SEARCH, m_strSearch );
			reg.Close();
		}

		if( !m_strLogPath.IsEmpty() )
		{
			ThreadStartup();
		}
	}
}

void CEQRollParserDlg::OnBnClickedRadio1()
{
	CRegKey reg;
	if( reg.Open( HKEY_CURRENT_USER, REG_ROOT_PATH, KEY_WRITE ) == ERROR_SUCCESS )
	{
		reg.SetDWORDValue( REG_KEY_READALL, 1 );
		reg.Close();
	}

	m_nReadWholeFile = 1;

	CEdit* pReadSize = ( CEdit* )GetDlgItem( IDC_READSIZE );
	pReadSize->EnableWindow( FALSE );

	UpdateData( FALSE );
}

void CEQRollParserDlg::OnBnClickedRadio2()
{
	CRegKey reg;
	if( reg.Open( HKEY_CURRENT_USER, REG_ROOT_PATH, KEY_WRITE ) == ERROR_SUCCESS )
	{
		reg.SetDWORDValue( REG_KEY_READALL, 0 );
		reg.Close();
	}

	m_nReadWholeFile = 0;

	CEdit* pReadSize = ( CEdit* )GetDlgItem( IDC_READSIZE );
	pReadSize->EnableWindow( TRUE );

	UpdateData( FALSE );
}

void CEQRollParserDlg::OnDestroy()
{
	if( m_pReadThread != NULL )
	{
		m_bExiting = TRUE;
		m_bCancel = TRUE;
	}

	INT nCount = 0;
	INT nMax = 50;

	while( m_pReadThread != NULL )
	{
		Sleep( 100 );
		nCount++;

		if( nCount == nMax )
			break;
	}

	CDialog::OnDestroy();
}

void CEQRollParserDlg::AddText( INT nIDC, CString strText )
{
	if( !m_bExiting )
	{
		CEdit* pEdit = ( CEdit* )GetDlgItem( nIDC );

		CString strCurrentText;
		pEdit->GetWindowText( strCurrentText );

		strCurrentText += strText + _T("\r\n");

		pEdit->SetWindowText( strCurrentText );
	}
}

void CEQRollParserDlg::PrintRolls( CString strTimeStamp, CArray<SRollRecord,SRollRecord>& arrRolls )
{
	CArray<SRollRecord,SRollRecord> arrSortedRolls;

	// First sort the array from highest to lowest roll.
	//
	INT i = -1;
	for( i = 0; i < arrRolls.GetSize(); i++ )
	{
		SRollRecord rollInfoNew = arrRolls.GetAt( i );

		if( arrSortedRolls.GetSize() == 0 )
		{
			arrSortedRolls.Add( rollInfoNew );
		}
		else
		{
			BOOL bInserted = FALSE;

			INT j = -1;
			for( j = 0; j < arrSortedRolls.GetSize(); j++ )
			{
				SRollRecord rollInfoCurrent = arrSortedRolls.GetAt( j );

				if( rollInfoNew.nRoll >= rollInfoCurrent.nRoll )
				{
					arrSortedRolls.InsertAt( j, rollInfoNew );
					bInserted = TRUE;
					break;
				}
			}

			if( !bInserted )
				arrSortedRolls.Add( rollInfoNew );
		}
	}

	ASSERT( arrRolls.GetSize() == arrSortedRolls.GetSize() );

	// Now print the rolls starting with the time.
	//
	AddText( IDC_ROLLS, _T("\r\n") + strTimeStamp );
	
	for( i = 0; i < arrSortedRolls.GetSize(); i++ )
	{
		SRollRecord rollInfoCurrent = arrSortedRolls.GetAt( i );

		CString strRollText;
		strRollText.Format( _T("%d) %s rolled %d (%d - %d)."), i + 1,
			rollInfoCurrent.strName, rollInfoCurrent.nRoll, rollInfoCurrent.nRangeLow, rollInfoCurrent.nRangeHigh );

		AddText( IDC_ROLLS, strRollText );
	}

}

void CEQRollParserDlg::ThreadStartup()
{
	CButton* pButton = ( CButton* )GetDlgItem( IDOK );
	pButton->GetWindowText( m_strSaveCaption );
	pButton->SetWindowText( _T("Cancel") );
	m_ctrlProgress.SetRange( 0, 100 );

	CEdit* pRolls  = ( CEdit* )GetDlgItem( IDC_ROLLS );
	CEdit* pLooted = ( CEdit* )GetDlgItem( IDC_LOOTED );
	CEdit* pSearch = ( CEdit* )GetDlgItem( IDC_SEARCH );

	pRolls->SetWindowText( _T("") );
	pLooted->SetWindowText( _T("") );
	pSearch->SetWindowTextW( _T("") );

	EnableWindows( FALSE );

	m_bCancel = FALSE;
	m_pReadThread = AfxBeginThread( LogFileReadThread, this );
}

void CEQRollParserDlg::ThreadCleanup()
{
	if( !m_bExiting )
	{
		CButton* pButton = ( CButton* )GetDlgItem( IDOK );
		pButton->SetWindowText( m_strSaveCaption );
		m_ctrlProgress.SetPos( 0 );

		EnableWindows( TRUE );
	}

	m_pReadThread = NULL;
}

void CEQRollParserDlg::EnableWindows( BOOL b )
{
	CEdit* pReadSize = ( CEdit* )GetDlgItem( IDC_READSIZE );
	CEdit* pLogPath  = ( CEdit* )GetDlgItem( IDC_LOGPATH );
	CButton* pRadio1 = ( CButton* )GetDlgItem( IDC_RADIO1 );
	CButton* pRadio2 = ( CButton* )GetDlgItem( IDC_RADIO2 );
	CButton* pBrowse = ( CButton* )GetDlgItem( IDC_BROWSE );

	pReadSize->EnableWindow( b );
	pLogPath->EnableWindow( b );
	pRadio1->EnableWindow( b );
	pRadio2->EnableWindow( b );
	pBrowse->EnableWindow( b );
}

UINT LogFileReadThread( LPVOID pParam )
{
	CEQRollParserDlg* pDlg = ( CEQRollParserDlg* )pParam;

	CStdioFile file;
	if( !file.Open( pDlg->m_strLogPath, CFile::modeRead | CFile::shareDenyNone ) )
	{
		// Failed to open the fail, exit the read thread.
		//
		pDlg->ThreadCleanup();

		return 0;
	}

	ULONGLONG ulLengthRead = 0;
	ULONGLONG ulLengthTotal = file.GetLength();
	ULONGLONG ulReadSize = ulLengthTotal;
	
	// If we aren't supposed to read the whole file, figure out how much we are
	// supposed to read and seek to that location in the file.
	//
	if( pDlg->m_nReadWholeFile == 0 )
		ulReadSize = pDlg->m_dwFileReadSize * 1000000;

	if( ( ulReadSize > 0 ) && ( ulReadSize < ulLengthTotal ) )
	{
		ULONGLONG ulSeek = ulLengthTotal - ulReadSize;
		file.Seek( ulSeek, CFile::begin );
	}
	else
	{
		ulReadSize = ulLengthTotal;
	}

	// Some handy variables.
	//
	INT nPercentRead = 0;
	INT nLines = 0;

	CString strName = _T("");
	CString strLootStart = _T("");
	CString strLootedResults = _T("");
	CString strSearchResults = _T("");
	CArray<SRollRecord,SRollRecord> arrRolls;

	CString strLine;
	while( file.ReadString( strLine ) )
	{
		// Keep track of how much we've read so we can update the progress bar.
		//
		ulLengthRead += strLine.GetLength() + 2;
		nLines++;

		if( strLine.Find( ROLL_START_PATTERN ) != -1 )
		{
			// We've found the divider pattern, print out any rolls we've read.
			//
			if( arrRolls.GetSize() > 0 )
			{
				pDlg->PrintRolls( strLootStart, arrRolls );

				arrRolls.RemoveAll();
				strLootStart.Empty();
			}
		}
		else if( strLine.Find( ROLL_START ) != -1 )
		{
			// Someone is rolling.  If this is the first roll, save the time.
			if( strLootStart.IsEmpty() )
			{
				INT nDateStampEnd = strLine.Find( _T("]") );
				strLootStart = strLine.Mid( 0, nDateStampEnd + 1 );
			}

			// Extract the player's name from the line.
			//
			INT nNameStart = strLine.Find( _T(" by ") ) + 4;
			INT nNameEnd = strLine.Find( _T(".") );

			strName = strLine.Mid( nNameStart, nNameEnd - nNameStart );
		}
		else if( strLine.Find( ROLL_ACTUAL ) != -1 )
		{
			// Extract the roll as well as the range that was rolled and save it.
			//
			SRollRecord rollRecordNew;

			INT nFrom = strLine.Find( _T(" from ") );
			INT nTo = strLine.Find( _T(" to ") );
			INT nBut = strLine.Find( _T(", but") );
			INT nA = strLine.Find( _T(" a ") );
			INT nPeriod = strLine.Find( _T(".") );

			INT nRangeLowStart = nFrom + 6;
			INT nRangeLowEnd = nTo;

			INT nRangeHighStart = nTo + 4;
			INT nRangeHighEnd = nBut;

			INT nRollStart = nA + 3;
			INT nRollEnd = nPeriod;

			CString strRangeLow = strLine.Mid( nRangeLowStart, nRangeLowEnd - nRangeLowStart );
			rollRecordNew.nRangeLow = _ttoi( strRangeLow );

			CString strRangeHigh = strLine.Mid( nRangeHighStart, nRangeHighEnd - nRangeHighStart );
			rollRecordNew.nRangeHigh = _ttoi( strRangeHigh );

			CString strRoll = strLine.Mid( nRollStart, nRollEnd - nRollStart );
			rollRecordNew.nRoll = _ttoi( strRoll );

			rollRecordNew.strName = strName;

			arrRolls.Add( rollRecordNew );
		}
		else if( strLine.Find( LOOT_PATTERN ) != -1 )
		{
			// Someone looted something, save it.
			//
			INT nNameStart = strLine.Find( _T(" --") ) + 3;
			INT nNameEnd = strLine.Find( _T(" has looted a ") );
			INT nLootStart = nNameEnd + 14;

			if( nNameEnd == -1 )
			{
				nNameEnd = strLine.Find( _T(" have looted a ") );
				nLootStart = nNameEnd + 15;
			}

			INT nLootEnd = strLine.Find( _T(".--") );

			CString strName = strLine.Mid( nNameStart, nNameEnd - nNameStart );
			CString strLoot = strLine.Mid( nLootStart, nLootEnd - nLootStart );

			if( !strLootedResults.IsEmpty() )
				strLootedResults += _T("\r\n");

			strLootedResults += strName + _T(" - ") + strLoot;
		}

		if( !pDlg->m_strSearch.IsEmpty() )
		{
			if( strLine.Find( pDlg->m_strSearch ) != -1 )
			{
				if( !strSearchResults.IsEmpty() )
					strSearchResults += _T("\r\n");
					
				strSearchResults += strLine;
			}
		}

		INT nCurrentPercent = ( int )( ( ( float )ulLengthRead / ( float )ulReadSize ) * 100 );
		if( nCurrentPercent > nPercentRead )
		{
			// We've read another 1%, update the progress bar.
			//
			nPercentRead = nCurrentPercent;
			pDlg->m_ctrlProgress.SetPos( nPercentRead );
		}

		if( pDlg->m_bCancel == TRUE )
		{
			// Cancel button was pressed, get out of the loop.
			//
			break;
		}
	}

	if( !pDlg->m_bExiting )
	{
		// If we are exiting, don't bother printing output or updating control states.
		//
		if( arrRolls.GetSize() > 0 )
		{
			pDlg->PrintRolls( strLootStart, arrRolls );

			arrRolls.RemoveAll();
			strLootStart.Empty();
		}

		pDlg->AddText( IDC_SEARCH, strSearchResults );
		pDlg->AddText( IDC_LOOTED, strLootedResults );
	}

	pDlg->ThreadCleanup();

	return 0;
}


