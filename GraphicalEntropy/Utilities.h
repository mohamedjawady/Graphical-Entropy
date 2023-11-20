#ifndef UTILITIES_H
#define UTILITIES_H

#include <wtypes.h>
#include <string>
#include <vector>

void DrawEntropyAndPath(HDC hdc);
bool IsBinary(const std::vector<unsigned char>& data);
std::wstring GetFileFormat(const std::wstring& filePath);
void DrawTableGrid(HDC hdc, int x, int y, int rows, int columns, int cellWidth, int cellHeight, int lineHeight);

#endif