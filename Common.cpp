#include "Common.h"
#include <Windows.h>
#include <iostream>

void saveBitmap(unsigned char* bitmap_data, int rowPitch, int height, const char* filename)
{
    // A file is created, this is where we will save the screen capture.

    FILE* f;

    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = rowPitch / 4;
    //Make the size negative if the image is upside down.
    bi.biHeight = -height;
    //There is only one plane in RGB color space where as 3 planes in YUV.
    bi.biPlanes = 1;
    //In windows RGB, 8 bit - depth for each of R, G, B and alpha.
    bi.biBitCount = 32;
    //We are not compressing the image.
    bi.biCompression = BI_RGB;
    // The size, in bytes, of the image. This may be set to zero for BI_RGB bitmaps.
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // rowPitch = the size of the row in bytes.
    DWORD dwSizeofImage = rowPitch * height;

    // Add the size of the headers to the size of the bitmap to get the total file size
    DWORD dwSizeofDIB = dwSizeofImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    //Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    //Size of the file
    bmfHeader.bfSize = dwSizeofDIB;

    //bfType must always be BM for Bitmaps
    bmfHeader.bfType = 0x4D42; //BM   

                               // TODO: Handle getting current directory
    fopen_s(&f, filename, "wb");

    DWORD dwBytesWritten = 0;
    dwBytesWritten += fwrite(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, f);
    dwBytesWritten += fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, f);
    dwBytesWritten += fwrite(bitmap_data, 1, dwSizeofImage, f);

    fclose(f);
}