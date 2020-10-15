#include "stdafx.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define NEW  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW  new
#endif

#include <tuple>
#include "_KGeneral.h"

#include "_F_View_Heat_map.h"


////////////////////////////////////////////////////////////////////////////////////////

static constexpr DWORD se_pcs_AMP_col = 75;
static constexpr DWORD se_pcs_AMP_row = 75;
static constexpr DWORD se_pcs_col = se_pcs_AMP_col * 2 + 1;
static constexpr DWORD se_pcs_row = se_pcs_AMP_row * 2 + 1;

static int sa_DBG_vals[se_pcs_col * se_pcs_row];

// -------------------------------------------------------------------------------------

void F_View_Heat_map::DBG_Set_SRC()
{
	// -----------------------------------
	// デバッグ用
	int val_row_top = 1;
	int* pdst = sa_DBG_vals;
	for (int row = 151; row > 0; --row)
	{
		int val = val_row_top;
		for (int col = 151; col > 0; --col)
		{
			*pdst++ = val++;
		}
		val_row_top += 10;
	}

	F_View_HM_SRC l_DBG_SRC;
	l_DBG_SRC.m_pVals_src = sa_DBG_vals;

	l_DBG_SRC.m_pcs_x = 151;
	l_DBG_SRC.m_pcs_y = 151;

	l_DBG_SRC.m_x_Axis_idx = 75;
	l_DBG_SRC.m_y_Axis_idx = 75;

	l_DBG_SRC.m_x_Aux_intvl_pcs = 10;
	l_DBG_SRC.m_y_Aux_intvl_pcs = 10;

	this->Set_SRC(l_DBG_SRC);
}


////////////////////////////////////////////////////////////////////////////////////////

F_View_Heat_map::~F_View_Heat_map()
{
	if (m_pPos_y__x_Aux) { delete[] m_pPos_y__x_Aux; }
	if (m_pPos_x__y_Aux) { delete[] m_pPos_x__y_Aux; }

	if (m_pVals_textized) { delete[] m_pVals_textized; }
	if (m_pBkColor) { delete[] m_pBkColor; }
}

// -------------------------------------------------------------------------------------

void F_View_Heat_map::Set_SRC(const F_View_HM_SRC& crSRC)
{
	if (m_pVals_src)
	{ throw KEXCEPTION(L"!!! F_View_Heat_map::Set_SRC() が、２度コールされました。"); }

	m_pVals_src = crSRC.m_pVals_src;

	m_pcs_x = crSRC.m_pcs_x;
	m_pcs_y = crSRC.m_pcs_y;

	// --------------------------------------------------
	// ViewSize の設定
	m_view_size.m_Width = EN_x_width * m_pcs_x + 2;
	m_view_size.m_Height = EN_y_height * m_pcs_y + 2;

	// --------------------------------------------------
	// x 軸の情報設定
	m_x_Axis_idx = crSRC.m_x_Axis_idx;
	if (m_x_Axis_idx >= 0)
	{
		if (m_x_Axis_idx >= int(m_pcs_y))
		{ throw KEXCEPTION(L"m_x_Axis_idx >= m_pcs_y"); }

		// x 主軸 ----------
		m_rc_x_axis = {
			0, EN_y_height * m_x_Axis_idx,
			m_view_size.m_Width, EN_y_height * (m_x_Axis_idx + 1)
		};

		// x 補助線 ----------
		if (const int x_aux_intvl_pcs = crSRC.m_x_Aux_intvl_pcs; x_aux_intvl_pcs <= 0)
		{ m_pcs_x_Aux = 0;  m_pPos_y__x_Aux = NULL; }
		else
		{
			const WORD pcs_x_Aux_up = WORD(m_x_Axis_idx / x_aux_intvl_pcs);
			const WORD pcs_x_Aux_down = WORD((m_pcs_y - m_x_Axis_idx - 1) / x_aux_intvl_pcs);
			m_pcs_x_Aux = pcs_x_Aux_up + pcs_x_Aux_down + 1;  // ＋１: 主軸の下側にも補助線を引く（見た目の違和感をなくすため）

			int* pdst = NEW int[m_pcs_x_Aux];
			m_pPos_y__x_Aux = pdst;

			int pos_y = EN_y_height * (m_x_Axis_idx + 1 - x_aux_intvl_pcs * pcs_x_Aux_up);
			for (int i = m_pcs_x_Aux; i > 0; --i)
			{
				*pdst++ = pos_y;
				pos_y += EN_y_height * x_aux_intvl_pcs;
			}
		}
	}

	// --------------------------------------------------
	// y 軸の情報設定
	m_y_Axis_idx = crSRC.m_y_Axis_idx;
	if (m_y_Axis_idx >= 0)
	{
		if (m_y_Axis_idx >= int(m_pcs_x))
		{ throw KEXCEPTION(L"m_y_Axis_idx >= m_pcs_x"); }

		// y 主軸 ----------
		m_rc_y_axis = {
			EN_x_width * m_y_Axis_idx - 2, 0,
			EN_x_width * (m_y_Axis_idx + 1) - 2, m_view_size.m_Height
		};

		// y 補助線 ----------
		if (const int y_aux_intvl_pcs = crSRC.m_y_Aux_intvl_pcs; y_aux_intvl_pcs <= 0)
		{ m_pcs_y_Aux = 0;  m_pPos_x__y_Aux = NULL; }
		else
		{
			const WORD pcs_y_Aux_left = WORD(m_y_Axis_idx / y_aux_intvl_pcs);
			const WORD pcs_x_Aux_right = WORD((m_pcs_x - m_y_Axis_idx - 1) / y_aux_intvl_pcs);
			m_pcs_y_Aux = pcs_y_Aux_left + pcs_x_Aux_right + 1;  // ＋１: 主軸の右側にも補助線を引く（見た目の違和感をなくすため）

			int* pdst = NEW int[m_pcs_y_Aux];
			m_pPos_x__y_Aux = pdst;

			int pos_x = EN_x_width * (m_y_Axis_idx + 1 - y_aux_intvl_pcs * pcs_y_Aux_left) - 3;
			for (int i = m_pcs_y_Aux; i > 0; --i)
			{
				*pdst++ = pos_x;
				pos_x += EN_x_width * y_aux_intvl_pcs;
			}
		}
	}

	// --------------------------------------------------
	this->Texitize_vals_src();
	this->Crt_BkColor();

	if (m_hWnd)
	{ InvalidateRect(m_hWnd, NULL, TRUE); }
}

// -------------------------------------------------------------------------------------

void F_View_Heat_map::Texitize_vals_src()
{
	const int* pvals_src = m_pVals_src;
	WCHAR* pwchrs_dst = NEW WCHAR[m_pcs_x * m_pcs_y * 3 + 1];  //＋１：４文字ずつ書き込むため（１文字余分に必要になる）
	m_pVals_textized = pwchrs_dst;

	for (int i = m_pcs_x * m_pcs_y; i > 0; --i)
	{
		int val = *pvals_src++;

		if (val == 0)
		{
			*pwchrs_dst = 0;
			pwchrs_dst += 3;
			continue;
		}

//		if (val < 0) { val = -val; }

		if (val < 10)
		{
			*(UINT64*)pwchrs_dst = KSTR4(L' ', L' ', L'0' + val, 0);
			pwchrs_dst += 3;
			continue;
		}

		// 以下は val >= 10
		UINT64 ui64_str;
		if (val > 999)
		{
			ui64_str = L'?';
			val %= 100;
		}
		else if (val > 99)
		{
			ui64_str = val / 100 + L'0';
			val %= 100;
		}
		else
		{
			ui64_str = L' ';
		}

		ui64_str += (UINT64(val / 10 + L'0') << 16) + (UINT64(val % 10 + L'0') << 32);
		*(UINT64*)pwchrs_dst = ui64_str;
		pwchrs_dst += 3;
	}
}

// -------------------------------------------------------------------------------------

 void F_View_Heat_map::Crt_BkColor()
{
	const int* pvals_src = m_pVals_src;
	COLORREF* pbk_color = NEW COLORREF[m_pcs_x * m_pcs_y];
	m_pBkColor = pbk_color;

	for (const int* const pTmnt_src = pvals_src + m_pcs_x * m_pcs_y;; )
	{
		int val_color = *pvals_src << 1;

		if (val_color >= 255)
		{ val_color = 0; }
		else
		{ val_color = 255 - val_color; }

		*pbk_color++ = RGB(255, val_color, val_color);
//		*pbk_color++ = RGB(255, 255, 255);

		if (++pvals_src == pTmnt_src) { break; }
	}
}


////////////////////////////////////////////////////////////////////////////////////////

void F_View_Heat_map::Vntfy_Crt_Window()
{
	m_hFont = CreateFont(
		11,		// nHeight
		0,		// nWidth
		0,		// nEscapement
		0,		// nOrientation
		FW_NORMAL,	// fnWeight
		FALSE, FALSE, FALSE,	// fdwItalic, fdwUnderline, fdwStrikeOut
		SHIFTJIS_CHARSET,		// fdwCharSet
		OUT_DEFAULT_PRECIS,		// fdwOutputPrecision 
		CLIP_DEFAULT_PRECIS,	// fdwClipPrecision
		DEFAULT_QUALITY,		// fdwQuality
		FIXED_PITCH,			// fdwPitchAndFamily
		L"MS ゴシック"
	);

	m_hBrush_Axis = CreateSolidBrush(EN_COLOR_Axis);
	m_hPen_Axis_Aux = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));

	SelectObject(m_hDC, m_hFont);
}

// -------------------------------------------------------------------------------------

void F_View_Heat_map::Vntfy_Delete_GDIObj()
{
	if (m_hFont) { DeleteObject(m_hFont); }
	if (m_hBrush_Axis) { DeleteObject(m_hBrush_Axis); }
	if (m_hPen_Axis_Aux) { DeleteObject(m_hPen_Axis_Aux); }
}


// -------------------------------------------------------------------------------------

void F_View_Heat_map::Vntfy_Draw()
{
	if (m_pVals_src == NULL) { return; }

	this->Draw_Axis();

	HDC hDC = m_hDC;  // 最適化を考慮
	const WCHAR* ptxt_src = m_pVals_textized;
	const COLORREF* pbk_color = m_pBkColor;

	int pos_y = 1;
	for (int idx_y = 0; idx_y < int(m_pcs_y); ++idx_y)
	{
		if (idx_y == m_x_Axis_idx)
		{
			int pos_x = 1;
			for (int col = m_pcs_x; col > 0; --col)
			{
				if (*ptxt_src)
				{
					SetBkColor(hDC, *pbk_color);
					TextOut(hDC, pos_x, pos_y, ptxt_src, 3);
				}
					
				pos_x += EN_x_width;
				ptxt_src += 3;
				pbk_color++;
			}
			pos_y += EN_y_height;
		}
		else
		{
			SetBkColor(hDC, EN_COLOR_Normal);
			int pos_x = 1;
			for (int idx_x = 0; idx_x < int(m_pcs_x); ++idx_x)
			{
				if (*ptxt_src)
				{
					SetBkColor(hDC, *pbk_color);
					TextOut(hDC, pos_x, pos_y, ptxt_src, 3);
				}

				pos_x += EN_x_width;
				ptxt_src += 3;
				pbk_color++;
			}
			pos_y += EN_y_height;
		}
	}
}

// -------------------------------------------------------------------------------------

static HPEN s_black_pen = (HPEN)GetStockObject(BLACK_PEN);

void F_View_Heat_map::Draw_Axis()
{
	HDC hDC = m_hDC;  // 最適化を考慮

	// 外枠の描画
	SelectObject(hDC, s_black_pen);
	Rectangle(hDC, -1, -1, m_view_size.m_Width + 1, m_view_size.m_Height + 1 );

	// 主軸の描画
	if (m_x_Axis_idx >= 0) { FillRect(hDC, &m_rc_x_axis, m_hBrush_Axis); }
	if (m_y_Axis_idx >= 0) { FillRect(hDC, &m_rc_y_axis, m_hBrush_Axis); }

	// 補助線の描画
	SelectObject(hDC, m_hPen_Axis_Aux);

	// x 軸補助線
	const int* ppos_y = m_pPos_y__x_Aux;
	for (WORD i = m_pcs_x_Aux; i > 0; --i)
	{
		const int pos_y = *ppos_y++;
		MoveToEx(hDC, 0, pos_y, NULL);
		LineTo(hDC, m_view_size.m_Width, pos_y);
	}

	// y 軸補助線
	const int* ppos_x = m_pPos_x__y_Aux;
	for (WORD i = m_pcs_y_Aux; i > 0; --i)
	{
		const int pos_x = *ppos_x++;
		MoveToEx(hDC, pos_x, 0, NULL);
		LineTo(hDC, pos_x, m_view_size.m_Height);
	}
}
