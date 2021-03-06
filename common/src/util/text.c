// md-toolchain print utility functions
// Michael Moffitt 2018
#include "util/text.h"

static uint16_t font_vram_pos;
static uint16_t font_pal_line;

void text_init(const unsigned char *font_chr, uint16_t font_len,
               uint16_t vram_pos,
               const unsigned char *font_pal, uint16_t pal_line)
{
	font_vram_pos = vram_pos;
	font_pal_line = pal_line & 0x3;
	if (font_chr)
	{
		dma_q_transfer_vram(font_vram_pos, (void *)font_chr, font_len/2, 2);
	}
	font_vram_pos /= 32;  // convert to tile number
	if (font_pal)
	{
		pal_upload(font_pal_line * 16, (void *)font_pal, 16);
	}
}

void text_puts(VdpPlane plane, uint16_t x, uint16_t y, const char *s)
{
	if (plane > VDP_PLANE_WINDOW)
	{
		return;
	}
	uint16_t dest_base = vdp_get_plane_base(plane);
	uint8_t plane_size = vdp_get_reg(VDP_PLANESIZE);
	uint16_t line_inc = 64;
	vdp_wait_dma();
	vdp_set_autoinc(2);
	switch(plane_size)
	{
		default:
			return;
		case VDP_PLANESIZE_32x32:
		case VDP_PLANESIZE_32x64:
		case VDP_PLANESIZE_32x128:
			line_inc = 64;
			break;
		case VDP_PLANESIZE_64x32:
		case VDP_PLANESIZE_64x64:
			line_inc = 128;
			break;
		case VDP_PLANESIZE_128x32:
			line_inc = 256;
			break;
	}

	dest_base += x * 2;
	dest_base += y * line_inc;

	VDPPORT_CTRL32 = VDP_CTRL_VRAM_WRITE | VDP_CTRL_ADDR(dest_base);

	while (*s)
	{
		if (*s == '\n')
		{
			y++;
			dest_base = vdp_get_plane_base(plane);
			dest_base += x * 2;
			dest_base += y * line_inc;

			VDPPORT_CTRL32 = VDP_CTRL_VRAM_WRITE | VDP_CTRL_ADDR(dest_base);
		}
		else
		{
			VDPPORT_DATA = VDP_ATTR(((font_vram_pos + *s) - 0x20),
			                        0, 0, font_pal_line, 1);
		}
		s++;
	}
}

uint16_t text_get_vram_pos(void)
{
	return font_vram_pos;
}
