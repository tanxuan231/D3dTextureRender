#include "cursorHelp.h"

static uint8_t* get_bitmap_data(HBITMAP hbmp, BITMAP* bmp)
{
	if (GetObject(hbmp, sizeof(*bmp), bmp) != 0) {
		uint8_t* output;
		unsigned int size = bmp->bmHeight * bmp->bmWidthBytes;

		output = (uint8_t*)malloc(size);
		GetBitmapBits(hbmp, size, output);
		return output;
	}

	return NULL;
}

static inline uint8_t bit_to_alpha(uint8_t* data, long pixel, bool invert)
{
	uint8_t pix_byte = data[pixel / 8];
	bool alpha = (pix_byte >> (7 - pixel % 8) & 1) != 0;

	if (invert) {
		return alpha ? 0xFF : 0;
	}
	else {
		return alpha ? 0 : 0xFF;
	}
}

static inline bool bitmap_has_alpha(uint8_t* data, long num_pixels)
{
	for (long i = 0; i < num_pixels; i++) {
		if (data[i * 4 + 3] != 0) {
			return true;
		}
	}

	return false;
}

static inline void apply_mask(uint8_t* color, uint8_t* mask, BITMAP* bmp_mask)
{
	long mask_pix_offs;

	for (long y = 0; y < bmp_mask->bmHeight; y++) {
		for (long x = 0; x < bmp_mask->bmWidth; x++) {
			mask_pix_offs = y * (bmp_mask->bmWidthBytes * 8) + x;
			color[(y * bmp_mask->bmWidth + x) * 4 + 3] =
				bit_to_alpha(mask, mask_pix_offs, false);
		}
	}
}

static inline uint8_t* copy_from_color(ICONINFO* ii, uint32_t* width,
	uint32_t* height)
{
	BITMAP bmp_color;
	BITMAP bmp_mask;
	uint8_t* color;
	uint8_t* mask;

	color = get_bitmap_data(ii->hbmColor, &bmp_color);
	if (!color) {
		return NULL;
	}

	if (bmp_color.bmBitsPixel < 32) {
		free(color);
		return NULL;
	}

	mask = get_bitmap_data(ii->hbmMask, &bmp_mask);
	if (mask) {
		long pixels = bmp_color.bmHeight * bmp_color.bmWidth;

		if (!bitmap_has_alpha(color, pixels))
			apply_mask(color, mask, &bmp_mask);

		free(mask);
	}

	*width = bmp_color.bmWidth;
	*height = bmp_color.bmHeight;
	return color;
}

static inline uint8_t* copy_from_mask(ICONINFO* ii, uint32_t* width,
	uint32_t* height)
{
	uint8_t* output;
	uint8_t* mask;
	long pixels;
	long bottom;
	BITMAP bmp;

	mask = get_bitmap_data(ii->hbmMask, &bmp);
	if (!mask) {
		return NULL;
	}

	bmp.bmHeight /= 2;

	pixels = bmp.bmHeight * bmp.bmWidth;
	output = (uint8_t*)malloc(pixels * 4);
	if (!output) {
		return NULL;
	}
	memset(output, 0, pixels * 4);

	bottom = bmp.bmWidthBytes * bmp.bmHeight;

	for (long i = 0; i < pixels; i++) {
		uint8_t alpha = bit_to_alpha(mask, i, false);
		uint8_t color = bit_to_alpha(mask + bottom, i, true);

		if (!alpha) {
			output[i * 4 + 3] = color;
		}
		else {
			*(uint32_t*)&output[i * 4] = !!color ? 0xFFFFFFFF
				: 0xFF000000;
		}
	}

	free(mask);

	*width = bmp.bmWidth;
	*height = bmp.bmHeight;
	return output;
}

uint8_t* cursor_capture_icon_bitmap(ICONINFO* ii, uint32_t* width,
	uint32_t* height)
{
	uint8_t* output;

	output = copy_from_color(ii, width, height);
	if (!output)
		output = copy_from_mask(ii, width, height);

	return output;
}
