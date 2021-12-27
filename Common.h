#pragma once

#define JUDGER(result) if (!result) return false;

void saveBitmap(unsigned char* bitmap_data, int rowPitch, int height, const char* filename);