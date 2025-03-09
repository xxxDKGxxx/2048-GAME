#pragma once

#include <windows.h>
#include <string>
#include "board.h"
#include <chrono>
#include <map>
#include "resource.h"
#include "resource1.h"

class app_2048
{
private:
	bool register_class();
	static std::wstring const s_class_name;
	static constexpr UINT_PTR s_timer = 1;
	static constexpr UINT_PTR s_anim_timer = 2;
	std::chrono::time_point<std::chrono::system_clock> m_start_time;
	static LRESULT CALLBACK window_proc_static(
		HWND window, UINT message, WPARAM wparam, LPARAM lparam
	);
	LRESULT window_proc(
		HWND window, UINT message, WPARAM wparam, LPARAM lparam);
	HWND create_window(DWORD style, HWND parent = nullptr, DWORD ex_style = 0);
	void on_window_move(HWND window, LPWINDOWPOS params);
	void update_transparency();
	void on_timer();
	void on_command(HWND window, WORD cmdID);
	void on_paint(HWND window);
	void on_menu_item_check(HMENU menu, WORD cmdID);
	void InvalidateAllWins(bool bkg = false);
	void on_key_pressed(WPARAM key);
	void on_losing();
	void on_winning();
	void draw_when_winning(HWND window, HDC dc);
	void draw_when_losing(HWND window, HDC dc);
	void save_game_state();
	void load_game_state();
	void on_animation();
	HINSTANCE m_instance;
	HWND m_main, m_popup;
	board m_board;
	HBRUSH m_field_brush;
	HBRUSH m_bkg_brush;
	HBRUSH m_green_brush;
	HBRUSH m_red_brush;
	HBITMAP m_bkg_bitmap;
	HBITMAP m_endgame_bitmap;
	POINT m_screen_size;
	int goal, score;
	bool game_finished;
	int game_result;
	RECT window_size { 0, 0, board::width, board::height + board::field_size + board::margin };
	std::map<int, int> goal_dict
	{
		{8, ID_GOAL_8},
		{16, ID_GOAL_16},
		{64, ID_GOAL_64},
		{2048, ID_GOAL_2048}
	};
	std::map<int, COLORREF> tile_colors_dict
	{
		{2, RGB(238, 228, 198)},
		{4, RGB(239, 225, 218)},
		{8, RGB(243, 179, 124)},
		{16, RGB(246, 153, 100)},
		{32, RGB(246, 125, 98)},
		{64, RGB(247, 93, 60)},
		{128, RGB(237, 206, 116)},
		{256, RGB(239, 204, 98)},
		{512, RGB(243, 201, 85)},
		{1024, RGB(238, 200, 72)},
		{2048, RGB(239, 192, 47)}
	};
public:
	app_2048(HINSTANCE instance);
	int run(int show_command);
	~app_2048();
};