#include "stdafx.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define NEW  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW  new
#endif

#include "_KGeneral.h"
#include "__KAppGlobal.h"

#include "CRetMsg.h"
#include "_E_View_Mdt.h"

#include "_E_View.h"


////////////////////////////////////////////////////////////////////////////////////////
// コンストラクタ

E_View_Scrl_H::E_View_Scrl_H(E_View* pParent, int* pParent_col_view_left, E_View_Ruler_H* pRuler_H, E_View_Ruler_V* pRuler_V)
	: mc_cpParet{ pParent }
	, mc_pRuler_H{ pRuler_H }
	, mc_pRuler_V{ pRuler_V }
	, mc_pParent_col_view_left{ pParent_col_view_left }
{
	m_sci_H.cbSize = sizeof(SCROLLINFO);
}


////////////////////////////////////////////////////////////////////////////////////////

void E_View_Scrl_H::On_InitCol(int pcs_col)
{
	m_sci_H.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
	m_sci_H.nMin = 0;			// nPos == m_col_view_left としたいため nMin = 0, nMax = pcs_col - 1 とする
	m_sci_H.nMax = pcs_col - 1;
	m_sci_H.nPage = UINT(se_View_size.m_Width);
	m_sci_H.nPos = 0;

	SetScrollInfo(m_hWnd, SB_HORZ, &m_sci_H, TRUE);

	m_sci_H.fMask = SIF_DISABLENOSCROLL | SIF_POS;

	if (pcs_col <= se_View_size.m_Width)
	{
		m_col_view_left_MAX = 0;
	}
	else
	{
		m_col_view_left_MAX = pcs_col - se_View_size.m_Width;
	}
}

// -------------------------------------------------------------------------------------

void E_View_Scrl_H::Hndlr_WM_HSCROLL(WPARAM wp)
{
	switch(LOWORD(wp))
	{
	case SB_LINELEFT:
		this->Move_Right(PIXS_MOVE_LINE_H);
		break;

	case SB_LINERIGHT:
		this->Move_Left(PIXS_MOVE_LINE_H);
		break;

	case SB_PAGELEFT:
		this->Move_Right(PIXS_MOVE_PAGE_H);
		break;

	case SB_PAGERIGHT:
		this->Move_Left(PIXS_MOVE_PAGE_H);
		break;
	}
}


// -------------------------------------------------------------------------------------
// dx は、dx > 0 で指定

void E_View_Scrl_H::Move_Left(int dx)
{
	int col_view_left = *mc_pParent_col_view_left;
	if (col_view_left == m_col_view_left_MAX) { return; }

	const int new_col_view_left = [&]() -> int {
		if (col_view_left + dx < m_col_view_left_MAX) { return col_view_left + dx; }

		dx = m_col_view_left_MAX - col_view_left;
		return m_col_view_left_MAX;
	}();
	*mc_pParent_col_view_left = new_col_view_left;  // ここで、m_pParent_col_view_left を更新しておく

	// スクロールバーの更新
	m_sci_H.nPos = new_col_view_left;
	SetScrollInfo(m_hWnd, SB_HORZ, &m_sci_H, TRUE);

	// View のスクロール
	RECT rcScroll = {se_View_pos.m_Left + dx, se_View_pos.m_Top, se_View_pos.m_Right, se_View_pos.m_Btm};
	ScrollWindowEx(m_hWnd, -dx, 0, &rcScroll, NULL, NULL, NULL, 0);

	const int pos_x_start_update = se_View_size.m_Width - dx;
	RECT rcUpdate = {pos_x_start_update, 0, se_View_size.m_Width, se_View_size.m_Height};
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// ruler V のスクロール
	rcScroll = {se_View_pos.m_Left + dx - E_View_Ruler_V::EN_LABEL_H_Offset,
				se_View_pos.m_Btm + 1, se_View_pos.m_Right + se_View_mgn.m_Right, se_View_pos.m_Btm + se_View_mgn.m_Btm};
	ScrollWindowEx(m_hWnd, -dx, 0, &rcScroll, NULL, NULL, NULL, 0);

	rcUpdate = {se_View_size.m_Width - dx + E_View_Ruler_V::EN_LABEL_H_Offset,
				se_View_size.m_Height + 1, se_View_size.m_Width + se_View_mgn.m_Right, se_View_size.m_Height + se_View_mgn.m_Btm};
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// 罫線を引いた後に、グラフを描画する
	mc_pRuler_V->Draw_Ruler_V(pos_x_start_update, se_View_size.m_Width);
	mc_pRuler_H->Draw_Ruler_H(pos_x_start_update, se_View_size.m_Width);

	mc_cpParet->DrawView(pos_x_start_update, se_View_size.m_Width);

	// XIdx_pos の表示を更新するために、E_View_Ind_Wnd にも通知する
	E_View_Ind_Wnd::On_Chg_col_view_left(new_col_view_left);
}


// -------------------------------------------------------------------------------------

void E_View_Scrl_H::Move_Right(int dx)
{
	int col_view_left = *mc_pParent_col_view_left;
	if (col_view_left == 0) { return; }

	const int new_col_view_left = [&]() -> int {
		if (col_view_left > dx) { return col_view_left - dx; }

		dx = col_view_left;
		return 0;
	}();
	*mc_pParent_col_view_left = new_col_view_left;  // ここで、m_pParent_col_view_left を更新しておく

	// スクロールバーの更新
	m_sci_H.nPos = new_col_view_left;
	SetScrollInfo(m_hWnd, SB_HORZ, &m_sci_H, TRUE);

	// dx の修正が必要な場合、ここで行う
	dx = col_view_left - new_col_view_left;
	
	// --------------------------------------------
	// View のスクロール
	// m_hWnd に対する座標系
	RECT rcScroll = {se_View_pos.m_Left, se_View_pos.m_Top, se_View_pos.m_Right - dx, se_View_pos.m_Btm};
	ScrollWindowEx(m_hWnd, dx, 0, &rcScroll, NULL, NULL, NULL, 0);

	// hDC に対する座標系
	RECT rcUpdate = {0, 0, dx, se_View_size.m_Height};
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// --------------------------------------------
	// ruler V ラベルのスクロール
	// m_hWnd に対する座標系
//	rcScroll = { se_View_pos.m_Left - E_View_Ruler_V::EN_LABEL_H_Offset, se_View_pos.m_Btm + 1,
//				se_View_pos.m_Right - dx + E_View_Ruler_V::EN_LABEL_H_Offset, se_View_pos.m_Btm + se_View_mgn.m_Btm };
	rcScroll = { se_View_pos.m_Left, se_View_pos.m_Btm + 1,
				se_View_pos.m_Right - dx + E_View_Ruler_V::EN_LABEL_H_Offset, se_View_pos.m_Btm + se_View_mgn.m_Btm };
	ScrollWindowEx(m_hWnd, dx, 0, &rcScroll, NULL, NULL, NULL, 0);

	// hDC に対する座標系
//	rcUpdate = { -E_View_Ruler_V::EN_LABEL_H_Offset, se_View_size.m_Height + 1,
//				dx - E_View_Ruler_V::EN_LABEL_H_Offset, se_View_size.m_Height + se_View_mgn.m_Btm };
	rcUpdate = { -E_View_Ruler_V::EN_LABEL_H_Offset, se_View_size.m_Height + 1,
				dx, se_View_size.m_Height + se_View_mgn.m_Btm };
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// 罫線を引いた後に、グラフを描画する
	mc_pRuler_V->Draw_Ruler_V(0, dx);
	mc_pRuler_H->Draw_Ruler_H(0, dx);

	mc_cpParet->DrawView(0, dx);

	// XIdx_pos の表示を更新するために、E_View_Ind_Wnd にも通知する
	E_View_Ind_Wnd::On_Chg_col_view_left(new_col_view_left);
}
