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

// コンストラクタ（文字列の初期化のみを行う）

E_Rulr_V_HourInfo::E_Rulr_V_HourInfo()
{
	*(UINT64*)(ma_SB_date + 7) = KSTR4(L'0', L' ', L'w', 0);

	*(UINT64*)ma_SB_hour = K4WStrToUINT64(L"00:0");
	*(DWORD*)(ma_SB_hour + 4) = KSTR2(L'0', 0);
}

// -------------------------------------------------------------------------------------

static WCHAR sa_week_wchr[] = L"月火水木金土日";

void E_Rulr_V_HourInfo::Set_by_Base(const E_Rulr_V_HourInfo_Base& crBase)
{
	m_year = crBase.m_year;
	m_month = crBase.m_month;
	m_day = crBase.m_day;
	m_week = crBase.m_week;

	m_hour = crBase.m_hour;

	{
		const UINT64 year = m_year - 2000;
		UINT64 ui64str_1, ui64str_2;
		if (year < 20)
		{
			ui64str_1 = K4WStrToUINT64(L"10/ ") + ((year % 10) << 16);
		}
		else
		{
			ui64str_1 = K4WStrToUINT64(L"20/ ") + ((year % 10) << 16);
		}

		if (m_month < 10)
		{
			ui64str_2 = K4WStrToUINT64(L"0/ 0") + m_month;
		}
		else
		{
			ui64str_1 += UINT64(0x31 - 0x20) << 48;
			ui64str_2 = K4WStrToUINT64(L"0/ 0") + (m_month % 10);
		}

		if (m_day < 10)
		{
			ui64str_2 += UINT64(m_day) << 48;
		}
		else
		{
			ui64str_2 += (UINT64((m_day / 10) + (0x31 - 0x20)) << 32) +(UINT64(m_day % 10) << 48);
		}

		*(UINT64*)ma_SB_date = ui64str_1;
		*(UINT64*)(ma_SB_date + 4) = ui64str_2;

		*(ma_SB_date + 9) = sa_week_wchr[m_week];
	}

	if (m_hour < 10)
	{
		*(DWORD*)ma_SB_hour = (m_hour << 16) + 0x0030'0020u;
	}
	else
	{
		if (m_hour < 20)
		{
			*(DWORD*)ma_SB_hour = ((m_hour - 10) << 16) + 0x0030'0031u;
		}
		else 
		{
			*(DWORD*)ma_SB_hour = ((m_hour - 20) << 16) + 0x0030'0032u;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////

void E_View_Ruler_V::Delete_GDIObj()
{
	if (m_hPen_blk_mark_rl_V) { DeleteObject(m_hPen_blk_mark_rl_V); }
	if (m_hPen_HOUR_rl_V) { DeleteObject(m_hPen_HOUR_rl_V); }

	if (m_hFont_rl_V) { DeleteObject(m_hFont_rl_V); }
}

// -------------------------------------------------------------------------------------

void E_View_Ruler_V::InitOnce_after_CreateDC(HDC hDC)
{
	m_hDC = hDC;

	m_hPen_blk_mark_rl_V = CreatePen(PS_DOT, 1, __E::EN_COLOR_blk_nark_rl_V);
	m_hPen_HOUR_rl_V = CreatePen(PS_SOLID, 1, __E::EN_COLOR_HOUR_rl_V);

	m_hFont_rl_V = CreateFont(
		EN_LABEL_Font_Size,		// nHeight
		0,		// nWidth
		900,	// nEscapement
		0,		// nOrientation
		FW_NORMAL,				// fnWeight
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

struct E_Rulr_V_HourInfo_Factry : public E_Rulr_V_HourInfo_Base
{
	// --------------------------------------------------------
	[[nodiscard]] const CRetMsg* Set_Ref(DWORD idx, const E_YMD* pEYMD)
	{
		m_ref_idx = idx;

		m_year = pEYMD->Get_Year();
		m_month = pEYMD->Get_Month();
		m_day = pEYMD->Get_Day();

		m_hour = pEYMD->Get_Hour();

		m_week = pEYMD->Get_Week();
		m_days_in_month = pEYMD->Get_Days_inMonth();

		m_minu_add = [&]() -> DWORD {
			UINT minu = pEYMD->Get_Minute();
			return minu == 0 ? 0 : 60 - minu;
		}();

		if (m_minu_add == 0)
		{
			m_idx = m_ref_idx;
			m_minu_frm_2017 = pEYMD->m_minu_frm_2017;
			return NULL;
		}
		else
		{
			m_idx = m_ref_idx + ((m_minu_add * 60'000 + 512) >> 10);
			m_minu_frm_2017 = pEYMD->m_minu_frm_2017 + m_minu_add;
			return this->Set_to_next_hour();
		}
	}

	// --------------------------------------------------------
	[[nodiscard]] const CRetMsg* Adv_next_hour()
	{
		m_minu_frm_2017 += 60;
		m_minu_add += 60;
		m_idx = m_ref_idx + ((m_minu_add * 60'000 + 512) >> 10);

		return this->Set_to_next_hour();
	}

	// --------------------------------------------------------
	DWORD Get_minu_frm_2017() const { return m_minu_frm_2017; }
	// 仕様上、hh:00:00 から hh:00:59 までの間の idx が返されることになる
	DWORD Get_idx() const { return m_idx; }

private:
	CRetMsg m_RetMsg;

	// Set_Ref で受け取ったもの。Set_Ref で受け取った pEYMD の時刻が hh時 mm 分であったとすると、
	// m_ref_idx は hh:mm:00 から hh:mm:59 までの間を指すことになる
	DWORD m_ref_idx = 0;

	DWORD m_minu_add = 0;
	DWORD m_minu_frm_2017 = 0;
	DWORD m_idx = 0;

	UINT m_days_in_month = 0;

	// --------------------------------------------------------
	[[nodiscard]] const CRetMsg* Set_to_next_hour()
	{
		m_hour++;
		if (m_hour == 24)
		{
			m_hour = 0;

			m_week++;
			if (m_week == N_Date::EN_Sun)
			{ m_RetMsg.CSET_MSG(L"week == N_Date::EN_Sun");  return &m_RetMsg; }

			m_day++;
			if (m_day > m_days_in_month)
			{
				m_day = 1;
				
				m_month++;
				if (m_month == 13)
				{
					m_month = 1;

					m_year++;
					if (m_year > MAX_YEAR)
					{ m_RetMsg.CSET_MSG(L"year > MAX_YEAR");  return &m_RetMsg; }
				}
			}
		}
		return NULL;
	}
}
static s_E_Rulr_V_HourInfo_Factry;


// -------------------------------------------------------------------------------------

// 配列の情報を設定する（pcs_unit の変更時）
const CRetMsg* E_View_Ruler_V::Update_Setting()
{
	enum : DWORD {
		EN_minu__EYMD_Tmnt_mark = 0xFFFF'FFFFu
	};

	const EI_PrAry* const cpE_PrAry = E_View_Mdt::S_GetBody_E_PrAry();

	// idx 情報
	const DWORD* p_idx__YMD_src = cpE_PrAry->Get_pTop_YMD_idx();
	const DWORD* const pDataTmnt_idx__YMD_src = cpE_PrAry->Get_pDataTmnt_YMD_idx();
	// idx に対応する EYMD
	const E_YMD* p_EYMD_src = cpE_PrAry->Get_pTop_EYMD();

	// idx の tmnt 値
	const DWORD cidx_tmnt = cpE_PrAry->Get_pcs_off_Pr();

	// ファクトリの初期値設定
	const CRetMsg* pret_msg;
	pret_msg = s_E_Rulr_V_HourInfo_Factry.Set_Ref(*p_idx__YMD_src++, p_EYMD_src++);
	if (pret_msg) { return pret_msg; }

	DWORD idx_next_Draw = s_E_Rulr_V_HourInfo_Factry.Get_idx();
	DWORD minu_frm_2017_next_Draw = s_E_Rulr_V_HourInfo_Factry.Get_minu_frm_2017();

	DWORD minu_frm_2017_next_EYMD;
	if (p_idx__YMD_src == pDataTmnt_idx__YMD_src)  // YMD 情報が１つしかない場合もありうる
	{
		minu_frm_2017_next_EYMD = 0xFFFF'FFFFu;
	}
	else
	{
		minu_frm_2017_next_EYMD = p_EYMD_src->m_minu_frm_2017;
	}

	// destination の設定
	DWORD* p_idx_Hour_dst = ma_idx_HOUR_rl_V;
	E_Rulr_V_HourInfo* p_HourInfo_dst = ma_HOUR_info_rl_V;

	// idx の destination のオーバーフローチェック用（１周間の解析なら、オーバーフローはしないはずだが）
	DWORD cnt_idx_Hour_dst = EN_MAX_pcs_HOUR_ruler;


	while (true)
	{
		// minu_frm_2017_next_Draw より、手前の時間を指す EYMD があれば、情報をそちらに更新する
		while (true)
		{
			if (minu_frm_2017_next_Draw < minu_frm_2017_next_EYMD) { break; }
			
			// minu_frm_2017_next_EYMD <= minu_frm_2017_next_Draw のときの処理
			// 新しいリファレンスを設定
			pret_msg = s_E_Rulr_V_HourInfo_Factry.Set_Ref(*p_idx__YMD_src++, p_EYMD_src++);
			if (pret_msg) { return pret_msg; }

			const DWORD new_idx_next_Draw = s_E_Rulr_V_HourInfo_Factry.Get_idx();
			const DWORD new_minu_frm_2017_next_Draw = s_E_Rulr_V_HourInfo_Factry.Get_minu_frm_2017();

			// エラー顕在化チェック
			// 仕様の上では、 KAbs(new_idx_next_Draw, idx_next_Draw) < 60 となる
			if (KAbs(new_idx_next_Draw, idx_next_Draw) > 70)
			{ m_RetMsg.CSET_MSG(L"KAbs(new_idx_next_Draw, idx_next_Draw) > 70");  return &m_RetMsg; }

			if (new_minu_frm_2017_next_Draw != minu_frm_2017_next_Draw)
			{ m_RetMsg.CSET_MSG(L"new_minu_frm_2017_next_Draw != minu_frm_2017_next_Draw");  return &m_RetMsg; }

			idx_next_Draw = new_idx_next_Draw;
			minu_frm_2017_next_Draw = new_minu_frm_2017_next_Draw;

			// まだ EYND の要素が残っているかどうかをチェック
			if (p_idx__YMD_src == pDataTmnt_idx__YMD_src)
			{
				minu_frm_2017_next_EYMD = EN_minu__EYMD_Tmnt_mark;
			}
			else
			{
				minu_frm_2017_next_EYMD = p_EYMD_src->m_minu_frm_2017;
			}
		}

		// minu_frm_2017_next_Draw に対する idx 値が定まった
		if (cidx_tmnt <= idx_next_Draw) { break; }  // Hour ルーラーの idx 処理は終了
		
		// デスティネーションに結果をストア
		*p_idx_Hour_dst++ = idx_next_Draw;
		p_HourInfo_dst->Set_by_Base(s_E_Rulr_V_HourInfo_Factry);
		p_HourInfo_dst++;

		if (--cnt_idx_Hour_dst == 0 ) { break; }

		// 次の Hour の情報へ進む
		pret_msg = s_E_Rulr_V_HourInfo_Factry.Adv_next_hour();
		if (pret_msg) { return pret_msg; }

		idx_next_Draw = s_E_Rulr_V_HourInfo_Factry.Get_idx();
		minu_frm_2017_next_Draw = s_E_Rulr_V_HourInfo_Factry.Get_minu_frm_2017();
	}

	m_pDataTmnt_idx_HOUR_rl_V = p_idx_Hour_dst;
	if (ma_idx_HOUR_rl_V == p_idx_Hour_dst)  // HOUR 情報が１つもなかった場合の処理
	{
		m_pDataTmnt_col_HOUR_rl_V = ma_col_HOUR_rl_V;
		return NULL;
	}

	// HOUR に対する idx が１つ以上あった場合、col も算出しておく
	m_pDataTmnt_col_HOUR_rl_V = E_idx_to_col::Cvt_SEC_to_col(ma_idx_HOUR_rl_V, m_pDataTmnt_idx_HOUR_rl_V, ma_col_HOUR_rl_V);

	return NULL;
}


// -------------------------------------------------------------------------------------

void E_View_Ruler_V::Draw_Ruler_V(const int pos_x_start, const int pos_x_tmnt) noexcept
{
	const auto L_Draw_LABEL = [&](int pos_x, E_Rulr_V_HourInfo* pHour_info) -> void {
		if (pHour_info->m_hour ==0)
		{
			TextOut(m_hDC, pos_x - EN_LABEL_H_Offset, se_View_size.m_Height + 45, pHour_info->ma_SB_date, 8);
			TextOut(m_hDC, pos_x + EN_LABEL_H_Offset, se_View_size.m_Height + 15, pHour_info->ma_SB_date + 9, 1);
		}
		else if (pHour_info->m_hour < 10)
		{
			TextOut(m_hDC, pos_x - EN_LABEL_H_Offset, se_View_size.m_Height + 27, pHour_info->ma_SB_hour + 1, 4);
		}
		else
		{
			TextOut(m_hDC, pos_x - EN_LABEL_H_Offset, se_View_size.m_Height + 33, pHour_info->ma_SB_hour, 5);
		}
	};

	// =================================================

	// 時刻文字等の描画を考えて、描画範囲を半文字分k拡大する
//	pos_x_start = pos_x_start <= 7 ? 0 : pos_x_start - 7;
//	pos_x_tmnt = pos_x_tmnt >= se_View_size.m_Width - 7 ? se_View_size.m_Width : pos_x_tmnt + 7;
	
	const int col_view_left = mc_cpParent->m_col_view_left;
	const int col_start_draw = col_view_left + pos_x_start;
	const int col_tmnt_draw = col_view_left + pos_x_tmnt;

	SelectObject(m_hDC, m_hFont_rl_V);

	// -------------------------------------------------
	// HOUR の描画
	{
		int* p_col_HOUR = ma_col_HOUR_rl_V;
		const int* const pDataTmnt_col_HOUR_rl_V = m_pDataTmnt_col_HOUR_rl_V;  // 最適化考慮

		int col;
		// ラベルの文字の右側の一部が消えている可能性が高いため、start 位置より１つ前の目盛りの描画を、必要があれば行う
		int col_start_draw_extend = col_start_draw - 25;

		// col_start_draw_extend は、表示領域に入っている必要がある
		if (col_start_draw_extend < col_view_left) { col_start_draw_extend = col_view_left; }

		while (true)
		{
			if (p_col_HOUR == pDataTmnt_col_HOUR_rl_V) { goto FINISH_HOUR_MARK; }
			
			col = *p_col_HOUR++;
			if (col >= col_start_draw_extend) { break; }  // 表示すべき col が見つかった
		}

		// ラベルの文字の右側の一部が消えている可能性が高いため、ラベルのみ１つ余分に書き足しておく
		if (col < col_start_draw)
		{
			L_Draw_LABEL(col - col_view_left, ma_HOUR_info_rl_V + (p_col_HOUR - ma_col_HOUR_rl_V - 1));

			if (p_col_HOUR == pDataTmnt_col_HOUR_rl_V) { goto FINISH_HOUR_MARK; }
			col = *p_col_HOUR++;
		}
		// col >= col_start_draw

		SelectObject(m_hDC, m_hPen_HOUR_rl_V);

		E_Rulr_V_HourInfo* p_HOUR_info = ma_HOUR_info_rl_V + (p_col_HOUR - ma_col_HOUR_rl_V - 1);

		// 描画開始
		while (true)
		{
			if (col >= col_tmnt_draw) { break; }

			// HOUR マークの処理では、col < 0 のものはないはずだけど。
			if (col >= 0)
			{
				// 罫線表示
				const int pos_x = col - col_view_left;
				MoveToEx(m_hDC, pos_x, 0, NULL);
				LineTo(m_hDC, pos_x, se_View_size.m_Height);

				// ラベル表示
				L_Draw_LABEL(pos_x, p_HOUR_info);
			}
			p_HOUR_info++;

			if (p_col_HOUR == pDataTmnt_col_HOUR_rl_V) { goto FINISH_HOUR_MARK; }
			col = *p_col_HOUR++;
		}

		// ラベルの文字の左側の一部が消えている可能性が高いため、ラベルのみ１つ余分に書き足しておく
		int col_tmnt_draw_extend = col_tmnt_draw + 8;

		// col_tmnt_draw_extend が表示領域に入っている必要がある
		if (col_tmnt_draw_extend > col_view_left + se_View_size.m_Width)
		{ col_tmnt_draw_extend = col_view_left + se_View_size.m_Width; }

		if (col < col_tmnt_draw_extend)
		{
			L_Draw_LABEL(col - col_view_left, ma_HOUR_info_rl_V + (p_col_HOUR - ma_col_HOUR_rl_V - 1));
		}
	}
FINISH_HOUR_MARK:

	// -------------------------------------------------
	// ブロックマークの描画
	{
		const int* p_col_blk_mark = mc_cpParent->m_pTop_col_blk_mark;
		if (p_col_blk_mark == NULL) { return; }		// データがロードされていない場合
		const int* const pDataTmnt_col_blk_mark = mc_cpParent->m_pDataTmnt_col_blk_mark;

		int col;
		while (true)
		{
			col = *p_col_blk_mark++;
			if (col >= col_start_draw) { break; }  // 表示すべき col が見つかった

			if (p_col_blk_mark == pDataTmnt_col_blk_mark) { goto FINISH_BLK_MARK; }
		}

		SelectObject(m_hDC, m_hPen_blk_mark_rl_V);

		// 描画開始
		while (true)
		{
			if (col >= col_tmnt_draw) { break; }

			// ブロックマークの処理では、col < 0 のものは処理しない（重複処理）
			if (col >= 0)
			{
				const int pos_x = col - col_view_left;
				MoveToEx(m_hDC, pos_x, 0, NULL);
				LineTo(m_hDC, pos_x, se_View_size.m_Height);
			}

			if (p_col_blk_mark == pDataTmnt_col_blk_mark) { break; }
			col = *p_col_blk_mark++;
		}
	}
FINISH_BLK_MARK:

	return;
}
