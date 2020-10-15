#pragma once

#include "_E_View_Root.h"

////////////////////////////////////////////////////////////////////////////////////////

// overlapped ウィンドウの位置（大きさは View のサイズから自動的に算出される）
struct { int m_Left; int m_Top; } static inline constexpr se_OL_Wnd_pos = { 100, 300 };

namespace __E
{
	enum : COLORREF {
		EN_COLOR_View_bkgd = RGB(255, 255, 255),
		EN_COLOR_PlotData = RGB(180, 180, 180),

		EN_COLOR_main_rl_H = RGB(210, 210, 210),
		EN_COLOR_sub_rl_H = RGB(235, 235, 235),

		EN_COLOR_blk_nark_rl_V = RGB(180, 180, 255),
		EN_COLOR_HOUR_rl_V = RGB(235, 235, 235),
	};
}


////////////////////////////////////////////////////////////////////////////////////////
// static クラス

class E_idx_to_col
{
	static inline DWORD ms_pcs_unit = 0;
	static inline DWORD ms_pcs_SEC = 0;
	// col は、座標系と関連し、また、負値を特定の値として利用したいため、int に仕様変更した
	static inline int ms_pcs_col = 0;

public:
	static void Set_pcs_unit(DWORD pcs_unit);
	static int Get_pcs_col_to_need() { return ms_pcs_col; }

	// 同じ col に属するものが連続して存在する場合は、後続の col を -1 に設定する
	// 表示等を行う場合、-1 を上手く利用すること
	// 戻り値は、col の tmnt アドレス
	static int* Cvt_SEC_to_col(const DWORD* pTop_SEC_src, const DWORD* pTmnt_SEC_src, int* pTop_col_dst);
};


////////////////////////////////////////////////////////////////////////////////////////

class E_View_Ruler_H;
class E_View_Ruler_V;
class E_View_Scrl_H;
class E_View_Scrl_V;
class E_View_Ind_Wnd;

class E_View
{
	friend class AP_E_View;

	friend class E_View_Ruler_H;
	friend class E_View_Ruler_V;

public:
	E_View(DWORD base_price, DWORD initl_MIN_Pr, DWORD initl_MAX_Pr, E_View_Root_Vntfy* pVntfy);
	~E_View();

	// ------------------------------------------------

	// pcs_unit: 何個単位で col を作成するかを指定する
	[[nodiscard]] const CRetMsg* Make_ColAry(DWORD pcs_unit);
	[[nodiscard]] const CRetMsg* OpenWindow();

	void DrawView(int col_start, int col_tmnt) const;

	HWND Get_hWnd() const { return m_hWnd; }
	HDC Get_hDC() const { return m_hDC; }
	HBRUSH Get_hBrush_bkgd() const { return m_hBrush_View_bkgd; }
	const DWORD* Get_pCoeff_Pr_to_pix() const { return &m_coeff_Pr_to_pix; }

	// 戻り値： pix_y でみた場合の、top の移動量（ ＞0 ）
	int Ntfy_MoveUp(DWORD price);  // price > 0 と想定している
	int Ntfy_MoveDown(DWORD price);  // price > 0 と想定している

	// ------------------------------------------------
	// 特殊用途（必要に迫られた場合、メソッド名を変えて、使用先を調査する）
	DWORD Get_pcs_inv_offPr_s___Inv() { return m_pcs_alloc_inv_off_Pr_s; }
	const DWORD* Get_pTop_inv_offPr_s___Inv() { return m_pTop_inv_off_Pr_s; }

	// ------------------------------------------------
	// XIdx 表示のため
	int Get_col_view_left() { return m_col_view_left; }

	// ================================================
	// この３つの値の初期化が、他のオブジェクトの初期化よりも先になるようにすること
	const DWORD mc_BasePrice;
	const DWORD mc_max_Pr_onSystem;
	const DWORD mc_min_Pr_onSystem;

	// ------------------------------------------------
private:
	CRetMsg m_RetMsg;
	const EI_PrAry* m_pE_PrAry = NULL;

	E_View_Ruler_H* const mc_pRuler_H;
	E_View_Ruler_V* const mc_pRuler_V;

	E_View_Scrl_H* const mc_pScrl_H;
	E_View_Scrl_V* const mc_pScrl_V;

	// このポインタを用いて、デイジーチェーンは「組んでいない」
	// 通知を発行するのは、mc_pVntfy_Root に向けて１回のみとなっている
	E_View_Root_Vntfy* const mc_pVntfy_Root;

	E_View_Ind_Wnd* const mc_pInd_Wnd;

	// ------------------------------------------------

	HWND m_hWnd = NULL;
	HDC m_hDC = NULL;
	bool mb_IsCreated = false;
	bool mb_IsOpen = false;  // 現在未使用（Open, Hide の実装前）

	static inline bool msb_IsInit_WndClass = false;

	// ------------------------------------------------

	// max Pr と min Pr は、表示しているものと考えるため、念の為 +1 している
	DWORD m_coeff_Pr_to_pix = 0;

	DWORD m_max_Pr_onView = 0;
	DWORD m_min_Pr_onView = 0;

	int m_pix_y_View_top = 0;
	int m_pix_y_View_btm = 0;

	//+++ 注意 +++ E_View_Scrl_H に変更することを許している
	int m_col_view_left = 0;

	// ------------------------------------------------
	
	// inv_off_Pr_s は「上位 16 bits が max Pr」、「下位 16 bits が min Pr」を表す。
	DWORD* m_pTop_inv_off_Pr_s = NULL;  //「0xFFFFu - off_pr」を記録していることに注意
	DWORD* m_pTmnt_inv_off_Pr_s = NULL;
	DWORD m_pcs_alloc_inv_off_Pr_s = 0;
	DWORD* m_pDataTmnt_inv_off_Pr_s = NULL;

	DWORD* m_pTop_pix_y_s = NULL;
	DWORD* m_pDataTmnt_pix_y_s = NULL;
	int m_pcs_col = 0;  // Make_ColAry() で設定される

	// ------------------------------------------------
	// 1st valid price -> last price -> 1st valid price -> ... の順で idx が格納されている 
	int* m_pTop_col_blk_mark = NULL;
	int* m_pTmnt_col_blk_mark = NULL;
	DWORD m_pcs_alloc_col_blk_mark = 0;
	int* m_pDataTmnt_col_blk_mark = NULL;

	// ------------------------------------------------

	HPEN m_hPen_plot_data = NULL;
	HBRUSH m_hBrush_View_bkgd = NULL;
	HFONT m_hFont = NULL;

	// ================================================
	friend static LRESULT CALLBACK S_WndProc(HWND, UINT, WPARAM, LPARAM);
	void Hndlr_WM_PAINT(HDC hdc, LPPAINTSTRUCT  pps);

	// ------------------------------------------------
	// inv_off_Pr_s、pix_y_s の配列領域確保 ＆ ブロックマークの処理
	void Prep_Arys();
};


////////////////////////////////////////////////////////////////////////////////////////

struct E_View_Pr_CStr
{
	WCHAR ma_pCStr[8] = L"***.***";

	void SetPrice(DWORD price);  // off_Pr でないことに注意
};

// -------------------------------------------------------------------------------------

class E_View_Ruler_H
{
public:
	// １円単位のルーラーを想定
	enum {
		EN_main_H_Ruler_initl_Units = 1'000,
		EN_MAX_pcs_main_H_ruler = 20,

		EN_sub_H_Ruler_initl_Units = 100,
		EN_MAX_pcs_sub_H_rular = 200,

		EN_LABEL_Font_Size = 15,
		EN_LABEL_V_Offset = 7
	};

	E_View_Ruler_H(const E_View* pParent, DWORD base_price)
		: mc_cpParent{ pParent },  mc_BasePrice{ base_price }  {}
	~E_View_Ruler_H() {}

	// ------------------------------------------------
	void InitOnce_after_CreateDC(HDC hDC);
	void Delete_GDIObj();  // WM_DESTROY でコールされる

	// 配列の情報を設定する（縦方向のスクロール、縦方向の倍率変更時）
	void Update_Setting();

	// 横線と、LABEL を描画する
	void Draw_Ruler_H(const int pos_x_start, const int pos_x_tmnt) noexcept;
	// LABEL のみ描画する
	void Draw_Label_only() noexcept;

private:
	// ================================================
	// 最後は -1 で終えること
	int ma_pix_y_main_ruler[EN_MAX_pcs_main_H_ruler + 1];
	E_View_Pr_CStr ma_Pr_CStr_main_ruler[EN_MAX_pcs_main_H_ruler];

	// 最後は -1 で終えること
	int ma_pix_y_sub_rulser[EN_MAX_pcs_sub_H_rular + 1];
	E_View_Pr_CStr ma_Pr_CStr_sub_ruler[EN_MAX_pcs_sub_H_rular];

	// ------------------------------------------------
	const E_View* mc_cpParent;
	const DWORD mc_BasePrice;
	HDC m_hDC = NULL;

	DWORD m_main_H_Ruler_Units = EN_main_H_Ruler_initl_Units;
	DWORD m_sub_H_Ruler_Units = EN_sub_H_Ruler_initl_Units;

	// ------------------------------------------------

	HPEN m_hPen_main_H_ruler = NULL;
	HPEN m_hPen_sub_H_ruler = NULL;
	HFONT m_hFont_rl_H = NULL;
};

////////////////////////////////////////////////////////////////////////////////////////

struct E_Rulr_V_HourInfo_Base
{
	UINT m_year  = 0;
	UINT m_month = 0;
	UINT m_day = 0;
	UINT m_week = 0;

	UINT m_hour = 0;
};

struct E_Rulr_V_HourInfo : public E_Rulr_V_HourInfo_Base
{
	// コンストラクタ（文字列の初期化のみを行う）
	E_Rulr_V_HourInfo();

	void Set_by_Base(const E_Rulr_V_HourInfo_Base& crBase);

	WCHAR ma_SB_date[CE_StrBuf(L"yy/mm/dd w")];
	WCHAR ma_SB_hour[CE_StrBuf(L"**:**")];
};

// -------------------------------------------------------------------------------------

class E_View_Ruler_V
{
public:
	// V Ruler は、分単位で指定
	enum {
//		EN_V_Ruler_initl_Units = 60,
		EN_MAX_pcs_HOUR_ruler = 121,  // 5日 x 24時間 + 1

		EN_LABEL_Font_Size = 15,
		EN_LABEL_H_Offset = 7
	};

	E_View_Ruler_V(E_View* pParent) : mc_cpParent{ pParent }  {}
	~E_View_Ruler_V() {}

	// ------------------------------------------------
	void InitOnce_after_CreateDC(HDC hDC);
	void Delete_GDIObj();  // WM_DESTROY でコールされる

	// 配列の情報を設定する（pcs_unit の変更時）
	[[nodiscard]] const CRetMsg* Update_Setting();

	void Draw_Ruler_V(int pos_x_start, int pos_x_tmnt) noexcept;

private:
	// ================================================
	DWORD ma_idx_HOUR_rl_V[EN_MAX_pcs_HOUR_ruler];
	DWORD* m_pDataTmnt_idx_HOUR_rl_V = NULL;

	int ma_col_HOUR_rl_V[EN_MAX_pcs_HOUR_ruler];
	int* m_pDataTmnt_col_HOUR_rl_V = NULL;
	E_Rulr_V_HourInfo ma_HOUR_info_rl_V[EN_MAX_pcs_HOUR_ruler];

	// ------------------------------------------------
	CRetMsg m_RetMsg;
	const E_View* const mc_cpParent;

	HDC m_hDC = NULL;

	// ------------------------------------------------
	HPEN m_hPen_blk_mark_rl_V = NULL;
	HPEN m_hPen_HOUR_rl_V = NULL;
	HFONT m_hFont_rl_V = NULL;

	// ================================================
	// s_hour_info_base にデータを設定する
	[[nodiscard]] const CRetMsg* Set_to_s_HourInfo(DWORD idx_PrAry, const E_YMD* pEYMD_PrAry);
};


////////////////////////////////////////////////////////////////////////////////////////

class E_View_Scrl_H
{
public:
	enum {
		PIXS_MOVE_LINE_H = 60,
		PIXS_MOVE_PAGE_H = 600
	};

	// ------------------------------------------------
	// コンストラクタ
	E_View_Scrl_H(E_View* pParent, int* pParent_col_view_left, E_View_Ruler_H* pRuler_H, E_View_Ruler_V* pRuler_V);
	~E_View_Scrl_H() {}

	// m_hWnd が設定された後で Init される（親の OpenWindow() からコールされる）
	void InitOnce_after_CreateDC(HWND hWnd, HDC hDC) {
		m_hWnd = hWnd;
		m_hDC = hDC;
		m_hBrush_bkgd = mc_cpParet->Get_hBrush_bkgd();
	}
	// col数が定まった後に、H 方向のスクロールバーの調整ができる
	void On_InitCol(int pcs_col);

	// S_WndProc() からコールされる
	void Hndlr_WM_HSCROLL(WPARAM wp);

	void Move_Left(int dx);  // dx > 0 であること
	void Move_Right(int dx);  // dx > 0 であること

	// ------------------------------------------------
private:
	const E_View* const mc_cpParet;
	E_View_Ruler_H* const mc_pRuler_H;
	E_View_Ruler_V* const mc_pRuler_V;

	// ------------------------------------------------
	HWND m_hWnd = NULL;	// ScrollWindowEx() で利用する
	HDC m_hDC = NULL;
	HBRUSH m_hBrush_bkgd = NULL;

	int* const mc_pParent_col_view_left;

	// スクロールができないときは 0 に設定する（On_InitCol() で設定される）
	int m_col_view_left_MAX = 0;

	SCROLLINFO m_sci_H;
};


////////////////////////////////////////////////////////////////////////////////////////

class E_View_Scrl_V
{
public:
	enum {
		PIXS_MOVE_LINE_V = 100,
		PIXS_MOVE_PAGE_V = 500
	};

	// ------------------------------------------------
	// コンストラクタ
	E_View_Scrl_V(E_View* pParent, E_View_Ruler_H* pRuler_H, E_View_Ruler_V* pRuler_V
				, const DWORD* pPr_max_onView, const DWORD* pPr_min_onView);

	// m_hWnd が設定された後で Init される（親の OpenWindow() からコールされる）
	void InitOnce_after_CreateDC(HWND hWnd, HDC hDC);

	// S_WndProc() からコールされる
	void Hndlr_WM_VSCROLL(WPARAM wp);

	void Move_Up(int dx);  // dx > 0 であること
	void Move_Down(int dx);  // dx > 0 であること

	// ------------------------------------------------
private:
	E_View* const mc_pParet;
	E_View_Ruler_H* const mc_pRuler_H = NULL;
	E_View_Ruler_V* const mc_pRuler_V = NULL;

	const DWORD* const mc_cpPr_max_on_view;
	const DWORD* const mc_cpPr_min_on_view;

	// ------------------------------------------------
	HWND m_hWnd = NULL;	// ScrollWindowEx() で利用する
	HDC m_hDC = NULL;
	HBRUSH m_hBrush_bkgd = NULL;

	DWORD m_max_Pr_onView_TOP = 0;
	DWORD m_min_Pr_onView_TOP = 0;

	SCROLLINFO m_sci_V;
};


////////////////////////////////////////////////////////////////////////////////////////

class E_View_Ind_Wnd
{
public:
	enum : COLORREF { EN_color_bkgd = RGB(1, 0, 0) };

	[[nodiscard]] const CRetMsg* Open_Ind_Wnd(const E_View_pos& crView_pos, HWND hWnd_Prnt);
	// システムリソースの解放を実行
	void Release_Rsc();

	void On_WM_MOVE___E_View();
	void On_MOUSEMOVE___E_View(int pos_x_onIndWnd);

	static void On_Chg_col_view_left(int new_col_view_left);

private:
	static CRetMsg ms_RetMsg;
	HWND m_hWnd_Prnt = NULL;

	HWND m_hWnd = NULL;
	HDC m_hDC = NULL;

	POINT m_LeftTop_onEView;  // 最初に一度だけ初期化して、その後は変更なし
	int m_Width = 0;
	int m_Height = 0;

	// ------------------------------------------------
	int m_X_cur_Ind = 0;

	HPEN m_hPen_Ind = NULL;
	HPEN m_hPen_Erase = NULL;

	// ------------------------------------------------
	[[nodiscard]] static const CRetMsg* S_Rgst_Ind_WndClass();
	static LRESULT CALLBACK S_Ind_WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	void On_WM_PAINT();

	///////////////////////////////////////////////////
	enum { EN_width_XIdx = 50,  EN_height_XIdx = 18,  EN_offset_XIdx = EN_width_XIdx / 2 };

	[[nodiscard]] static const CRetMsg* S_Rgst_XIdx_WndClass();
	static LRESULT CALLBACK S_XIdx_WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	static void On_WM_PAINT_XIdx_Wnd();

	HWND m_hWnd_XIdx = NULL;
	inline static HDC ms_hDC_XIdx = NULL;

	int m_Left_XIdx_byScrn;
	int m_Top_XIdx_byScrn;

	inline static DWORD ms_col_view_left = 0;
	inline static DWORD ms_pos_x_onIndWnd = 0;
	inline static WCHAR msa_SB_XIdx[10] = L"abcdefghi";
};
