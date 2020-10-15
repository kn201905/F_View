#pragma once

#include "_F_View.h"

////////////////////////////////////////////////////////////////////////////////////////

struct F_View_HM_SRC
{
	const int* m_pVals_src = NULL;

	DWORD m_pcs_x;
	DWORD m_pcs_y;

	int m_x_Axis_idx;  // Axis が不要な場合は、負値を設定
	int m_y_Axis_idx;  // Axis が不要な場合は、負値を設定

	int m_x_Aux_intvl_pcs;  // Axis が不要な場合は、負値を設定
	int m_y_Aux_intvl_pcs;  // Axis が不要な場合は、負値を設定
};

////////////////////////////////////////////////////////////////////////////////////////

class F_View_Heat_map : public Vntfy_F_View
{
	enum : COLORREF { EN_COLOR_Axis = RGB(220, 220, 220),  EN_COLOR_Normal = RGB(255, 255, 255) };
	enum { EN_x_width = 22, EN_y_height = 12 };

	static inline constexpr F_View_mgn me_mgn_dflt = { 3, 3, 3, 3 };
	static inline constexpr F_View_padding me_padding_dflt = { 1, 1, 1, 1 };
	// ViewSize は、Set_SRC() がコールされたときに再設定される
	static inline constexpr F_View_ViewSize me_view_size_dflt = { 100, 100 };

	// ----------------------------------------------------
public:
	F_View_Heat_map() : Vntfy_F_View{ me_mgn_dflt, me_padding_dflt, me_view_size_dflt } {}
	~F_View_Heat_map();

	// ----------------------------------------------------
	void Set_SRC(const F_View_HM_SRC& crSRC);

	void DBG_Set_SRC();

private:
	void Texitize_vals_src();
	void Crt_BkColor();
	void Draw_Axis();

	// ====================================================
	// Vntfy_F_View
	virtual void Vntfy_Crt_Window() override;
	virtual void Vntfy_Delete_GDIObj() override;
	virtual void Vntfy_Draw() override;

	// ====================================================
	DWORD m_pcs_x = 0;
	DWORD m_pcs_y = 0;

	const int* m_pVals_src = NULL;  // 値によって、文字の色を変えることを考えているため、データ値へのポインタを保持している
	const WCHAR* m_pVals_textized = NULL;
	const COLORREF* m_pBkColor = NULL;

	// ----------------------------------------------------
	int m_x_Axis_idx;  // 主軸が必要な場合は、負値を設定する
	int m_y_Axis_idx;  // 主軸が必要な場合は、負値を設定する
	RECT m_rc_x_axis;
	RECT m_rc_y_axis;

	// 補助線を引く場合には、主軸が必ず必要になる（主軸をスタートとして補助線を引いていくため）
	// pcs_Aux == 0 であるときは、補助線は描画されない
	WORD m_pcs_x_Aux = 0;
	WORD m_pcs_y_Aux = 0;
	const int* m_pPos_y__x_Aux = NULL;  // y 座標の配列
	const int* m_pPos_x__y_Aux = NULL;  // x 座標の配列

	// ----------------------------------------------------
	HFONT m_hFont = NULL;
	HBRUSH m_hBrush_Axis = NULL;
	HPEN m_hPen_Axis_Aux = NULL;
};
