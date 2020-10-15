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
#include "_KGeneral_num.h"

#include "CRetMsg.h"
//#include "__Facade.h"
#include "_E_View_Mdt.h"

#include "_E_View.h"


extern bool g_DBG_Push_Close_Btn;

////////////////////////////////////////////////////////////////////////////////////////

E_View_Ind_Wnd s_E_View_Ind_Wnd;  // 唯一の実体

namespace __E_View
{
	// 公開メソッド
	E_View_Ind_Wnd* Get_E_View_Ind_Wnd() { return &s_E_View_Ind_Wnd; }
}

////////////////////////////////////////////////////////////////////////////////////////

CRetMsg E_View_Ind_Wnd::ms_RetMsg;

// このブラシは Window Class に登録するため、静的にするようにこのように実装した
struct Ind_Wnd_Bkgd_Brush
{
	~Ind_Wnd_Bkgd_Brush() { DeleteObject(mc_hBrush); }
	const HBRUSH mc_hBrush = CreateSolidBrush(E_View_Ind_Wnd::EN_color_bkgd);
}
static s_Ind_Wnd_Bkgd_Brush;  // 唯一の実体

////////////////////////////////////////////////////////////////////////////////////////

const CRetMsg* E_View_Ind_Wnd::Open_Ind_Wnd(const E_View_pos& crView_pos, const HWND hWnd_Prnt)
{
	{
		const CRetMsg* pret_msg = E_View_Ind_Wnd::S_Rgst_Ind_WndClass();
		if (pret_msg) { return pret_msg; }
	}

	m_hWnd_Prnt = hWnd_Prnt;

	m_LeftTop_onEView = { crView_pos.m_Left, crView_pos.m_Top };
	m_Width = crView_pos.m_Right - crView_pos.m_Left;
	m_Height = crView_pos.m_Btm - crView_pos.m_Top;
	
	POINT pt_on_scrn = m_LeftTop_onEView;
	ClientToScreen(hWnd_Prnt, &pt_on_scrn);

	m_hWnd = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TRANSPARENT,	// dwExStyle
		L"E_View_Ind_Wnd",  // class name
		L"E_View_Ind_Wnd",  // window name
		WS_POPUP | WS_VISIBLE,	// style
		pt_on_scrn.x, pt_on_scrn.y,
		m_Width, m_Height,
		hWnd_Prnt,  // hWndParent
		NULL,	// hMenu
		KAppGlobal::HInstance(),
		NULL	// WM_CREATE メッセージの lParam パラメータとして渡される構造体ポインタ
	);

	if (m_hWnd == NULL)
	{ ms_RetMsg.CSET_MSG(L"!!! Ind_Wnd の CreateWindow() に失敗しました。"); return &ms_RetMsg; }

	BOOL bret = SetLayeredWindowAttributes(m_hWnd, EN_color_bkgd, 100, LWA_COLORKEY | LWA_ALPHA );
	if (bret == FALSE)
	{ ms_RetMsg.CSET_MSG(L"!!! SetLayeredWindowAttributes() に失敗しました。"); return &ms_RetMsg; }

	m_hDC = GetDC(m_hWnd);
	if (m_hDC == NULL)
	{ ms_RetMsg.CSET_MSG(L"!!! GetDC() に失敗しました。"); return &ms_RetMsg; }

	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	SelectObject(m_hDC, GetStockObject(BLACK_PEN));

	m_hPen_Ind = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	m_hPen_Erase = CreatePen(PS_SOLID, 1, EN_color_bkgd);

	SelectObject(m_hDC, m_hPen_Ind);

	// --------------------------------------------
	{
		const CRetMsg* pret_msg = E_View_Ind_Wnd::S_Rgst_XIdx_WndClass();
		if (pret_msg) { return pret_msg; }

		POINT pt_xIdx_on_scrn = { crView_pos.m_Left, crView_pos.m_Btm };
		ClientToScreen(hWnd_Prnt, &pt_xIdx_on_scrn);
		m_Left_XIdx_byScrn = pt_xIdx_on_scrn.x - EN_offset_XIdx;
		m_Top_XIdx_byScrn = pt_xIdx_on_scrn.y;

		m_hWnd_XIdx = CreateWindowEx(
			0,	// dwExStyle
			L"E_View_XIdx_Wnd",  // class name
			L"E_View_XIdx_Wnd",  // window name
			WS_POPUP | WS_VISIBLE,	// style
			m_Left_XIdx_byScrn, m_Top_XIdx_byScrn,		// x, y
			EN_width_XIdx, EN_height_XIdx,	// width, height
			hWnd_Prnt,  // hWndParent
			NULL,	// hMenu
			KAppGlobal::HInstance(),
			NULL	// WM_CREATE メッセージの lParam パラメータとして渡される構造体ポインタ
		);

		if (m_hWnd_XIdx == NULL)
		{ ms_RetMsg.CSET_MSG(L"!!! XIdx_Wnd の CreateWindow() に失敗しました。"); return &ms_RetMsg; }

		SetWindowLongPtr(m_hWnd_XIdx, GWLP_USERDATA, (LONG_PTR)this);

		ms_hDC_XIdx = GetDC(m_hWnd_XIdx);
		if (ms_hDC_XIdx == NULL)
		{ ms_RetMsg.CSET_MSG(L"!!! m_hDC_XIdx の GetDC() に失敗しました。"); return &ms_RetMsg; }

		SelectObject(ms_hDC_XIdx, GetStockObject(WHITE_BRUSH));
		SelectObject(ms_hDC_XIdx, GetStockObject(BLACK_PEN));

		SelectObject(ms_hDC_XIdx, g_GDI_Cmn_Obj.m_hFont_Meiryo_15pt);
	}

	return NULL;
}

// -------------------------------------------------------------------------------------

const CRetMsg* E_View_Ind_Wnd::S_Rgst_Ind_WndClass()
{
	static bool ss_Rgst_Child_WndClass = false;
	if (ss_Rgst_Child_WndClass) { return NULL; }
	ss_Rgst_Child_WndClass = true;

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = E_View_Ind_Wnd::S_Ind_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = KAppGlobal::HInstance();
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = s_Ind_Wnd_Bkgd_Brush.mc_hBrush;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"E_View_Ind_Wnd";
	wc.hIconSm = NULL;

	if (RegisterClassEx(&wc) == 0)
	{ ms_RetMsg.CSET_MSG(L"!!! RegisterClass() に失敗しました。");  return &ms_RetMsg; }

	return NULL;
}

// -------------------------------------------------------------------------------------

static bool sb_ReleaseRsc = false;

LRESULT CALLBACK E_View_Ind_Wnd::S_Ind_WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PAINT)
	{
		ValidateRect(hWnd, NULL);

		E_View_Ind_Wnd* pInd_Wnd = (E_View_Ind_Wnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pInd_Wnd->On_WM_PAINT();

		return 0;
	}

	// WM_CLOSE、WM_DESTROY メッセージを捕捉できないため、以下の手段を用いている
	if (g_DBG_Push_Close_Btn == true)
	{
		if (sb_ReleaseRsc == false)
		{
			sb_ReleaseRsc = true;
			E_View_Ind_Wnd* const pInd_Wnd = (E_View_Ind_Wnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			pInd_Wnd->Release_Rsc();
		}
	}

	return DefWindowProc(hWnd , msg , wp , lp);
}

// -------------------------------------------------------------------------------------

void E_View_Ind_Wnd::Release_Rsc()
{
	if (m_hDC)
	{
		SelectObject(m_hDC, GetStockObject(WHITE_BRUSH));
		SelectObject(m_hDC, GetStockObject(BLACK_PEN));

		ReleaseDC(m_hWnd, m_hDC);
		m_hDC = NULL;
	}

	if (m_hPen_Ind) { DeleteObject(m_hPen_Ind);  m_hPen_Ind = NULL; }
	if (m_hPen_Erase) { DeleteObject(m_hPen_Erase);  m_hPen_Erase = NULL; }

	// -----------------------------------------------------
	if (ms_hDC_XIdx)
	{
		ReleaseDC(m_hWnd_XIdx, ms_hDC_XIdx);
		ms_hDC_XIdx = NULL;
	}

//	OutputDebugString(L"--- E_View_Ind_Wnd のシステムリソースを解放しました\r\n");
}

// -------------------------------------------------------------------------------------

void E_View_Ind_Wnd::On_WM_PAINT()
{
	MoveToEx(m_hDC, m_X_cur_Ind, 0, NULL);
	LineTo(m_hDC, m_X_cur_Ind, m_Height);
}

// -------------------------------------------------------------------------------------

void E_View_Ind_Wnd::On_WM_MOVE___E_View()
{
//	if (m_hWnd == NULL) { return; }

	POINT pt_on_scrn = m_LeftTop_onEView;
	ClientToScreen(m_hWnd_Prnt, &pt_on_scrn);

	MoveWindow(m_hWnd, pt_on_scrn.x, pt_on_scrn.y, m_Width, m_Height, FALSE);

	// ------------------------------------------------
	m_Left_XIdx_byScrn = pt_on_scrn.x + m_X_cur_Ind - EN_offset_XIdx;
	m_Top_XIdx_byScrn = pt_on_scrn.y + m_Height;
	MoveWindow(m_hWnd_XIdx, m_Left_XIdx_byScrn, m_Top_XIdx_byScrn, EN_width_XIdx, EN_height_XIdx, TRUE);
}

// -------------------------------------------------------------------------------------

void E_View_Ind_Wnd::On_MOUSEMOVE___E_View(const int pos_x_onIndWnd)
{
	// E_View が、Screen 上で動いてないため、差分で算出できる
	m_Left_XIdx_byScrn += pos_x_onIndWnd - m_X_cur_Ind;


	SelectObject(m_hDC, m_hPen_Erase);
	MoveToEx(m_hDC, m_X_cur_Ind, 0, NULL);
	LineTo(m_hDC, m_X_cur_Ind, m_Height);

	SelectObject(m_hDC, m_hPen_Ind);
	MoveToEx(m_hDC, pos_x_onIndWnd, 0, NULL);
	LineTo(m_hDC, pos_x_onIndWnd, m_Height);

	m_X_cur_Ind = pos_x_onIndWnd;

	// ------------------------------------------------
	ms_pos_x_onIndWnd = DWORD(pos_x_onIndWnd);
	G_UINT_to_6Dec_Spc(ms_col_view_left + ms_pos_x_onIndWnd, msa_SB_XIdx);
	MoveWindow(m_hWnd_XIdx, m_Left_XIdx_byScrn, m_Top_XIdx_byScrn, EN_width_XIdx, EN_height_XIdx, FALSE);
	E_View_Ind_Wnd::On_WM_PAINT_XIdx_Wnd();
}

////////////////////////////////////////////////////////////////////////////////////////

const CRetMsg* E_View_Ind_Wnd::S_Rgst_XIdx_WndClass()
{
	static bool ss_Rgst_Child_WndClass = false;
	if (ss_Rgst_Child_WndClass) { return NULL; }
	ss_Rgst_Child_WndClass = true;

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = E_View_Ind_Wnd::S_XIdx_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = KAppGlobal::HInstance();
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"E_View_XIdx_Wnd";
	wc.hIconSm = NULL;

	if (RegisterClassEx(&wc) == 0)
	{ ms_RetMsg.CSET_MSG(L"!!! RegisterClass() に失敗しました。");  return &ms_RetMsg; }

	return NULL;
}

// -------------------------------------------------------------------------------------

LRESULT CALLBACK E_View_Ind_Wnd::S_XIdx_WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PAINT)
	{
		ValidateRect(hWnd, NULL);

//		E_View_Ind_Wnd* pInd_Wnd = (E_View_Ind_Wnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		E_View_Ind_Wnd::On_WM_PAINT_XIdx_Wnd();

		return 0;
	}

	return DefWindowProc(hWnd , msg , wp , lp);
}

// -------------------------------------------------------------------------------------

void E_View_Ind_Wnd::On_WM_PAINT_XIdx_Wnd()
{
	Rectangle(ms_hDC_XIdx, 0, 0, EN_width_XIdx, EN_height_XIdx);
	TextOut(ms_hDC_XIdx, 5, 2, msa_SB_XIdx, 6);
}

// -------------------------------------------------------------------------------------

void E_View_Ind_Wnd::On_Chg_col_view_left(int new_col_view_left)
{
	ms_col_view_left = DWORD(new_col_view_left);
	G_UINT_to_6Dec_Spc(ms_col_view_left + ms_pos_x_onIndWnd, msa_SB_XIdx);
	E_View_Ind_Wnd::On_WM_PAINT_XIdx_Wnd();
}
