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

E_View_Scrl_V::E_View_Scrl_V(E_View* pParent, E_View_Ruler_H* pRuler_H, E_View_Ruler_V* pRuler_V,
											const DWORD* pPr_max_onView, const DWORD* pPr_min_onView)
	: mc_pParet{ pParent }
	, mc_pRuler_H{ pRuler_H }
	, mc_pRuler_V{ pRuler_V }
	, mc_cpPr_max_on_view{ pPr_max_onView }
	, mc_cpPr_min_on_view{ pPr_min_onView }
{
	m_sci_V.cbSize = sizeof(SCROLLINFO);
}


////////////////////////////////////////////////////////////////////////////////////////

// m_hWnd が設定された後で Init される（親の OpenWindow() からコールされる）
void E_View_Scrl_V::InitOnce_after_CreateDC(HWND hWnd, HDC hDC)
{
	m_hWnd = hWnd;
	m_hDC = hDC;
	m_hBrush_bkgd = mc_pParet->Get_hBrush_bkgd();

	const DWORD max_Pr_onView = *mc_cpPr_max_on_view;
	const DWORD min_Pr_onView = *mc_cpPr_min_on_view;

	m_sci_V.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
	m_sci_V.nMin = 0;
	m_sci_V.nMax = 0xFFFFu - 0x100u;
	m_sci_V.nPage = max_Pr_onView - min_Pr_onView + 1;
	m_sci_V.nPos = int(0xFFFFu - (max_Pr_onView - mc_pParet->mc_BasePrice));

	SetScrollInfo(m_hWnd, SB_VERT, &m_sci_V, TRUE);

	m_sci_V.fMask = SIF_DISABLENOSCROLL | SIF_POS;

	m_max_Pr_onView_TOP = mc_pParet->mc_BasePrice + 0xFFFFu;
	m_min_Pr_onView_TOP = mc_pParet->mc_BasePrice + 0x100u + (max_Pr_onView - min_Pr_onView);
}

// -------------------------------------------------------------------------------------

void E_View_Scrl_V::Move_Up(int dy)  // dy > 0
{
	const DWORD cur_max_Pr_onView = *mc_cpPr_max_on_view;
	if (cur_max_Pr_onView == m_min_Pr_onView_TOP) { return; }

	const DWORD new_max_Pr_onView = [&]() -> DWORD {
		if (cur_max_Pr_onView - dy >= m_min_Pr_onView_TOP) { return cur_max_Pr_onView - dy; }

		dy = int(cur_max_Pr_onView - m_min_Pr_onView_TOP);
		return m_min_Pr_onView_TOP;
	}();
	const int dy_pix = mc_pParet->Ntfy_MoveUp(UINT(dy));

	// スクロールバーの更新
	m_sci_V.nPos = int(0xFFFFu - (new_max_Pr_onView - mc_pParet->mc_BasePrice));
	SetScrollInfo(m_hWnd, SB_VERT, &m_sci_V, TRUE);

	// View のスクロール
	// hWnd 座標系（ラベルもまとめてスクロールさせてしまう）
	RECT rcScroll = { 0, se_View_pos.m_Top + dy_pix, se_View_pos.m_Right, se_View_pos.m_Btm };
	ScrollWindowEx(m_hWnd, 0, -dy_pix, &rcScroll, NULL, NULL, NULL, 0);

	// 新規表示部分の View エリアのクリア
	RECT rcUpdate = { 0, se_View_size.m_Height - dy_pix, se_View_size.m_Width, se_View_size.m_Height };
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// 新規表示部分の LABEL エリアのクリア
	rcUpdate = { -se_View_mgn.m_Left, se_View_size.m_Height - dy_pix, -1, se_View_size.m_Height + E_View_Ruler_H::EN_LABEL_V_Offset };
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// LABEL 表示のゴミが残るため、それを消去
	rcUpdate = { -se_View_mgn.m_Left, -E_View_Ruler_H::EN_LABEL_V_Offset, -1, 0 };
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// 罫線を引いた後に、グラフを描画する
	mc_pRuler_V->Draw_Ruler_V(0, se_View_size.m_Width);
	mc_pRuler_H->Draw_Ruler_H(0, se_View_size.m_Width);

	mc_pParet->DrawView(0, se_View_size.m_Width);
}

// -------------------------------------------------------------------------------------

void E_View_Scrl_V::Move_Down(int dy)  // dy > 0
{
	const DWORD cur_max_Pr_onView = *mc_cpPr_max_on_view;
	if (cur_max_Pr_onView == m_max_Pr_onView_TOP) { return; }

	const DWORD new_max_Pr_onView = [&]() -> DWORD {
		if (cur_max_Pr_onView + dy <= m_max_Pr_onView_TOP) { return cur_max_Pr_onView + dy; }

		dy = int(m_max_Pr_onView_TOP - cur_max_Pr_onView);
		return m_max_Pr_onView_TOP;
	}();
	const int dy_pix = mc_pParet->Ntfy_MoveDown(UINT(dy));

	// スクロールバーの更新
	m_sci_V.nPos = int(0xFFFFu - (new_max_Pr_onView - mc_pParet->mc_BasePrice));
	SetScrollInfo(m_hWnd, SB_VERT, &m_sci_V, TRUE);

	// View のスクロール
	// hWnd 座標系（ラベルもまとめてスクロールさせてしまう）
	RECT rcScroll = { 0, se_View_pos.m_Top, se_View_pos.m_Right, se_View_pos.m_Btm - dy_pix };
	ScrollWindowEx(m_hWnd, 0, dy_pix, &rcScroll, NULL, NULL, NULL, 0);

	// 新規表示部分の View エリアのクリア
	// hDC 座標系
	RECT rcUpdate = { 0, 0, se_View_size.m_Width, dy_pix };
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// 新規表示部分の LABEL エリアのクリア
	// hDC 座標系
	rcUpdate = { -se_View_mgn.m_Left, -E_View_Ruler_H::EN_LABEL_V_Offset, -1, dy_pix };
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// LABEL 表示のゴミが残るため、それを消去
	rcUpdate = { -se_View_mgn.m_Left, se_View_size.m_Height, -1, se_View_size.m_Height + E_View_Ruler_H::EN_LABEL_V_Offset };
	FillRect(m_hDC, &rcUpdate, m_hBrush_bkgd);

	// 罫線を引いた後に、グラフを描画する
	mc_pRuler_V->Draw_Ruler_V(0, se_View_size.m_Width);
	mc_pRuler_H->Draw_Ruler_H(0, se_View_size.m_Width);

	mc_pParet->DrawView(0, se_View_size.m_Width);
}
