#include "board.h"
#include <vector>
#include <set>
#include <time.h>
#include <list>
#include <algorithm>
#include <string>
#include <sstream>
#include <cmath>

board::board(): m_fields()
{
	for (LONG row = 0; row < rows; ++row)
	{
		for (LONG column = 0; column < columns; ++column)
		{
			auto& f = m_fields[row * columns + column];
			f.position.top = (row + 1) * (field_size + margin) + margin;
			f.position.left = column * (field_size + margin) + margin;
			f.position.bottom = f.position.top + field_size;
			f.position.right = f.position.left + field_size;
			f.tileNumber = 0;
			f.combined = false;
			f.animationType = 0;
			f.scale = 1.0f;
			f.scaling_direction = 1.0f;
		}
	}
	MakeRandomTwo();
}

int board::MakeRandomTwo()
{
	std::vector<POINT> free_fields;
	for (LONG row = 0; row < rows; ++row)
	{
		for (LONG column = 0; column < columns; ++column)
		{
			if (m_fields[row * columns + column].tileNumber == 0)
			{
				free_fields.push_back(POINT{
					.x = column,
					.y = row
					});
			}
		}
	}

	srand(time(NULL));
	if (free_fields.size() != 0)
	{
		int rand_point = rand() % free_fields.size();
		m_fields[free_fields[rand_point].y * columns + free_fields[rand_point].x].tileNumber = 2;
		m_fields[free_fields[rand_point].y * columns + free_fields[rand_point].x].animationType = CREATE_ANIMATION;
		m_fields[free_fields[rand_point].y * columns + free_fields[rand_point].x].scale = 0.0f;
		return 0;
	}

	return -1;
}

void board::Reset()
{
	for (LONG row = 0; row < rows; ++row)
	{
		for (LONG column = 0; column < columns; ++column)
		{
			m_fields[row * columns + column].tileNumber = 0;
		}
	}
	MakeRandomTwo();

}

int board::MoveLeft(int &score, int goal)
{
	for (LONG column = 1; column < columns; ++column)
	{
		for (LONG row = 0; row < rows; ++row)
		{
			if (m_fields[row * columns + column].tileNumber > 0)
			{
				for (LONG col = column - 1; col >= 0; col--)
				{
					if (m_fields[row * columns + col].tileNumber > 0)
					{

						if (m_fields[row * columns + col].tileNumber == m_fields[row * columns + column].tileNumber && !m_fields[row * columns + col].combined)
						{
							score += MoveTile(row, column, row, col);
							if (m_fields[row * columns + col].tileNumber >= goal) return 1;
							break;
						}
						else
						{
							score += MoveTile(row, column, row, col + 1);
							break;
						}
					}
					if (col == 0)
					{
						score += MoveTile(row, column, row, col);
						break;
					}
				}
			}
		}
	}
	ClearCombined();
	MakeRandomTwo();
	return 0;
}

int board::MoveRight(int& score, int goal)
{
	for (LONG column = columns-2; column >= 0; --column)
	{
		for (LONG row = 0; row < rows; ++row)
		{
			if (m_fields[row * columns + column].tileNumber > 0)
			{
				for (LONG col = column + 1; col < columns; col++)
				{
					if (m_fields[row * columns + col].tileNumber > 0)
					{

						if (m_fields[row * columns + col].tileNumber == m_fields[row * columns + column].tileNumber && !m_fields[row * columns + col].combined)
						{
							score += MoveTile(row, column, row, col);
							if (m_fields[row * columns + col].tileNumber >= goal) return 1;
							break;
						}
						else
						{
							score += MoveTile(row, column, row, col - 1);
							break;
						}
					}
					if (col == columns - 1)
					{
						score += MoveTile(row, column, row, col);
						break;
					}
				}
			}
		}
	}
	ClearCombined();
	MakeRandomTwo();
	return 0;
}

int board::MoveUp(int& score, int goal)
{
	for (LONG row = 1; row < rows; ++row)
	{
		for (LONG column = 0; column < columns; ++column)
		{
			if (m_fields[row * columns + column].tileNumber > 0)
			{
				for (LONG r = row - 1; r >= 0; --r)
				{
					if (m_fields[r * columns + column].tileNumber > 0)
					{

						if (m_fields[r * columns + column].tileNumber == m_fields[row * columns + column].tileNumber && !m_fields[r * columns + column].combined)
						{
							score += MoveTile(row, column, r, column);
							if (m_fields[r * columns + column].tileNumber >= goal) return 1;
							break;
						}
						else
						{
							score += MoveTile(row, column, r + 1, column);
							break;
						}
					}
					if (r == 0)
					{
						score += MoveTile(row, column, r, column);
						break;
					}
				}
			}
		}
	}
	ClearCombined();
	MakeRandomTwo();
	return 0;
}

int board::MoveDown(int& score, int goal)
{
	for (LONG row = rows - 2; row >= 0; --row)
	{
		for (LONG column = 0; column < columns; ++column)
		{
			if (m_fields[row * columns + column].tileNumber > 0)
			{
				for (LONG r = row + 1; r < rows; ++r)
				{
					if (m_fields[r * columns + column].tileNumber > 0)
					{

						if (m_fields[r * columns + column].tileNumber == m_fields[row * columns + column].tileNumber && !m_fields[r * columns + column].combined)
						{
							score += MoveTile(row, column, r, column);
							if (m_fields[r * columns + column].tileNumber >= goal) return 1;
							break;
						}
						else
						{
							score += MoveTile(row, column, r - 1, column);
							break;
						}
					}
					if (r == rows - 1)
					{
						score += MoveTile(row, column, r, column);
						break;
					}
				}
			}
		}
	}
	ClearCombined();
	MakeRandomTwo();
	return 0;
}

int board::MoveTile(LONG row_from, LONG col_from, LONG row_to, LONG col_to)
{
	if (row_from == row_to && col_from == col_to)
	{
		return 0;
	}

	// combining two tiles
	if (m_fields[row_from * columns + col_from].tileNumber == m_fields[row_to * columns + col_to].tileNumber)
	{
		m_fields[row_from * columns + col_from].tileNumber = 0;
		m_fields[row_to * columns + col_to].tileNumber *= 2;
		m_fields[row_to * columns + col_to].combined = true;
		m_fields[row_to * columns + col_to].animationType = animationType::MERGE_ANIMATION;
		return m_fields[row_to * columns + col_to].tileNumber;
	}

	// moving to empty tile
	m_fields[row_to * columns + col_to].tileNumber = m_fields[row_from * columns + col_from].tileNumber;
	m_fields[row_from * columns + col_from].tileNumber = 0;
	return 0;
}

void board::ClearCombined()
{
	for (LONG column = 0; column < columns; ++column)
	{
		for (LONG row = 0; row < rows; ++row)
		{
			m_fields[row * columns + column].combined = false;
		}
	}
}

void board::ResetAllAnimations()
{
	for (auto& f : m_fields)
	{
		f.animationType = animationType::NO_ANIMATION;
		f.scale = 1.0f;
		f.scaling_direction = 1.0f;
	}
}

bool board::isBoardMovable()
{
	for (LONG row = 0; row < rows; ++row)
	{
		for (LONG column = 0; column < columns; ++column)
		{
			auto& f = m_fields[row * columns + column];
			if(f.tileNumber == 0) return true;
			if (row - 1 >= 0)
			{
				if (m_fields[(row - 1) * columns + column].tileNumber == f.tileNumber) return true;
			}
			if (row + 1 < rows)
			{
				if (m_fields[(row + 1) * columns + column].tileNumber == f.tileNumber) return true;
			}
			if (column - 1 >= 0)
			{
				if (m_fields[(row) * columns + column - 1].tileNumber == f.tileNumber) return true;
			}
			if (column + 1 < columns)
			{
				if (m_fields[(row) * columns + column + 1].tileNumber == f.tileNumber) return true;
			}
		}
	}
	return false;
}

std::wstring board::get_string()
{
	std::wstringstream ss;
	for (LONG row = 0; row < rows; ++row)
	{
		for (LONG column = 0; column < columns; ++column)
		{
			ss << m_fields[row * columns + column].tileNumber << " ";
		}
	}
	return ss.str();
}

void board::load_board_from_wchar_t(wchar_t * buf)
{
	std::wstringstream ss(buf);
	for (LONG row = 0; row < rows; ++row)
	{
		for (LONG column = 0; column < columns; ++column)
		{
			ss >> m_fields[row * columns + column].tileNumber;
		}
	}
}

bool board::animate()
{
	bool ret = false;
	for (auto& f : m_fields)
	{
		switch (f.animationType)
		{
		case animationType::NO_ANIMATION:
			continue;
		case animationType::CREATE_ANIMATION:
		case animationType::MERGE_ANIMATION:
			if (f.tileNumber == 0)
			{
				f.animationType = animationType::NO_ANIMATION;
				f.scale = 1.0f;
				f.scaling_direction = 1.0f;
				continue;
			}
			if (abs(f.scale - 1.2f) < 0.01f)
			{
				f.scaling_direction = -1.0f;
			}
			f.scale += f.scaling_direction * 0.1f;
			if (f.scaling_direction < 0 && abs (f.scale - 1.0f )< 0.01f || f.scale < 0 || f.scale > 1.3f)
			{
				f.scale = 1.0f;
				f.scaling_direction = 1.0f;
				f.animationType = NO_ANIMATION;
			}
			ret = true;
			break;
		}
	}
	return ret;
}
