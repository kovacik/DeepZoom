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



// ChildView.cpp : implementation of the CImageView class
//

#include "stdafx.h"
#include "DeepZoom.h"
#include "ImageView.h"
#include "..\SceneGraph\SceneGraphLib.h"
#include "..\RendererDirectX11\RendererDirectX11Lib.h"
#include "..\ToolManager\ToolManagerLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CImageView, CWnd)

// CImageView

CImageView::CImageView()
{
    InitializeToolManager();
}

CImageView::~CImageView()
{
    m_ToolManager->Destroy();
}


BEGIN_MESSAGE_MAP(CImageView, CWnd)
	ON_WM_PAINT()
    ON_WM_SIZE()

    ON_WM_MOUSEMOVE()
    ON_WM_MOUSEWHEEL()
    ON_WM_MOUSELEAVE()

    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_MBUTTONDOWN()
    ON_WM_MBUTTONDBLCLK()
    ON_WM_MBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONDBLCLK()
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CImageView message handlers

BOOL CImageView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

HRESULT CImageView::InitializeRenderer()
{
    HRESULT hr = CreateRendererDirectX11(this->m_hWnd, theApp.GetSceneGraph2D(), &m_Renderer );
    if (SUCCEEDED(hr))
    {
        hr = m_ViewController->SetRenderer(m_Renderer);

        if (SUCCEEDED(hr))
        {
			hr = m_Renderer->SetCamera(theApp.GetCamera2D());
        }
    }
    
    return hr;
}

void CImageView::Update()
{
    if (m_Renderer)
    {
        m_Renderer->Draw(true);
    }
}

void CImageView::OnPaint() 
{
    if (m_Renderer == NULL)
    {
        InitializeRenderer();
    }
    else
    {
        RECT rect;
        GetClientRect(&rect);
        m_Renderer->Resize( rect );
    }
	
    CPaintDC dc(this); 

    if (m_Renderer)
    {
        m_Renderer->Draw(true);
    }
}

BOOL CImageView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    m_ToolManager->ProcessMouseEvent(nFlags, pt, zDelta);
    return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CImageView::OnMouseLeave()
{
    CWnd::OnMouseLeave();
}

void CImageView::OnMouseMove(UINT flags, CPoint pt)
{
    CWnd::OnMouseMove(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

void CImageView::OnLButtonDown(UINT flags, CPoint pt)
{
    CWnd::OnLButtonDown(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

void CImageView::OnLButtonDblClk(UINT flags, CPoint pt)
{
    CWnd::OnRButtonDblClk(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

void CImageView::OnLButtonUp(UINT flags, CPoint pt)
{
    CWnd::OnLButtonUp(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

void CImageView::OnMButtonDown(UINT flags, CPoint pt)
{
    CWnd::OnMButtonDown(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

void CImageView::OnMButtonDblClk(UINT flags, CPoint pt)
{
    CWnd::OnRButtonDblClk(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

void CImageView::OnMButtonUp(UINT flags, CPoint pt)
{
    CWnd::OnMButtonUp(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

void CImageView::OnRButtonDown(UINT flags, CPoint pt)
{
    CWnd::OnRButtonDown(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

void CImageView::OnRButtonDblClk(UINT flags, CPoint pt)
{
    CWnd::OnRButtonDblClk(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

void CImageView::OnRButtonUp(UINT flags, CPoint pt)
{
    CWnd::OnRButtonUp(flags, pt);
    m_ToolManager->ProcessMouseEvent(flags, pt);
}

HRESULT CImageView::InitializeToolManager()
{ 
    HRESULT hr = CreateSceneViewController(&m_ViewController);
    if (SUCCEEDED(hr))
    {
        hr = CreateToolManager(m_ViewController, &m_ToolManager);
        if (SUCCEEDED(hr))
        {
			SmartPtr<IToolSource> spToolSource;
            hr = CreateBackgroundToolSource(m_hWnd, &spToolSource);

            if (SUCCEEDED(hr))
            {
				// Add tool for camera movement and rotation
				SmartPtr<ITool> spCameraTool;
				hr = CreateCamera2DMoveTool(&spCameraTool);

				if (SUCCEEDED(hr))
				{
					hr = spToolSource->AddTool(spCameraTool);

					if (SUCCEEDED(hr))
					{
						hr = m_ToolManager->AddToolSource(spToolSource);
					}
				}
            }
        }
    }
    return hr;
}

bool CImageView::OnIdle()
{
    return m_ToolManager->OnIdle() != S_OK;
}

void CImageView::OnSize(UINT nType, int cx, int cy) 
{
    UNREFERENCED_PARAMETER(cx);
    UNREFERENCED_PARAMETER(cy);
    UNREFERENCED_PARAMETER(nType);
}
