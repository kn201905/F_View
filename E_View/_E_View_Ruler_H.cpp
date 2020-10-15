#include "stdafx.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define NEW  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW  new
#endif

#include "_KGeneral.h"

#include "CRetMsg.h"
#include "_E_View_Mdt.h"

#include "_E_View.h"


////////////////////////////////////////////////////////////////////////////////////////

void E_View_Ruler_H::Delete_GDIObj()
{
	if (m_hPen_main_H_ruler) { DeleteObject(m_hPen_main_H_ruler); }
	if (m_hPen_sub_H_ruler) { DeleteObject(m_hPen_sub_H_ruler); }

	if (m_hFont_rl_H) { DeleteObject(m_hFont_rl_H); }
}

// -------------------------------------------------------------------------------------

void E_View_Ruler_H::InitOnce_after_CreateDC(HDC hDC)
{
	m_hDC = hDC;

	m_hPen_main_H_ruler = CreatePen(PS_SOLID, 1, __E::EN_COLOR_main_rl_H);
	m_hPen_sub_H_ruler = CreatePen(PS_SOLID, 1, __E::EN_COLOR_sub_rl_H);

	m_hFont_rl_H = CreateFont(
		EN_LABEL_Font_Size,		// nHeight
		0,		// nWidth
		0,		// nEscapement
		0,		// nOrientation
		FW_NORMAL,	// fnWeight
		FALSE, FALSE, FALSE,	// fdwItalic, fdwUnderline, fdwStrikeOut
		SHIFTJIS_CHARSET,		// fdwCharSet
		OUT_DEFAULT_PRECIS,		// fdwOutputPrecision 
		CLIP_DEFAULT_PRECIS,	// fdwClipPrecision
		DEFAULT_QUALITY,		// fdwQuality
		0,						// fdwPitchAndFamily
		L"メイリオ"
	);
}

// -------------------------------------------------------------------------------------

void E_View_Ruler_H::Update_Setting()
{
	const DWORD main_H_ruler_price_start = (mc_cpParent->m_max_Pr_onView / m_main_H_Ruler_Units) * m_main_H_Ruler_Units;
	const DWORD min_Pr_onView = mc_cpParent->m_min_Pr_onView;
	const DWORD coeff_Pr_to_pix = mc_cpParent->m_coeff_Pr_to_pix;

	int* p_pix_y_main_ruler = ma_pix_y_main_ruler;
	E_View_Pr_CStr* p_Pr_CStr_main_ruler = ma_Pr_CStr_main_ruler;

	DWORD cnt_main_H_ruler = EN_MAX_pcs_main_H_ruler;
	for (DWORD price = main_H_ruler_price_start; price >= min_Pr_onView; price -= m_main_H_Ruler_Units)
	{
		*p_pix_y_main_ruler++ = int(((0xFFFFu - (price - mc_BasePrice)) * coeff_Pr_to_pix) >> 16);
		p_Pr_CStr_main_ruler->SetPrice(price);
		p_Pr_CStr_main_ruler++;

		if (--cnt_main_H_ruler == 0) { break; }
	}
	*p_pix_y_main_ruler = -1;

	// ---------------------------------------------------

	const DWORD sub_H_ruler_price_start = (mc_cpParent->m_max_Pr_onView / m_sub_H_Ruler_Units) * m_sub_H_Ruler_Units;

	int* p_pix_y_sub_ruler = ma_pix_y_sub_rulser;
	E_View_Pr_CStr* p_Pr_CStr_sub_ruler = ma_Pr_CStr_sub_ruler;

	DWORD cnt_sub_H_ruler = EN_MAX_pcs_sub_H_rular;
	for (DWORD price = sub_H_ruler_price_start; price >= min_Pr_onView; price -= m_sub_H_Ruler_Units)
	{
		// メインルーラーと重なる場合はスキップする
		if ((price / m_main_H_Ruler_Units) * m_main_H_Ruler_Units == price) { continue; }

		*p_pix_y_sub_ruler++ = int(((0xFFFFu - (price - mc_BasePrice)) * coeff_Pr_to_pix) >> 16);
		p_Pr_CStr_sub_ruler->SetPrice(price);
		p_Pr_CStr_sub_ruler++;

		if (--cnt_sub_H_ruler == 0) { break; }
	}
	*p_pix_y_sub_ruler = -1;
}

// -------------------------------------------------------------------------------------

void E_View_Ruler_H::Draw_Ruler_H(const int pos_x_start, const int pos_x_tmnt) noexcept
{
	const int cpix_y_top = (int)mc_cpParent->m_pix_y_View_top;

	SelectObject(m_hDC, m_hFont_rl_H);

	// ---------------------------------------------------
	// メインルーラの描画
	SelectObject(m_hDC, m_hPen_main_H_ruler);

	int* p_pix_y_main_ruler = ma_pix_y_main_ruler;
	E_View_Pr_CStr* p_main_ruler_Pr_CStr = ma_Pr_CStr_main_ruler;

	while (true)
	{
		int pix_y = *p_pix_y_main_ruler++;
		if (pix_y < 0) { break; }
		pix_y -= cpix_y_top;

		TextOut(m_hDC, -se_View_mgn.m_Left + 3, pix_y - EN_LABEL_V_Offset, p_main_ruler_Pr_CStr->ma_pCStr, 5);
		p_main_ruler_Pr_CStr++;

		if (pix_y < se_View_size.m_Height)
		{
			MoveToEx(m_hDC, pos_x_start, pix_y, NULL);
			LineTo(m_hDC, pos_x_tmnt, pix_y);
		}
	}

	// ---------------------------------------------------
	// サブルーラの描画
	SelectObject(m_hDC, m_hPen_sub_H_ruler);

	int* p_pix_y_sub_ruler = ma_pix_y_sub_rulser;
	E_View_Pr_CStr* p_sub_ruler_Pr_CStr = ma_Pr_CStr_sub_ruler;

	while (true)
	{
		int pix_y = *p_pix_y_sub_ruler++;
		if (pix_y < 0) { break; }
		pix_y -= cpix_y_top;

		TextOut(m_hDC, -se_View_mgn.m_Left + 3, pix_y - EN_LABEL_V_Offset, p_sub_ruler_Pr_CStr->ma_pCStr, 5);
		p_sub_ruler_Pr_CStr++;

		if (pix_y < se_View_size.m_Height)
		{
			MoveToEx(m_hDC, pos_x_start, pix_y, NULL);
			LineTo(m_hDC, pos_x_tmnt, pix_y);
		}
	}
}


// -------------------------------------------------------------------------------------

void E_View_Ruler_H::Draw_Label_only() noexcept
{
	const int cpix_y_top = (int)mc_cpParent->m_pix_y_View_top;

	SelectObject(m_hDC, m_hFont_rl_H);

	// ---------------------------------------------------
	// メインルーラの描画

	int* p_pix_y_main_ruler = ma_pix_y_main_ruler;
	E_View_Pr_CStr* p_main_ruler_Pr_CStr = ma_Pr_CStr_main_ruler;

	while (true)
	{
		int pix_y = *p_pix_y_main_ruler++;
		if (pix_y < 0) { break; }
		pix_y -= cpix_y_top;

		TextOut(m_hDC, -se_View_mgn.m_Left + 3, pix_y - 7, p_main_ruler_Pr_CStr->ma_pCStr, 5);
		p_main_ruler_Pr_CStr++;
	}

	// ---------------------------------------------------
	// サブルーラの描画

	int* p_pix_y_sub_ruler = ma_pix_y_sub_rulser;
	E_View_Pr_CStr* p_sub_ruler_Pr_CStr = ma_Pr_CStr_sub_ruler;

	while (true)
	{
		int pix_y = *p_pix_y_sub_ruler++;
		if (pix_y < 0) { break; }
		pix_y -= cpix_y_top;

		TextOut(m_hDC, -se_View_mgn.m_Left + 3, pix_y - 7, p_sub_ruler_Pr_CStr->ma_pCStr, 5);
		p_sub_ruler_Pr_CStr++;
	}
}

