#pragma once

#include <Windows.h>
#include <WinUser.h>

#define JUDGER(result) if (!result) { /*MessageBox(nullptr, L"Error JUDGER", L##"__FUNCTION__", MB_OK);*/ return false; }

void saveBitmap(unsigned char* bitmap_data, int rowPitch, int height, const char* filename);