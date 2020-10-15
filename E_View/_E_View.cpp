#include "stdafx.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define NEW  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW  new
#endif

#include <windowsx.h>  // GET_X_LPARAM のため

#include "_KGeneral.h"
#include "__KAppGlobal.h"

#include "CRetMsg.h"
#include "_E_View_Mdt.h"

#include "_E_View.h"
#include "__Facade.h"

// OutputDebugString() 用
//#include <Windows.h>

extern bool g_DBG_Push_Close_Btn;


////////////////////////////////////////////////////////////////////////////////////////

namespace __E_View
{
	// プロトタイプ
	// キャレット縦線インジケータウィンドウ
	E_View_Ind_Wnd* Get_E_View_Ind_Wnd();
}

////////////////////////////////////////////////////////////////////////////////////////

class AP_E_View
{
public:
	void Crt_Body(DWORD base_price, DWORD initl_MIN_Pr, DWORD initl_MAX_Pr, E_View_Root_Vntfy* pVntfy, const EI_PrAry* pEI_PrAry)
	{
		if (m_pEview)
		{ throw KEXCEPTION(L"!!! AP_E_View が２度 Crt_Body() されました。"); }

		m_pEview = NEW E_View(base_price, initl_MIN_Pr, initl_MAX_Pr, pVntfy);
		m_pEview->m_pE_PrAry = pEI_PrAry;
	}

	AP_E_View() {}
	~AP_E_View() {
		if (m_pEview) { delete m_pEview; }
	}

	E_View* operator->() { return m_pEview; }
	E_View* GetPtr() { return m_pEview; }

private:
	E_View* m_pEview = NULL;
}
static sAP_E_View_USDJPY;

// -------------------------------------------------------------------------------------

namespace __E
{
	// 公開メソッド
	void G_E_View_Crt_Body(DWORD base_price, DWORD initl_MIN_Pr, DWORD initl_MAX_Pr, E_View_Root_Vntfy* pVntfy)
	{
		sAP_E_View_USDJPY.Crt_Body(base_price, initl_MIN_Pr, initl_MAX_Pr, pVntfy, E_View_Mdt::S_GetBody_E_PrAry());
	}

	E_View* G_E_GetBody_E_View() { return sAP_E_View_USDJPY.GetPtr(); }

	// ---------------------------------------------
	// 特殊用途（必要に迫られた場合、メソッド名を変えて、使用先を調査する）
	DWORD E_View_Get_pcs_inv_offPr___Inv() { return sAP_E_View_USDJPY->Get_pcs_inv_offPr_s___Inv(); }
	const DWORD* E_View_Get_pTop_inv_offPr_s___Inv() { return sAP_E_View_USDJPY->Get_pTop_inv_offPr_s___Inv();; }
}


// 公開メソッド
// 基本のコール順序：
// G_E_View_Make_ColAry() -> G_E_Anlz_Base_Exec() -> G_E_View_Open() -> F_View_Open()
[[nodiscard]] const CRetMsg* __E::to_E_Body::E_View_Make_ColAry(DWORD pcs_unit) { return sAP_E_View_USDJPY->Make_ColAry(pcs_unit); }
[[nodiscard]] const CRetMsg* __E::to_E_Body::E_View_Open() { return sAP_E_View_USDJPY->OpenWindow(); }

//const DWORD* E_View_Root::Get_pCoeff_Pr_to_pix() { return sAP_E_View_USDJPY->Get_pCoeff_Pr_to_pix(); }
const DWORD* __E_View::Get_pCoeff_Pr_to_pix() { return sAP_E_View_USDJPY->Get_pCoeff_Pr_to_pix(); }


////////////////////////////////////////////////////////////////////////////////////////
// コンストラクタ ＆ デストラクタ

E_View::E_View(DWORD base_price, DWORD initl_MIN_Pr, DWORD initl_MAX_Pr, E_View_Root_Vntfy* pVntfy)
	: mc_BasePrice{ base_price }
	, mc_max_Pr_onSystem{ base_price + 0xFFFFu }
	, mc_min_Pr_onSystem{ base_price + 0x100u }
	, mc_pRuler_H{ NEW E_View_Ruler_H{ this, base_price } }
	, mc_pRuler_V{ NEW E_View_Ruler_V{ this } }
	, mc_pScrl_H{ NEW E_View_Scrl_H{ this, &this->m_col_view_left, mc_pRuler_H, mc_pRuler_V } }
	, mc_pScrl_V{ NEW E_View_Scrl_V{this, mc_pRuler_H, mc_pRuler_V, &this->m_max_Pr_onView, &this->m_min_Pr_onView } }
	, mc_pVntfy_Root{ pVntfy }
	, mc_pInd_Wnd{ __E_View::Get_E_View_Ind_Wnd() }
{
	m_max_Pr_onView = initl_MAX_Pr;
	m_min_Pr_onView = initl_MIN_Pr;

	m_coeff_Pr_to_pix = (se_View_size.m_Height << 16) / (m_max_Pr_onView - m_min_Pr_onView + 1);

	m_pix_y_View_top = int(((0xFFFFu - (m_max_Pr_onView - mc_BasePrice)) * m_coeff_Pr_to_pix) >> 16);
	m_pix_y_View_btm = int(((0xFFFFu - (m_min_Pr_onView - mc_BasePrice)) * m_coeff_Pr_to_pix) >> 16);
}

// -------------------------------------------------------------------------------------

E_View::~E_View()
{
	if (m_hPen_plot_data) { DeleteObject(m_hPen_plot_data); }
	if (m_hBrush_View_bkgd) { DeleteObject(m_hBrush_View_bkgd); }
	if (m_hFont) { DeleteObject(m_hFont); }

	if (m_pTop_inv_off_Pr_s) { delete[] m_pTop_inv_off_Pr_s; }
	if (m_pTop_pix_y_s) { delete[] m_pTop_pix_y_s; }
	if (m_pTop_col_blk_mark) { delete[] m_pTop_col_blk_mark; }

	delete mc_pRuler_H;
	delete mc_pRuler_V;

	delete mc_pScrl_H;
	delete mc_pScrl_V;
}


////////////////////////////////////////////////////////////////////////////////////////

static bool sb_Release_Wnd_Rsc = false;

static LRESULT CALLBACK S_WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		E_View* const pE_View = (E_View*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pE_View->Hndlr_WM_PAINT(hdc, &ps);

		EndPaint(hWnd, &ps);
		} return 0; 

	case WM_HSCROLL: {
		E_View* const pE_View = (E_View*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pE_View->mc_pScrl_H->Hndlr_WM_HSCROLL(wp);
		} return 0;

	case WM_KEYDOWN: {
		E_View* const pE_View = (E_View*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		switch(wp)
		{
		case VK_RIGHT:
			if (GetKeyState(VK_SHIFT) < 0)
			{ pE_View->mc_pScrl_H->Move_Left(E_View_Scrl_H::PIXS_MOVE_PAGE_H); }
			else
			{ pE_View->mc_pScrl_H->Move_Left(E_View_Scrl_H::PIXS_MOVE_LINE_H); }
			return 0;

		case VK_LEFT:
			if (GetKeyState(VK_SHIFT) < 0)
			{ pE_View->mc_pScrl_H->Move_Right(E_View_Scrl_H::PIXS_MOVE_PAGE_H); }
			else
			{ pE_View->mc_pScrl_H->Move_Right(E_View_Scrl_H::PIXS_MOVE_LINE_H); }
			return 0;

		case VK_DOWN:
			if (GetKeyState(VK_SHIFT) < 0)
			{ pE_View->mc_pScrl_V->Move_Down(E_View_Scrl_V::PIXS_MOVE_PAGE_V); }
			else
			{ pE_View->mc_pScrl_V->Move_Down(E_View_Scrl_V::PIXS_MOVE_LINE_V); }
			return 0;

		case VK_UP:
			if (GetKeyState(VK_SHIFT) < 0)
			{ pE_View->mc_pScrl_V->Move_Up(E_View_Scrl_V::PIXS_MOVE_PAGE_V); }
			else
			{ pE_View->mc_pScrl_V->Move_Up(E_View_Scrl_V::PIXS_MOVE_LINE_V); }
			return 0;
		}
		} break;

	case WM_MOVE: {
		E_View* const pE_View = (E_View*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (pE_View == NULL) { return 0; }

		pE_View->mc_pInd_Wnd->On_WM_MOVE___E_View();
		} return 0;

	case WM_MOUSEMOVE: {
		const int pos_X_onIndWnd = (int)(short)LOWORD(lp) - se_View_pos.m_Left;

		if (0 <= pos_X_onIndWnd && pos_X_onIndWnd < se_View_size.m_Width)
		{
			E_View* const pE_View = (E_View*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			pE_View->mc_pInd_Wnd->On_MOUSEMOVE___E_View(pos_X_onIndWnd);
		}
		} return 0;
	}

	if (g_DBG_Push_Close_Btn == true && sb_Release_Wnd_Rsc == false)
	{
		sb_Release_Wnd_Rsc = true;

		E_View* const pE_View = (E_View*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		HDC* phDC = &pE_View->m_hDC;

		if (*phDC)
		{
			SelectObject(*phDC, GetStockObject(BLACK_PEN));		// ペンの解放
			SelectObject(*phDC, GetStockObject(WHITE_BRUSH));	// ブラシの解放
			SelectObject(*phDC, GetStockObject(SYSTEM_FONT));	// フォントの解放
			ReleaseDC(pE_View->m_hWnd, *phDC);

			pE_View->mc_pRuler_H->Delete_GDIObj();
			pE_View->mc_pRuler_V->Delete_GDIObj();

			pE_View->mc_pInd_Wnd->Release_Rsc();

			*phDC = NULL;
		}

//		OutputDebugString(L"--- E_View のウィンドウリソースを解放しました\r\n");
	}

	return DefWindowProc(hWnd , msg , wp , lp);
}

// -------------------------------------------------------------------------------------

static const CRetMsg* S_Init_WndClass(CRetMsg* pret_msg)
{
	WNDCLASS wc;

//	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.style = 0;
	wc.lpfnWndProc = S_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = KAppGlobal::HInstance();
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"HS3_BMP";

	if (RegisterClass(&wc) == 0)
	{ pret_msg->CSET_MSG(L"!!! RegisterClass() に失敗しました。");  return pret_msg; }

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////

// 後で、Open、Hide の’機能も実装すること

const CRetMsg* E_View::OpenWindow()
{
	if (mb_IsCreated) { return NULL; }
	mb_IsCreated = true;

	if (msb_IsInit_WndClass == false)
	{
		const CRetMsg* pret_msg = S_Init_WndClass(&m_RetMsg);
		if (pret_msg) { return pret_msg; }

		msb_IsInit_WndClass = true;
	}

	const auto [clt_width, clt_height] = [&]() -> std::pair<int, int> {
		// AdjustWindowRect() を利用するための RECT
		RECT rect { 0, 0, long(se_View_size.m_Width + se_View_mgn.m_Left + se_View_mgn.m_Right),
						  long(se_View_size.m_Height + se_View_mgn.m_Top + se_View_mgn.m_Btm) };

		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW,
			FALSE);  // メニューフラグ

		const int scrlbar_width = GetSystemMetrics(SM_CXVSCROLL);
		const int scrlbar_height = GetSystemMetrics(SM_CYHSCROLL);

		return { rect.right - rect.left + scrlbar_width, rect.bottom - rect.top + scrlbar_height };
	}();

	m_hWnd = CreateWindow(
		L"HS3_BMP",  // class name
		L"view window",  // window name
		WS_OVERLAPPED | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_SYSMENU,  // style
		se_OL_Wnd_pos.m_Left , se_OL_Wnd_pos.m_Top,	// left, top
		clt_width, clt_height,
		NULL,  // hWndParent
		NULL,  // hMenu
		KAppGlobal::HInstance(),
		NULL);  // WM_CREATE メッセージの lParam パラメータとして渡される構造体ポインタ

	if (m_hWnd == NULL)
	{ m_RetMsg.CSET_MSG(L"!!! CreateWindow() に失敗しました。"); return &m_RetMsg; }

	m_hDC = GetDC(m_hWnd);
	if (m_hDC == NULL)
	{ m_RetMsg.CSET_MSG(L"!!! GetDC() に失敗しました。"); return &m_RetMsg; }

	// View の原点を設定しておく
	SetViewportOrgEx(m_hDC, se_View_mgn.m_Left, se_View_mgn.m_Top,
		NULL);  // LPPOINT: 変更前の left, top が欲しい場合に利用する

	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	// -------------------------------------------
	m_hPen_plot_data = CreatePen(PS_SOLID, 1, __E::EN_COLOR_PlotData);
	m_hBrush_View_bkgd = CreateSolidBrush(__E::EN_COLOR_View_bkgd);

	m_hFont = CreateFont(
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

	SelectObject(m_hDC, m_hBrush_View_bkgd);
	SelectObject(m_hDC, m_hFont);

	// m_hWnd、m_hDC が設定された後で Init される
	mc_pRuler_H->InitOnce_after_CreateDC(m_hDC);
	mc_pRuler_H->Update_Setting();

	mc_pRuler_V->InitOnce_after_CreateDC(m_hDC);
	// s_E_View_Ruler_V.Update_Setting()  // これは、MakeCol でコールされる（利用する Pr の idx数、col数 などが必要となる）

	mc_pScrl_H->InitOnce_after_CreateDC(m_hWnd, m_hDC);
	mc_pScrl_V->InitOnce_after_CreateDC(m_hWnd, m_hDC);

	mc_pVntfy_Root->Vntfy_InitOnce_after_CreateDC(this, m_hDC);

	if (m_pcs_col)
	{ mc_pScrl_H->On_InitCol(m_pcs_col); }

	const CRetMsg* ret_msg = mc_pInd_Wnd->Open_Ind_Wnd(se_View_pos, m_hWnd);
	if (ret_msg) { return ret_msg; }

	return NULL;
}

// -------------------------------------------------------------------------------------

void E_View::Hndlr_WM_PAINT(HDC hdc, LPPAINTSTRUCT pps)
{
	// View の外枠
	Rectangle(hdc, se_View_pos.m_Left - 1, se_View_pos.m_Top - 1, se_View_pos.m_Right + 1, se_View_pos.m_Btm + 1);

	const int ps_left = pps->rcPaint.left;
	const int ps_right = pps->rcPaint.right;
	if (ps_left >= se_View_pos.m_Right) { mc_pRuler_H->Draw_Label_only();  return; }
	if (ps_right <= se_View_pos.m_Left) { mc_pRuler_H->Draw_Label_only();  return; }

	const int pos_x_start = ps_left < se_View_pos.m_Left ? 0 : ps_left - se_View_pos.m_Left;
	const int pos_x_tmnt = KMin(ps_right - se_View_pos.m_Left, se_View_size.m_Width);

	mc_pRuler_H->Draw_Ruler_H(pos_x_start, pos_x_tmnt);
	mc_pRuler_V->Draw_Ruler_V(pos_x_start, pos_x_tmnt);

	this->DrawView(pos_x_start, pos_x_tmnt);
}

// -------------------------------------------------------------------------------------
// 0 <= col_start < col_tmnt <= View_width

void E_View::DrawView(int pos_x_start, int pos_x_tmnt) const
{
/*
	if (pos_x_tmnt <= pos_x_start || pos_x_tmnt > se_View_size.m_Width)
	{ throw KEXCEPTION(L"クリティカルエラー： pos_x_tmnt <= pos_x_start || pos_x_tmnt > se_View_size.m_Width"); }
*/

	if (m_pcs_col <= m_col_view_left) { return; }  // データ未設定時など。念のため

	const int pos_x_datatmnt = m_pcs_col - m_col_view_left;
	// 描画範囲がデータ未設定領域のときは描画しなくて良い
	if (pos_x_datatmnt <= pos_x_start) { return; }

	pos_x_tmnt = KMin(pos_x_tmnt, pos_x_datatmnt);  // 再設定
	mc_pVntfy_Root->Vntfy_DrawView(m_col_view_left, pos_x_start, pos_x_tmnt, m_pix_y_View_top);

	const DWORD* p_pix_y_s = m_pTop_pix_y_s + m_col_view_left + DWORD(pos_x_start);

	// ループ内の最適化を考慮
	const int cpix_y_top = m_pix_y_View_top;
//	const int cpix_y_btm = m_pix_y_View_btm;
	const HDC chDC = m_hDC;
//	const DWORD cView_height = m_View_height;

	// データプロット用のペン
	SelectObject(chDC, m_hPen_plot_data);

	for (int pos_x = pos_x_start; pos_x < pos_x_tmnt; pos_x++)
	{
		const DWORD pix_y_s = *p_pix_y_s++;
		int pos_y_max = int(pix_y_s >> 16) - cpix_y_top;
		int pos_y_min = int(pix_y_s & 0xFFFF) - cpix_y_top + 1;

		if (pos_y_max > se_View_size.m_Height || pos_y_min < 0) { continue; }

		if (pos_y_max < 0) { pos_y_max = 0; }
		if (pos_y_min > se_View_size.m_Height) { pos_y_min = se_View_size.m_Height; }
		
		MoveToEx(chDC, pos_x, pos_y_max, NULL);
		LineTo(chDC, pos_x, pos_y_min);
	}
}


// -------------------------------------------------------------------------------------
// 戻り値： pix_y でみた場合の、top の移動量（ > 0 ）

int E_View::Ntfy_MoveUp(const DWORD dPr)
{
	const int old_pix_y_view_top = m_pix_y_View_top;

	m_max_Pr_onView -= dPr;
	m_min_Pr_onView -= dPr;

	m_pix_y_View_top = int(((0xFFFFu - (m_max_Pr_onView - mc_BasePrice)) * m_coeff_Pr_to_pix) >> 16);
	m_pix_y_View_btm = int(((0xFFFFu - (m_min_Pr_onView - mc_BasePrice)) * m_coeff_Pr_to_pix) >> 16);

	mc_pRuler_H->Update_Setting();

	return m_pix_y_View_top - old_pix_y_view_top;
}


int E_View::Ntfy_MoveDown(const DWORD dPr)
{
	const int old_pix_y_view_top = m_pix_y_View_top;

	m_max_Pr_onView += dPr;
	m_min_Pr_onView += dPr;

	m_pix_y_View_top = int(((0xFFFFu - (m_max_Pr_onView - mc_BasePrice)) * m_coeff_Pr_to_pix) >> 16);
	m_pix_y_View_btm = int(((0xFFFFu - (m_min_Pr_onView - mc_BasePrice)) * m_coeff_Pr_to_pix) >> 16);

	mc_pRuler_H->Update_Setting();

	// old_pix_y_view_top > m_pix_y_View_top となることに注意
	return old_pix_y_view_top - m_pix_y_View_top;
}


////////////////////////////////////////////////////////////////////////////////////////

void E_idx_to_col::Set_pcs_unit(DWORD pcs_unit)
{
	ms_pcs_unit = pcs_unit;
	ms_pcs_SEC = E_View_Mdt::S_GetBody_E_PrAry()->Get_pcs_off_Pr();
	ms_pcs_col = int((ms_pcs_SEC + pcs_unit - 1) / pcs_unit);
}

// -------------------------------------------------------------------------------------

int* E_idx_to_col::Cvt_SEC_to_col(const DWORD* p_SEC_src, const DWORD* const pTmnt_SEC_src, int* p_col_dst)
{
	int col_prev = -1;
	while(true)
	{
		if (p_SEC_src >= pTmnt_SEC_src) { break; }

		int col = int(*p_SEC_src++ / ms_pcs_unit);
		if (col == col_prev)
		{
			*p_col_dst++ = -1;
		}
		else
		{
			*p_col_dst++ = col;
			col_prev = col;
		}
	}
	return p_col_dst;
}


////////////////////////////////////////////////////////////////////////////////////////

// 注意： pcs_unit >= 2 と想定している（<- この束縛は外したつもりであるが、、、）

const CRetMsg* E_View::Make_ColAry(const DWORD pcs_unit)
{
	// pcs_unit == 1 でも大丈夫なようにしたつもりであるが、、
//	if (pcs_unit < 2)
//	{ m_RetMsg.CSET_MSG(L"!!! pcs_unit < 2"); return &m_RetMsg; }

	E_idx_to_col::Set_pcs_unit(pcs_unit);

	const CRetMsg* pret_msg;
	pret_msg = mc_pRuler_V->Update_Setting();	// pcs_unit が決まったら、just hour の col も定まる
	if (pret_msg) { return pret_msg; }

	// inv_off_Pr_s、pix_y_s の配列領域確保 ＆ ブロックマークの処理
	this->Prep_Arys();


	WORD const* p_off_Pr_src = m_pE_PrAry->Get_pTop_off_Pr();
	WORD const* const c_pDataTmnt_off_Pr_src = m_pE_PrAry->Get_pDataTmnt_off_Pr();

	m_pDataTmnt_inv_off_Pr_s = m_pTop_inv_off_Pr_s;
	m_pDataTmnt_pix_y_s = m_pTop_pix_y_s;
	DWORD max_off_Pr, min_off_Pr;


	while (true)
	{
		min_off_Pr = max_off_Pr = *p_off_Pr_src++;

		if (p_off_Pr_src == c_pDataTmnt_off_Pr_src) { break; }

		for (DWORD cnt_pcs_unit = pcs_unit - 1; cnt_pcs_unit > 0; cnt_pcs_unit--)
		{
			const DWORD off_Pr = *p_off_Pr_src++;

			if (off_Pr > max_off_Pr) { max_off_Pr = off_Pr; }
			else if (off_Pr < min_off_Pr) { min_off_Pr = off_Pr; }

			if (p_off_Pr_src == c_pDataTmnt_off_Pr_src) { goto EXIT_WHILE; }
		}

		// pcs_unit 分のデータを走査したため、結果を記録する
		const DWORD max_inv_off_Pr = 0xFFFFu - max_off_Pr;
		const DWORD min_inv_off_pr = 0xFFFFu - min_off_Pr;

		// ===== col 生成 =====
		*m_pDataTmnt_inv_off_Pr_s++ = (max_inv_off_Pr << 16) | min_inv_off_pr;
		*m_pDataTmnt_pix_y_s++ = ((max_inv_off_Pr * m_coeff_Pr_to_pix) & 0xFFFF'0000u) | ((min_inv_off_pr * m_coeff_Pr_to_pix) >> 16);
	}

EXIT_WHILE:
	// エラー検出。念のため
	if (m_pDataTmnt_inv_off_Pr_s >= m_pTmnt_inv_off_Pr_s)
	{ m_RetMsg.CSET_MSG(L"!!! p_inv_off_Pr_s_dst >= m_pTmnt_col_inv_off_Pr"); return &m_RetMsg; }

	const DWORD max_inv_off_Pr = 0xFFFFu - max_off_Pr;
	const DWORD min_inv_off_pr = 0xFFFFu - min_off_Pr;

	// ===== col 生成 =====
	*m_pDataTmnt_inv_off_Pr_s++ = (max_inv_off_Pr << 16) | min_inv_off_pr;
	*m_pDataTmnt_pix_y_s++ = ((max_inv_off_Pr * m_coeff_Pr_to_pix) & 0xFFFF'0000u) | ((min_inv_off_pr * m_coeff_Pr_to_pix) >> 16);

	m_pcs_col = int(m_pDataTmnt_pix_y_s - m_pTop_pix_y_s);
	
	// col 数が定まった後に、H方向のスクロールバーの調整ができる
	if (mb_IsCreated)
	{ mc_pScrl_H->On_InitCol(m_pcs_col); }

	// ----------------------------------------------------

	mc_pVntfy_Root->Vntfy_Crtd_Col_onEView(pcs_unit, m_pE_PrAry->Get_pcs_off_Pr(), DWORD(m_pcs_col));

	return NULL;
}


// -------------------------------------------------------------------------------------
// inv_off_Pr_s、pix_y_s の配列領域確保 ＆ ブロックマークの処理

void E_View::Prep_Arys()
{
	const DWORD pcs_col_to_need = (DWORD)E_idx_to_col::Get_pcs_col_to_need();
	if (m_pcs_alloc_inv_off_Pr_s < pcs_col_to_need)
	{
		if (m_pTop_inv_off_Pr_s) { delete[]  m_pTop_inv_off_Pr_s; }
		if (m_pTop_pix_y_s) { delete[] m_pTop_pix_y_s; }

		m_pTop_inv_off_Pr_s = NEW DWORD[pcs_col_to_need];
		m_pTmnt_inv_off_Pr_s = m_pTop_inv_off_Pr_s + pcs_col_to_need;
		m_pcs_alloc_inv_off_Pr_s = pcs_col_to_need;

		m_pTop_pix_y_s = NEW DWORD[pcs_col_to_need];
	}
//	m_pDataTmnt_col_inv_off_Pr = m_pTop_inv_off_Pr_s;
//	m_pDataTmnt_col_pix = m_pTop_col_pix;

	// -------------------------------------------------------
	// ブロックマークの col 情報生成は簡単なため、この関数内で終わらせておく

	const DWORD pcs_idx_blk_mark = m_pE_PrAry->Get_pcs_idx_blk_mark();

	if (m_pcs_alloc_col_blk_mark < pcs_idx_blk_mark)
	{
		if (m_pTop_col_blk_mark) { delete[] m_pTop_col_blk_mark; }

		m_pTop_col_blk_mark = NEW int[pcs_idx_blk_mark];
		m_pTmnt_col_blk_mark = m_pTop_col_blk_mark + pcs_idx_blk_mark;
		m_pcs_alloc_col_blk_mark = pcs_idx_blk_mark;
	}

	m_pDataTmnt_col_blk_mark = E_idx_to_col::Cvt_SEC_to_col(
			m_pE_PrAry->Get_pTop_idx_blk_mark(), m_pE_PrAry->Get_pDataTmnt_idx_blk_mark(), m_pTop_col_blk_mark);
}


////////////////////////////////////////////////////////////////////////////////////////

void E_View_Pr_CStr::SetPrice(DWORD price)
{
	// 最上位だけは別処理とする
	UINT64 ui64str_1;
	if (price > 999'999)
	{
		ui64str_1 = L'?';
		price %= 100'000;
	}
	else
	{
		if (price > 99'999)
		{
			ui64str_1 = L'0' + price / 100'000;
			price %= 100'000;
		}
		else
		{
			ui64str_1 = L' ';
		}
	}

	// 残り５桁の処理（MXN, ZAR は４桁であるが、その処理はあえて無視する）
	const UINT64 dig1 = price / 10'000;
	price %= 10'000;
	const UINT64 dig2 = price / 1'000;
	price %= 1'000;
	const UINT64 dig3 = price / 100;
	price %= 100;

	ui64str_1 += KSTR4(0, dig1, dig2, 0) + KSTR4(0, L'0', L'0', L'.');
	*(UINT64*)ma_pCStr = ui64str_1;
	*(UINT64*)(ma_pCStr + 4) = KSTR4(dig3, price / 10, price % 10, 0) + KSTR4(L'0', L'0', L'0', 0);
}
