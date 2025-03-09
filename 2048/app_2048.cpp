#pragma comment (lib, "Dwmapi")
#pragma comment(lib, "Msimg32.lib")
#include "app_2048.h"
#include <stdexcept>
#include <dwmapi.h>
#include <wingdi.h>
#include "resource1.h"
#include "resource.h"


std::wstring const app_2048::s_class_name{ L"2048 Window" };

bool app_2048::register_class()
{
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(m_instance, s_class_name.c_str(), &desc) != 0) return true;
	desc = {
		.cbSize = sizeof(WNDCLASSEXW),
		.lpfnWndProc = window_proc_static,
		.hInstance = m_instance,
		.hIcon = static_cast<HICON>(
			LoadImageW(
				m_instance,
				MAKEINTRESOURCE(ID_APPICON),
				IMAGE_ICON,
				0, 0,
				LR_SHARED | LR_DEFAULTSIZE
				)
),
		.hCursor = LoadCursorW(nullptr, L"IDC_ARROW"),
		.hbrBackground = CreateSolidBrush(RGB(250, 247, 238)),
		.lpszMenuName = MAKEINTRESOURCE(ID_MAINMENU),
		.lpszClassName = s_class_name.c_str()
	};
	return RegisterClassExW(&desc) != 0;
}

LRESULT app_2048::window_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	app_2048* app = nullptr;
	if (message == WM_NCCREATE)
	{
		auto p = reinterpret_cast<LPCREATESTRUCTW>(lparam);
		app = static_cast<app_2048*>(p->lpCreateParams);
		SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
	}
	else
	{
		app = reinterpret_cast<app_2048*>(
			GetWindowLongPtr(window, GWLP_USERDATA)
			);
	}
	if (app != nullptr)
	{
		return app->window_proc(window, message, wparam, lparam);
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT app_2048::window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_CLOSE:
		save_game_state();
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		if (window == m_main)
		{
			PostQuitMessage(EXIT_SUCCESS);
			return 0;
		}
		break;
	/*case WM_CTLCOLORSTATIC:
		return reinterpret_cast<INT_PTR>(m_field_brush);*/
	case WM_WINDOWPOSCHANGED:
		on_window_move(window, reinterpret_cast<LPWINDOWPOS>(lparam));
		return 0;
	case WM_TIMER:
		if (wparam == s_anim_timer)
		{
			m_board.animate();
			InvalidateAllWins();
		}
		else
		{
			on_timer();
		}
		return 0;
	case WM_COMMAND:
		on_command(window, LOWORD(wparam));
		return 0;
	case WM_PAINT:
		on_paint(window);
		return 0;
	case WM_KEYDOWN:
		if (!m_board.isBoardMovable())
		{
			on_losing();
			return 0;
		}
		on_key_pressed(wparam);
		return 0;
	}
	
	return DefWindowProcW(window, message, wparam, lparam);
}

HWND app_2048::create_window(DWORD style, HWND parent, DWORD ex_style)
{
	RECT size{ 0, 0, board::width, board::height + board::field_size + board::margin};
	AdjustWindowRectEx(&size, style, true, 0);
	HWND window = CreateWindowEx(
		ex_style,
		s_class_name.c_str(),
		L"2048",
		style,
		CW_USEDEFAULT, 0,
		size.right-size.left, size.bottom - size.top,
		parent,
		nullptr,
		m_instance,
		this
	);
	/*for (auto& f : m_board.fields())
	{
		CreateWindowExW(
			0,
			L"STATIC",
			nullptr,
			WS_CHILD | WS_VISIBLE | SS_CENTER,
			f.position.left, f.position.top,
			f.position.right - f.position.left,
			f.position.bottom - f.position.top,
			window,
			nullptr,
			m_instance,
			nullptr
		);
	}*/
	return window;
}

void app_2048::on_window_move(HWND window, LPWINDOWPOS params)
{
	HWND other = (window == m_main) ? m_popup : m_main;
	RECT other_rc;
	GetWindowRect(other, &other_rc);
	SIZE other_size{
		.cx = other_rc.right - other_rc.left,
		.cy = other_rc.bottom - other_rc.top
	};

	// Board version
	POINT cw
	{
		.x = board::width / 2,
		.y = board::height / 2
	};

	ClientToScreen(window, reinterpret_cast<LPPOINT>(&cw));

	POINT new_pos
	{
		.x = m_screen_size.x - cw.x - board::width / 2,
		.y = m_screen_size.y - cw.y - board::height / 2
	};
	if (new_pos.x == other_rc.left && new_pos.y == other_rc.top) return;
	SetWindowPos(other, nullptr, new_pos.x, new_pos.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	update_transparency();
}

void app_2048::update_transparency()
{
	RECT main_rc, popup_rc, inter;
	DwmGetWindowAttribute(m_main, DWMWA_EXTENDED_FRAME_BOUNDS, &main_rc, sizeof(RECT));
	DwmGetWindowAttribute(m_popup, DWMWA_EXTENDED_FRAME_BOUNDS, &popup_rc, sizeof(RECT));
	IntersectRect(&inter, &main_rc, &popup_rc);
	BYTE a = IsRectEmpty(&inter) ? 255 : 255 * 50 / 100;
	SetLayeredWindowAttributes(m_popup, 0, a, LWA_ALPHA);
}

void app_2048::on_timer()
{
	using namespace std::chrono;
	auto title = std::format(L"2048 {:%M:%S}",
		duration_cast<duration<int>>(system_clock::now() - m_start_time));
	SetWindowTextW(m_main, title.c_str());
	SetWindowTextW(m_popup, title.c_str());
}

void app_2048::on_command(HWND window, WORD cmdID)
{
	HMENU menu = GetMenu(window);
	switch (cmdID)
	{
	case ID_NEWGAME:
		m_start_time = std::chrono::system_clock::now();
		on_timer();
		score = 0;
		m_board.Reset();
		game_finished = false;
		game_result = 0;
		InvalidateAllWins(true);
		break;
	case ID_GOAL_8:
	case ID_GOAL_16:
	case ID_GOAL_64:
	case ID_GOAL_2048:
		on_menu_item_check(menu, cmdID);
		break;
	}
}

void app_2048::on_paint(HWND window)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(window, &ps);
	HDC comDc = CreateCompatibleDC(dc);
	auto oldBitmap = SelectObject(comDc, m_bkg_bitmap);
	FillRect(comDc, &window_size, m_bkg_brush);

	auto oldBrush = SelectObject(comDc, m_field_brush);
	auto oldPen = SelectObject(comDc, GetStockObject(NULL_PEN));

	// score
	HFONT font = CreateFontW(
		24, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, NULL);
	auto oldColor = SetTextColor(comDc, RGB(255, 0, 0));
	auto oldBkg = SetBkMode(comDc, TRANSPARENT);
	auto oldFont = SelectObject(comDc, font);
	RECT score_rec
	{
		.left = board::margin,
		.top = board::margin,
		.right = board::width - board::margin,
		.bottom = board::margin + board::field_size
	};
	DRAWTEXTPARAMS dp
	{
		.cbSize = sizeof(DRAWTEXTPARAMS),

	};
	auto text = std::format(L"Score: {}", score);
	RoundRect(comDc, board::margin, board::margin, board::width - board::margin, board::margin + board::field_size, 11, 11);
	DrawTextEx(comDc, const_cast<wchar_t *>(text.c_str()), text.length(), &score_rec, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL);
	SetTextColor(comDc, oldColor);

	// tiles
	for (auto& f : m_board.fields())
	{
		if (f.tileNumber > 0)
		{
			POINT cw
			{
				.x = (f.position.left + f.position.right) / 2,
				.y = (f.position.top + f.position.bottom) / 2
			};
			auto dx = cw.x - f.position.left;
			auto dy = cw.y - f.position.top;
			auto text = std::format(L"{}", f.tileNumber);
			auto newBrush = CreateSolidBrush(tile_colors_dict[f.tileNumber]);
			auto toRestoreBrush = SelectObject(comDc, newBrush);
			RoundRect(comDc, f.position.left + (1 - f.scale) * dx, f.position.top + (1 - f.scale) * dy, f.position.right - (1 - f.scale) * dx, f.position.bottom - (1 - f.scale) * dy, 11, 11);
			// RoundRect(dc, f.position.left, f.position.top, f.position.right, f.position.bottom, 11, 11);
			DrawTextEx(comDc, const_cast<wchar_t *>(text.c_str()), text.length(), const_cast<LPRECT>(&f.position), DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL);
			SelectObject(comDc, toRestoreBrush);
			DeleteObject(newBrush);
		}
		else
		{
			RoundRect(comDc, f.position.left, f.position.top, f.position.right, f.position.bottom, 11, 11);
		}
	}

	if (game_result == 1)
	{
		draw_when_winning(window, comDc);
	}
	if (game_result == -1)
	{
		draw_when_losing(window, comDc);
	}
	SelectObject(comDc, oldBrush);
	SelectObject(comDc, oldPen);
	SelectObject(comDc, oldFont);
	SetBkMode(comDc, oldBkg);
	DeleteObject(font);
	BitBlt(dc, 0, 0, window_size.right - window_size.left, window_size.bottom - window_size.top, comDc, 0, 0, SRCCOPY);
	SelectObject(comDc, oldBitmap);
	DeleteDC(comDc);
	EndPaint(window, &ps);
}

void app_2048::on_menu_item_check(HMENU menu, WORD cmdID)
{
	int new_goal = 0;
	for (auto pair : goal_dict)
	{
		if (pair.second == cmdID)
		{
			new_goal = pair.first;
			break;
		}
	}
	CheckMenuItem(menu, goal_dict[goal], MF_UNCHECKED);
	CheckMenuItem(menu, cmdID, MF_CHECKED);
	goal = new_goal;
}

void app_2048::InvalidateAllWins(bool bkg)
{
	InvalidateRect(m_main, NULL, bkg);
	InvalidateRect(m_popup, NULL, bkg);
}

void app_2048::on_key_pressed(WPARAM key)
{
	if (game_finished) return;
	if(game_result != 2)
	{
		const int W_KEY = 0x57;
		const int S_KEY = 0x53;
		const int A_KEY = 0x41;
		const int D_KEY = 0x44;
		switch (key)
		{
		case W_KEY:
			if (m_board.MoveUp(score, goal) == 1)
			{
				on_winning();
				return;
			}
			InvalidateAllWins();
			break;
		case S_KEY:
			if (m_board.MoveDown(score, goal) == 1)
			{
				on_winning();
			}
			InvalidateAllWins();
			break;
		case A_KEY:
			if (m_board.MoveLeft(score, goal) == 1)
			{
				on_winning();
			}
			InvalidateAllWins();
			break;
		case D_KEY:
			if (m_board.MoveRight(score, goal) == 1)
			{
				on_winning();
			}
			InvalidateAllWins();
			break;
		default:
			return;
		}
	}
}

void app_2048::on_losing()
{
	game_finished = true;
	game_result = -1;
	m_board.ResetAllAnimations();
	InvalidateAllWins();
}

void app_2048::on_winning()
{
	game_finished = true;
	game_result = 1;
	m_board.ResetAllAnimations();
	InvalidateAllWins();
}

void app_2048::draw_when_winning(HWND window, HDC dc)
{
	int width = window_size.right - window_size.left;
	int height = window_size.bottom - window_size.top;
	auto comDc = CreateCompatibleDC(dc);
	SelectObject(comDc, m_endgame_bitmap);
	FillRect(comDc, &window_size, m_green_brush);
	BLENDFUNCTION bf
	{
		.BlendOp = AC_SRC_OVER,
		.BlendFlags = 0,
		.SourceConstantAlpha = 128,
		.AlphaFormat = 0
	};
	AlphaBlend(dc, 0, 0, width, height, comDc, 0, 0, width, height, bf);
	auto oldColor = SetTextColor(dc, RGB(255, 255, 255));
	HFONT font = CreateFontW(
		50, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, NULL);
	auto oldFont = SelectObject(dc, font);
	DrawTextExW(dc, const_cast<wchar_t*>(L"YOU WIN!\0"), -1, &window_size, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL);
	SetTextColor(dc, oldColor);
	SelectObject(dc, oldFont);
	DeleteObject(font);
	DeleteDC(comDc);
}

void app_2048::draw_when_losing(HWND window, HDC dc)
{
	int width = window_size.right - window_size.left;
	int height = window_size.bottom - window_size.top;
	auto comDc = CreateCompatibleDC(dc);;
	SelectObject(comDc, m_endgame_bitmap);
	FillRect(comDc, &window_size, m_red_brush);
	BLENDFUNCTION bf
	{
		.BlendOp = AC_SRC_OVER,
		.BlendFlags = 0,
		.SourceConstantAlpha = 128,
		.AlphaFormat = 0
	};
	AlphaBlend(dc, 0, 0, width, height, comDc, 0, 0, width, height, bf);
	auto oldColor = SetTextColor(dc, RGB(255, 255, 255));
	HFONT font = CreateFontW(
		50, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, NULL);
	auto oldFont = SelectObject(dc, font);
	DrawTextExW(dc, const_cast<wchar_t*>(L"YOU LOSE!\0"), -1, &window_size, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL);
	SetTextColor(dc, oldColor);
	SelectObject(dc, oldFont);
	DeleteObject(font);
	DeleteDC(comDc);
}

void app_2048::save_game_state()
{
	auto sectionName = L"GAME DATA";
	auto fileName = L".\\2048.ini";
	auto goalstr = std::format(L"{}", goal);
	auto scorestr = std::format(L"{}", score);
	auto game_resultstr = std::format(L"{}", game_result);
	auto board_str = m_board.get_string();
	bool res1 = WritePrivateProfileStringW(sectionName, L"RESULT", game_resultstr.c_str(), fileName);
	bool res2 = WritePrivateProfileStringW(sectionName, L"GOAL", goalstr.c_str(), fileName);
	bool res3 = WritePrivateProfileStringW(sectionName, L"SCORE", scorestr.c_str(), fileName);
	bool res4 = WritePrivateProfileStringW(sectionName, L"BOARD", board_str.c_str(), fileName);
	//if (!res1 || !res2 || !res3 || !res4) {
	//	DWORD err = GetLastError();
	//	wchar_t buf[256];
	//	wsprintfW(buf, L"Nie uda³o siê zapisaæ! Kod b³êdu: %d", err);
	//	MessageBoxW(NULL, buf, L"B³¹d", MB_OK);
	//}
}

void app_2048::load_game_state()
{
	auto sectionName = L"GAME DATA";
	auto fileName = L".\\2048.ini";
	score = GetPrivateProfileIntW(sectionName, L"SCORE", 0, fileName);

	int new_goal = GetPrivateProfileIntW(sectionName, L"GOAL", 2048, fileName);
	HMENU menu1 = GetMenu(m_main);
	HMENU menu2 = GetMenu(m_popup);
	on_menu_item_check(menu1, goal_dict[new_goal]);
	on_menu_item_check(menu2, goal_dict[new_goal]);
	goal = new_goal;

	game_result = GetPrivateProfileIntW(sectionName, L"RESULT", 0, fileName);
	if (game_result != 0) game_finished = true;

	wchar_t buf[256];
	GetPrivateProfileStringW(sectionName, L"BOARD", L"N", buf, 256, fileName);
	if(buf[0] != L'N')
	{
		m_board.load_board_from_wchar_t(buf);
	}
}


app_2048::app_2048(HINSTANCE instance): m_instance(instance), m_main(), m_popup(), m_field_brush(CreateSolidBrush(RGB(204, 192, 174))),
m_screen_size(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)), goal(2048), score(0), game_finished(false), game_result(0)
{
	register_class();
	DWORD main_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	DWORD popup_style = WS_OVERLAPPED | WS_CAPTION;
	m_main = create_window(main_style, nullptr, WS_EX_COMPOSITED);
	m_popup = create_window(popup_style, m_main, WS_EX_LAYERED | WS_EX_COMPOSITED);
	SetLayeredWindowAttributes(m_popup, 0, 255, LWA_ALPHA);
	load_game_state();
	HDC dc = GetDC(m_main);
	m_bkg_brush = CreateSolidBrush(RGB(250, 247, 238));
	m_green_brush = CreateSolidBrush(RGB(0, 255, 0));
	m_red_brush = CreateSolidBrush(RGB(255, 0, 0));
	m_bkg_bitmap = CreateCompatibleBitmap(dc, window_size.right - window_size.left, window_size.bottom - window_size.top);
	m_endgame_bitmap = CreateCompatibleBitmap(dc, window_size.right - window_size.left, window_size.bottom - window_size.top);
	ReleaseDC(m_main, dc);
}

int app_2048::run(int show_command)
{
	ShowWindow(m_main, show_command);
	ShowWindow(m_popup, SW_SHOWNA);
	SetTimer(m_main, s_timer, 1000, nullptr);
	SetTimer(m_main, s_anim_timer, 10, nullptr);
	m_start_time = std::chrono::system_clock::now();
	HACCEL shortcuts = LoadAcceleratorsW(m_instance, MAKEINTRESOURCE(ID_SHORTCUTS));
	MSG msg{};
	BOOL result = TRUE;
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (result == -1)
		{
			return EXIT_FAILURE;
		}
		if(!TranslateAcceleratorW(msg.hwnd, shortcuts, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	return EXIT_SUCCESS;
}

app_2048::~app_2048()
{
	DeleteObject(m_bkg_brush);
	DeleteObject(m_green_brush);
	DeleteObject(m_red_brush);
	DeleteObject(m_endgame_bitmap);
	DeleteObject(m_bkg_bitmap);
	DeleteObject(m_field_brush);
}
