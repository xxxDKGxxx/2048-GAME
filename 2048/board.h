#pragma once
#include <array>
#include <windows.h>
#include <string>

struct field
{
	RECT position;
	int tileNumber;
	bool combined;
	int animationType;
	float scale;
	float scaling_direction;
};

enum animationType
{
	NO_ANIMATION = 0,
	CREATE_ANIMATION = 1,
	MERGE_ANIMATION = 2
};
class board
{
public:
	static constexpr LONG columns = 4;
	static constexpr LONG rows = 4;
	static constexpr LONG margin = 10;
	static constexpr LONG field_count = rows * columns;
	static constexpr LONG field_size = 60;
	static constexpr LONG width =
		columns * (field_size + margin) + margin;
	static constexpr LONG height =
		rows * (field_size + margin) + margin;
	using field_array = std::array<field, field_count>;
	board();
	field_array const& fields() const { return m_fields; };
	int MakeRandomTwo();
	void Reset();
	int MoveLeft(int &score, int goal);
	int MoveRight(int &score, int goal);
	int MoveUp(int &score, int goal);
	int MoveDown(int &score, int goal);
	int MoveTile(LONG row_from, LONG col_from, LONG row_to, LONG col_to, bool& moved);
	void ClearCombined();
	void ResetAllAnimations();
	bool isBoardMovable();
	std::wstring get_string();
	void load_board_from_wchar_t(wchar_t * buf);
	bool animate();
private:
	field_array m_fields;
};