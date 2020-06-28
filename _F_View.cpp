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

#include "__Facade.h"

#include "_F_View.h"


extern bool g_DBG_Push_Close_Btn;

////////////////////////////////////////////////////////////////////////////////////////

CRetMsg F_View::ms_RetMsg;

static F_View_OL_Wnd_pos constexpr se_F_Wnd_pos = { 100, 50 };

// F_View の実体（現時点では、１つと考えている）
static F_View s_F_View{ se_F_Wnd_pos };

// -------------------------------------------------------------------------------------

namespace __F
{
	void G_F_View_Open___S_DI_Btn_E_Analyze()
	{
		const CRetMsg* pret_msg = s_F_View.OpenWindow();
		if (pret_msg) { pret_msg->Show_toStDisp(); }
	}

	// デバッグ用
	void G_F_View_Attach_Child_View___S_DI_Btn_E_Analyze(Vntfy_F_View* pChild)
	{
		s_F_View.Attach_Child_View(pChild, 0, 0);
	}
}


////////////////////////////////////////////////////////////////////////////////////////

void F_View::Attach_Child_View(Vntfy_F_View* pChild, const DWORD idx_col, const DWORD idx_row, const DWORD len_col, const DWORD len_row)
{
	if (idx_col >= EN_col_child_view)
	{ throw KEXCEPTION(L"!!! pos_col >= EN_col_child_view"); }

	if (idx_row >= EN_row_child_view)
	{ throw KEXCEPTION(L"!!! idx_row >= EN_row_child_view"); }

	if (idx_col + len_col - 1 >= EN_col_child_view)
	{ throw KEXCEPTION(L"!!! idx_col + len_col - 1 >= EN_col_child_view"); }

	if (idx_row + len_row - 1 >= EN_row_child_view)
	{ throw KEXCEPTION(L"!!! idx_row + len_row - 1 >= EN_row_child_view"); }

	// --------------------------------------------------

	if (pChild->m_pPrnt_F_View)
	{ throw KEXCEPTION(L"F_View の child view が、２重に登録されました。"); }

	pChild->m_pPrnt_F_View = this;

	if (m_pView_Child_top == NULL)
	{
		m_pView_Child_top = pChild;
		m_pView_Child_last = pChild;
	}
	else
	{
		m_pView_Child_last->m_pNext = pChild;
		m_pView_Child_last = pChild;
	}

	// --------------------------------------------------

	pChild->m_pos_on_F_View.m_idx_col = idx_col;
	pChild->m_pos_on_F_View.m_idx_row = idx_row;

	pChild->m_pos_on_F_View.m_len_col = len_col;
	pChild->m_pos_on_F_View.m_len_row = len_row;

	// --------------------------------------------------
	// 位置決めのための mtx を準備
	Vntfy_F_View** p_pChild = ma_child_view_mtx + idx_col + idx_row * EN_col_child_view;

	for (DWORD row = len_row; row > 0; --row)
	{
		Vntfy_F_View** p_pChild_inner = p_pChild;
		for (DWORD col = len_col; col > 0; --col)
		{
			if (*p_pChild_inner != NULL)
			{ throw KEXCEPTION(L"!!! ma_child_view_mtx に重複部分があることを検知しました。"); }

			*p_pChild_inner++ = pChild;
		}
		p_pChild += EN_col_child_view;
	}
}

// -------------------------------------------------------------------------------------
// ma_child_view_mtx の情報を元にして、child view の位置決めを行っていく

void F_View::Set_Child_View_pos()
{
	// 位置決めのために、m_pos_on_F_View の m_width_rem, m_height_rem を設定する
	// anlz 中に、m_view_size を適宜調節してよいようにするため、ここで m_width_rem, m_height_rem を調整する

	for (Vntfy_F_View* pChild = m_pView_Child_top; pChild != NULL; pChild = pChild->m_pNext)
	{
		pChild->m_pos_on_F_View.m_width_rem = pChild->m_mgn.m_Left + pChild->m_mgn.m_Right
											+ pChild->m_padding.m_Left + pChild->m_padding.m_Right + pChild->m_view_size.m_Width;

		pChild->m_pos_on_F_View.m_height_rem = pChild->m_mgn.m_Top + pChild->m_mgn.m_Btm
											+ pChild->m_padding.m_Top + pChild->m_padding.m_Btm + pChild->m_view_size.m_Height;
	}

	// --------------------------------------------------

	int a_col_width[EN_col_child_view] = {};
	int a_row_height[EN_row_child_view] = {};

	Vntfy_F_View* const* const pTmnt_pChild = ma_child_view_mtx + EN_col_child_view * EN_row_child_view;

	// column の幅を決める
	for (int len_col = 1; len_col <= EN_col_child_view; ++len_col)
	{
		for (int idx_col = len_col - 1; idx_col < EN_col_child_view; ++idx_col)
		{
			int width_col = a_col_width[idx_col];

			for (Vntfy_F_View** p_pChild = ma_child_view_mtx + idx_col; p_pChild < pTmnt_pChild; )
			{
				if (*p_pChild != NULL && (*p_pChild)->m_pos_on_F_View.m_len_col == len_col)
				{
					Vntfy_F_View::ChildPos& rchild_pos = (*p_pChild)->m_pos_on_F_View;

					int width_rem = rchild_pos.m_width_rem;
					if (width_rem > width_col) { width_col = width_rem; }

					p_pChild += EN_col_child_view * rchild_pos.m_len_row;
				}
				else
				{
					p_pChild += EN_col_child_view;
				}
			}

			a_col_width[idx_col] = width_col;

			for (Vntfy_F_View** p_pChild = ma_child_view_mtx + idx_col; p_pChild < pTmnt_pChild; )
			{
				if (*p_pChild != NULL)
				{
					Vntfy_F_View::ChildPos& rchild_pos = (*p_pChild)->m_pos_on_F_View;

					int width_rem = rchild_pos.m_width_rem;
					rchild_pos.m_width_rem = width_rem <= width_col ? 0 : width_rem - width_col;

					p_pChild += EN_col_child_view * rchild_pos.m_len_row;
				}
				else
				{
					p_pChild += EN_col_child_view;
				}
			}
		}
	}

	// --------------------------------------------------
	// row の高さを決める
	for (int len_row = 1; len_row <= EN_row_child_view; ++len_row)
	{
		for (int idx_row = len_row - 1; idx_row < EN_row_child_view; ++idx_row)
		{
			int height_row = a_row_height[idx_row];

			for(Vntfy_F_View** p_pChild = ma_child_view_mtx + idx_row * EN_col_child_view; p_pChild < pTmnt_pChild; )
			{
				if (*p_pChild != NULL && (*p_pChild)->m_pos_on_F_View.m_len_row == len_row)
				{
					Vntfy_F_View::ChildPos& rchild_pos = (*p_pChild)->m_pos_on_F_View;

					int height_rem = rchild_pos.m_height_rem;
					if (height_rem > height_row) { height_row = height_rem; }

					p_pChild += rchild_pos.m_len_col;
				}
				else
				{
					p_pChild++;
				}
			}

			a_row_height[idx_row] = height_row;

			for(Vntfy_F_View** p_pChild = ma_child_view_mtx + idx_row * EN_col_child_view; p_pChild < pTmnt_pChild; )
			{
				if (*p_pChild != NULL)
				{
					Vntfy_F_View::ChildPos& rchild_pos = (*p_pChild)->m_pos_on_F_View;

					int height_rem = rchild_pos.m_height_rem;
					rchild_pos.m_height_rem = height_rem <= height_row ? 0 : height_rem - height_row;

					p_pChild += rchild_pos.m_len_col;
				}
				else
				{
					p_pChild++;
				}
			}
		}
	}

	// --------------------------------------------------
	// 各 child の位置を書き込む
	int a_col_left[EN_col_child_view];
	int a_row_top[EN_row_child_view];

	{
		int left = 0;
		for (int idx_col = 0; idx_col < EN_col_child_view; ++idx_col)
		{
			a_col_left[idx_col] = left;
			left += a_col_width[idx_col];
		}
		m_width_Clt = left < 100 ? 100 : left;
	}

	{
		int top = 0;
		for (int idx_row = 0; idx_row < EN_row_child_view; ++idx_row)
		{
			a_row_top[idx_row] = top;
			top += a_row_height[idx_row];
		}
		m_Height_Clt = top < 100 ? 100 : top;
	}

	for (Vntfy_F_View* pChild = m_pView_Child_top; pChild != NULL; pChild = pChild->m_pNext)
	{
		Vntfy_F_View::ChildPos& rchild_pos = pChild->m_pos_on_F_View;

		rchild_pos.m_Left = a_col_left[rchild_pos.m_idx_col];
		rchild_pos.m_Top = a_row_top[rchild_pos.m_idx_row];
	}
}


////////////////////////////////////////////////////////////////////////////////////////

void F_View::Delete_GDIObj()
{
	if (m_hDC_Prnt)
	{
		SelectObject(m_hDC_Prnt, GetStockObject(BLACK_PEN));		// ペンの解放
		SelectObject(m_hDC_Prnt, GetStockObject(WHITE_BRUSH));	// ブラシの解放
		SelectObject(m_hDC_Prnt, GetStockObject(SYSTEM_FONT));	// フォントの解放
	}

	// 子ウィンドウに、ウィンドウリソースの解放を通知する
	for (Vntfy_F_View* pChild = m_pView_Child_top; pChild != NULL; pChild = pChild->m_pNext)
	{
		if (pChild->m_hDC)
		{
			SelectObject(pChild->m_hDC, GetStockObject(BLACK_PEN));		// ペンの解放
			SelectObject(pChild->m_hDC, GetStockObject(WHITE_BRUSH));	// ブラシの解放
			SelectObject(pChild->m_hDC, GetStockObject(SYSTEM_FONT));	// フォントの解放
		}

		pChild->Vntfy_Delete_GDIObj();
		if (pChild->m_hDC)
		{
			ReleaseDC(pChild->m_hWnd, pChild->m_hDC);
			pChild->m_hDC = NULL;
		}
	}

	if (m_hDC_Prnt)
	{
		ReleaseDC(m_hWnd_Prnt, m_hDC_Prnt);
		m_hDC_Prnt = NULL;
	}

	if (ms_hFont)
	{
		DeleteObject(ms_hFont);
		ms_hFont = NULL;
	}

//	OutputDebugString(L"--- F_View のウィンドウリソースを解放しました。\r\n");
}

// -------------------------------------------------------------------------------------

const CRetMsg* F_View::OpenWindow()
{
	if (mb_IsCreated) { return NULL; }
	mb_IsCreated = true;

	const CRetMsg* pret_msg = S_Rgst_Parent_WndClass();
	if (pret_msg) { return pret_msg; }

	pret_msg = S_Rgst_Child_WndClass();
	if (pret_msg) { return pret_msg; }

	// Child View の位置決め、client window のサイズを決定する
	this->Set_Child_View_pos();

	const auto [ol_wnd_width, ol_wnd_height] = [&]() -> std::pair<int, int> {
		// AdjustWindowRect() を利用するための RECT
		RECT rect { 0, 0, m_width_Clt, m_Height_Clt};

		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW,
			FALSE);  // メニューフラグ

		return { rect.right - rect.left, rect.bottom - rect.top };
	}();
	
	m_hWnd_Prnt = CreateWindow(
		L"F_View",  // class name
		L"F_View",  // window name
		WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU,	// style
		m_OL_Wnd_pos.m_Left , m_OL_Wnd_pos.m_Top,	// left, top
		ol_wnd_width, ol_wnd_height,
		NULL,  // hWndParent
		NULL,  // hMenu
		KAppGlobal::HInstance(),
		NULL);  // WM_CREATE メッセージの lParam パラメータとして渡される構造体ポインタ

	if (m_hWnd_Prnt == NULL)
	{ ms_RetMsg.CSET_MSG(L"!!! CreateWindow() に失敗しました。"); return &ms_RetMsg; }

	m_hDC_Prnt = GetDC(m_hWnd_Prnt);
	if (m_hDC_Prnt == NULL)
	{ ms_RetMsg.CSET_MSG(L"!!! GetDC() に失敗しました。"); return &ms_RetMsg; }

	SetWindowLongPtr(m_hWnd_Prnt, GWLP_USERDATA, (LONG_PTR)this);

	// -------------------------------------------

	ms_hFont = CreateFont(
		15,		// nHeight
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

//	SelectObject(ms_hDC_Clt, ms_hFont);

	return this->Open_Child_Windows();
}

// -------------------------------------------------------------------------------------

const CRetMsg* F_View::Open_Child_Windows()
{
	for (Vntfy_F_View* pChild = m_pView_Child_top; pChild != NULL; pChild = pChild->m_pNext)
	{
		const int child_width = pChild->m_view_size.m_Width + pChild->m_padding.m_Left + pChild->m_padding.m_Right;
		const int child_height = pChild->m_view_size.m_Height + pChild->m_padding.m_Top + pChild->m_padding.m_Btm;

		HWND hWnd_child = CreateWindow(
			L"F_View_CHILD",	// class name
			NULL,				// window name 
			WS_CHILD | WS_VISIBLE,  // style
			pChild->m_pos_on_F_View.m_Left + pChild->m_mgn.m_Left,	// left
			pChild->m_pos_on_F_View.m_Top + pChild->m_mgn.m_Top,	// top
			child_width, child_height,
			m_hWnd_Prnt,	// hWndParent
			NULL,			// hMenu
			KAppGlobal::HInstance(),
			NULL  // WM_CREATE メッセージの lParam パラメータとして渡される構造体ポインタ
		);

		if (hWnd_child == NULL)
		{ ms_RetMsg.CSET_MSG(L"!!! CreateWindow() に失敗しました。"); return &ms_RetMsg; }

		HDC hDC_child = GetDC(hWnd_child);
		if (hDC_child == NULL)
		{ ms_RetMsg.CSET_MSG(L"!!! GetDC() に失敗しました。"); return &ms_RetMsg; }
	
		SetWindowLongPtr(hWnd_child, GWLP_USERDATA, (LONG_PTR)pChild);

		pChild->m_hWnd = hWnd_child;
		pChild->m_hDC = hDC_child;

		SetViewportOrgEx(hDC_child, pChild->m_padding.m_Left, pChild->m_padding.m_Top,
		NULL);  // LPPOINT: 変更前の left, top が欲しい場合に利用する

		pChild->Vntfy_Crt_Window();
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////

const CRetMsg* F_View::S_Rgst_Parent_WndClass()
{
	static bool ss_Rgst_Parent_WndClass = false;
	if (ss_Rgst_Parent_WndClass) { return NULL; }
	ss_Rgst_Parent_WndClass = true;

	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = F_View::S_Parent_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = KAppGlobal::HInstance();
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"F_View";

	if (RegisterClass(&wc) == 0)
	{ ms_RetMsg.CSET_MSG(L"!!! RegisterClass() に失敗しました。");  return &ms_RetMsg; }

	return NULL;
}

// -------------------------------------------------------------------------------------

static bool sb_Release_Parent_Wnd_Rsc = false;

LRESULT CALLBACK F_View::S_Parent_WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// WM_CLOSE、WM_DESTROY メッセージを捕捉できないため、以下の手段を用いている
	if (g_DBG_Push_Close_Btn == true && sb_Release_Parent_Wnd_Rsc == false)
	{
		sb_Release_Parent_Wnd_Rsc = true;

		F_View* const pF_View = (F_View*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pF_View->Delete_GDIObj();
	}

	return DefWindowProc(hWnd , msg , wp , lp);
}

// -------------------------------------------------------------------------------------

const CRetMsg* F_View::S_Rgst_Child_WndClass()
{
	static bool ss_Rgst_Child_WndClass = false;
	if (ss_Rgst_Child_WndClass) { return NULL; }
	ss_Rgst_Child_WndClass = true;

	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = F_View::S_Child_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = KAppGlobal::HInstance();
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"F_View_CHILD";

	if (RegisterClass(&wc) == 0)
	{ ms_RetMsg.CSET_MSG(L"!!! RegisterClass() に失敗しました。");  return &ms_RetMsg; }

	return NULL;
}

// -------------------------------------------------------------------------------------


LRESULT CALLBACK F_View::S_Child_WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PAINT)
	{
		ValidateRect(hWnd, NULL);

		Vntfy_F_View* const pVntfy = (Vntfy_F_View*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pVntfy->Vntfy_Draw();

		return 0;
	}

	return DefWindowProc(hWnd , msg , wp , lp);
}
