#pragma once

#include "CRetMsg.h"

////////////////////////////////////////////////////////////////////////////////////////

//struct { int m_Left; int m_Top; } static constexpr se_OL_Wnd_pos = { -1500, 50 };
//struct { int m_Left; int m_Top; } static inline constexpr se_F_OL_Wnd_pos = { 100, 50 };

struct F_View_OL_Wnd_pos { int m_Left; int m_Top; };

struct F_View_mgn { int m_Left; int m_Top; int m_Right; int m_Btm; };
struct F_View_padding { int m_Left; int m_Top; int m_Right; int m_Btm; };
struct F_View_ViewSize { int m_Width; int m_Height; };


////////////////////////////////////////////////////////////////////////////////////////

// F_View の子ウィンドウが通知を受け取るためのクラス
// mgn, padding, view_size を設定した上で、親 F_View にアタッチする形式を想定している

class F_View;

struct Vntfy_F_View
{
	Vntfy_F_View(const F_View_mgn& mgn, const F_View_padding& padding, const F_View_ViewSize& view_size)
		: m_mgn{ mgn }, m_padding{ padding }, m_view_size{ view_size } {}
	~Vntfy_F_View() {}

	// ------------------------------------------------
	F_View* m_pPrnt_F_View = NULL;
	Vntfy_F_View* m_pNext = NULL;

	HWND m_hWnd = NULL;
	HDC m_hDC = NULL;

	F_View_mgn m_mgn;
	F_View_padding m_padding;
	F_View_ViewSize m_view_size;

	// ------------------------------------------------
	// 親 F_View での位置決めだけに利用される情報
	class ChildPos {
		friend class F_View;
		int m_idx_col;
		int m_idx_row;

		int m_len_col;
		int m_len_row;

		int m_width_rem ;
		int m_height_rem;

		int m_Left;
		int m_Top;
	} m_pos_on_F_View;
	
	// ================================================
	virtual void Vntfy_Crt_Window() =0;
	virtual void Vntfy_Delete_GDIObj() =0;
	virtual void Vntfy_Draw() =0;
};


// -------------------------------------------------------------------------------------
// 子ウィンドウの WndProc を「持たない」ものに対する通知クラス

struct Vntfy_F_View_Parts
{
	Vntfy_F_View_Parts(const F_View_ViewSize& view_size)
		: mc_view_size{ view_size } {}
	~Vntfy_F_View_Parts() {}

	// ------------------------------------------------
	Vntfy_F_View* m_pNext = NULL;  // 将来的にチェーンとして利用することを想定

	HWND m_hWnd = NULL;
	HDC m_hDC = NULL;

	const F_View_ViewSize mc_view_size;

	virtual void Vntfy_Crt_Window() =0;
	virtual void Vntfy_Delete_GDIObj() =0;
	virtual void Vntfy_Draw() =0;
};


// -------------------------------------------------------------------------------------

class F_View
{
public:
	F_View(const F_View_OL_Wnd_pos& wnd_pos) : m_OL_Wnd_pos{ wnd_pos } {}
	~F_View() {}

	// ------------------------------------------------
	[[nodiscard]] const CRetMsg* OpenWindow();

	// 0 <= idx_col < EN_col_child_view , 0 <= idx_row < EN_row_child_view
	void Attach_Child_View(Vntfy_F_View* pChild, DWORD idx_col, DWORD idx_row, DWORD len_col = 1, DWORD len_row = 1);
	void Set_Child_View_pos();

	static HFONT S_Get_DefFont() { return ms_hFont; }

	// ------------------------------------------------
private:
	static CRetMsg ms_RetMsg;
	bool mb_IsCreated = false;

	Vntfy_F_View* m_pView_Child_top = NULL;
	Vntfy_F_View* m_pView_Child_last = NULL;

	// ------------------------------------------------
	F_View_OL_Wnd_pos m_OL_Wnd_pos;
	HWND m_hWnd_Prnt = NULL;
	HDC m_hDC_Prnt = NULL;

	static inline HFONT ms_hFont = NULL;

	// ------------------------------------------------
	// 未実装ではあるが、ms_pVntfy のチェーンにそって作動する関数群
	[[nodiscard]] const CRetMsg* Open_Child_Windows();

	// ================================================
	[[nodiscard]] static const CRetMsg* S_Rgst_Parent_WndClass();
	static LRESULT CALLBACK S_Parent_WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	[[nodiscard]] static const CRetMsg* S_Rgst_Child_WndClass();
	static LRESULT CALLBACK S_Child_WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	void Delete_GDIObj();

	// ------------------------------------------------
	// 子 View の位置決めにのみ利用されるもの
	enum { EN_col_child_view = 5,  EN_row_child_view = 5 };
	Vntfy_F_View* ma_child_view_mtx[ EN_col_child_view * EN_row_child_view ] ={};

	int m_width_Clt;
	int m_Height_Clt;
};
