//
// Copyright (C) 2013, Alojz Kovacik, http://kovacik.github.com
//
// This file is part of Deep Zoom.
//
// Deep Zoom is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Deep Zoom is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Deep Zoom. If not, see <http://www.gnu.org/licenses/>.
//



// DeepZoom.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "DeepZoom.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDeepZoomApp

BEGIN_MESSAGE_MAP(CDeepZoomApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CDeepZoomApp::OnAppAbout)
    ON_COMMAND(ID_FILE_OPENIMAGE, &CDeepZoomApp::OnImageOpen)
END_MESSAGE_MAP()


// CDeepZoomApp construction

CDeepZoomApp::CDeepZoomApp()
{
	m_bHiColorIcons = TRUE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("DeepZoom.AppID.1_0"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CDeepZoomApp object

CDeepZoomApp theApp;



int AllocHook( int allocType, void *userData, size_t size, int blockType, long requestNumber, const unsigned char *filename, int lineNumber)
{
    UNREFERENCED_PARAMETER(allocType);
    UNREFERENCED_PARAMETER(userData);
    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(blockType);
    UNREFERENCED_PARAMETER(requestNumber);
    UNREFERENCED_PARAMETER(filename);
    UNREFERENCED_PARAMETER(lineNumber);

    int b;
    if (allocType == _HOOK_ALLOC)
    {
        if (requestNumber == 332)
        {
            b = 0;
        }
    }


    return TRUE;

}


// CDeepZoomApp initialization

BOOL CDeepZoomApp::InitInstance()
{
    _CrtSetAllocHook(AllocHook);

    // Initialize COM (needed for WIC)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        wprintf( L"Failed to initialize COM (%08X)\n", hr);
        return 1;
    }

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    if (!InitContextMenuManager())
        return false;

	if (!InitKeyboardManager())
        return false;

	if (!InitTooltipManager())
        return false;

    if (!InitCamera2D())
		return false;

    if (!InitSceneGraph2D())
        return false;

    CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

    
	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}

int CDeepZoomApp::ExitInstance()
{
    m_SceneGraph2D->Destroy();
    m_SceneGraph2D = NULL;

    CoUninitialize();

	//TODO: handle additional resources you may have added
	return CWinAppEx::ExitInstance();
}



// main running routine until thread exits
int CDeepZoomApp::Run()
{
	_AFX_THREAD_STATE* pState = AfxGetThreadState();

	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;

	// acquire and dispatch messages until a WM_QUIT message is received.
	for (;;)
	{
		// phase1: check to see if we can do idle work
		while (bIdle &&
			!::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE))
		{
			// call OnIdle while in bIdle state
			if (!OnIdle(lIdleCount++))
				bIdle = FALSE; // assume "no idle" state
		}

		// phase2: wait for messages while sleeping, update internal idle handlers
        while (!::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE))
		{    

            CMainFrame *pFrame = (CMainFrame*)m_pMainWnd;
            if (pFrame)
            {
                if (!pFrame->OnIdle())
                    break;
            }

            Sleep(10);
        }

		// phase3: pump messages while available
		do
		{
           
            // pump message, but quit on WM_QUIT
			if (!PumpMessage())
				return ExitInstance();

            CMainFrame *pFrame = (CMainFrame*)m_pMainWnd;
            if (pFrame)
            {
                m_SceneGraph2D->Validate();
                pFrame->Update();
            }

            // reset "no idle" state after pumping "normal" message
			//if (IsIdleMessage(&m_msgCur))
			if (IsIdleMessage(&(pState->m_msgCur)))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}

        } while (::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE));
	}
}


// CDeepZoomApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
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

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CDeepZoomApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CDeepZoomApp customization load/save methods

void CDeepZoomApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CDeepZoomApp::LoadCustomState()
{
}

void CDeepZoomApp::SaveCustomState()
{
}

// CDeepZoomApp message handlers

BOOL CDeepZoomApp::InitSceneGraph2D()
{
    HRESULT hr = CreateSceneGraph(&m_SceneGraph2D);

    if (SUCCEEDED(hr))
    {
        hr = CreateSceneGraphEditor(m_SceneGraph2D, m_spCamera2D, false, &m_spSceneGraph2DEditor);
    }

    return hr == S_OK;
}

BOOL CDeepZoomApp::InitCamera2D()
{
    HRESULT hr = CreateCamera(L"camera2d", ProjectionType::Orthographic, &m_spCamera2D);
    return hr == S_OK;
}

ISceneGraph* CDeepZoomApp::GetSceneGraph2D()
{
    return m_SceneGraph2D;
}

ISceneObjectCamera* CDeepZoomApp::GetCamera2D()
{
    return m_spCamera2D;
}


void CDeepZoomApp::OnImageOpen()
{
    LPCTSTR filter = L"JPEG (*.jpg)|*.jpg|Bitmap (*.bmp)|*.bmp|PNG (*.png)|*.png||";

    CView* pActiveView = ((CFrameWnd*) AfxGetMainWnd())->GetActiveView();
    CFileDialog openImageDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter, pActiveView);

    if (openImageDlg.DoModal() == IDOK)
    {
        CString path = openImageDlg.GetPathName();
        
        HRESULT hr = m_spSceneGraph2DEditor->ClearSceneGraph();
        if (SUCCEEDED(hr))
        {
            hr = m_spSceneGraph2DEditor->AddImage(path);
        }
    }
}