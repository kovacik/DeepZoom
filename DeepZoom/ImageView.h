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


#pragma once

// CImageView window

class CImageView : public CWnd
{
    DECLARE_DYNCREATE(CImageView)

    SmartPtr<IRenderer> m_Renderer;
    SmartPtr<IViewControllerScene> m_ViewController;
    SmartPtr<IToolManager> m_ToolManager;
    
public:
	CImageView();
	~CImageView();

    bool OnIdle();
    void Update();

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);

    afx_msg void OnMouseMove(UINT flags, CPoint pt);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnMouseLeave();
    afx_msg void OnLButtonDown(UINT flags, CPoint pt);
    afx_msg void OnLButtonDblClk(UINT flags, CPoint pt);
    afx_msg void OnLButtonUp(UINT flags, CPoint pt);
    afx_msg void OnMButtonDown(UINT flags, CPoint pt);
    afx_msg void OnMButtonDblClk(UINT flags, CPoint pt);
    afx_msg void OnMButtonUp(UINT flags, CPoint pt);
    afx_msg void OnRButtonDown(UINT flags, CPoint pt);
    afx_msg void OnRButtonDblClk(UINT flags, CPoint pt);
    afx_msg void OnRButtonUp(UINT flags, CPoint pt);
        
    HRESULT InitializeToolManager();
    HRESULT InitializeRenderer();

	DECLARE_MESSAGE_MAP()
};

