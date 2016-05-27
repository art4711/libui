#include "uipriv_unix.h"
#include "draw.h"

#include <machine/endian.h>

struct uiImage {
	cairo_surface_t *s;
};

/*
 * A standard way of specifying endianness with a macro/constant has
 * not yet been invented. Roll our own. This works on gcc/clang and
 * gets optimized to compile time constants on i386, but not arm.
 * Also hasn't been tested on a big endian machine, other than that the
 * generated assembler looks right. What could possibly go wrong?
 */
const static union {
        uint32_t a;
        struct {
                char little;
                char pad0;
                char pad1;
                char big;
        } is;
} endian = {
        .a = 1
};

uiImage *uiNewImage(int width, int height)
{
	uiImage *img = uiNew(uiImage);
	img->s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	return img;
}

void uiFreeImage(uiImage *img)
{
	cairo_surface_destroy(img->s);
	uiFree(img);
}

void uiImageGetData(uiImage *img, uiImageData *id)
{
	id->fmt = uiPixmap32FormatHasAlpha | uiPixmap32FormatAlphaPremultiplied;
	if (endian.is.little) {
		id->fmt |= uiPixmap32FormatOffsets(3, 2, 1, 0);
	} else {
		id->fmt |= uiPixmap32FormatOffsets(0, 1, 2, 3);
	}
	id->width = cairo_image_surface_get_width(img->s);
	id->height = cairo_image_surface_get_height(img->s);
	id->rowstride = cairo_image_surface_get_stride(img->s);
	id->data = cairo_image_surface_get_data(img->s);
}

/*
 * Load a pixmap into the image with the upper left corner at x,y.
 * If the x+width and/or y+height of the source are bigger than the
 * image, the result will be clamped.
 *
 * Rowstride of the pixmap must be divisible by 4. Or else.
 *
 * note: a smaller rectangle than your whole image can be loaded by
 * manipulating width, height and offset into the data so there's no
 * need to support that.
 */
void uiImageLoadPixmap32Raw(uiImage *img, int x, int y, int width, int height, int rowstrideBytes, uiPixmap32Format fmt, void *data)
{
	int sw;
	int sh;
	int srs;
	int dw;
	int dh;
	int drs;
	uint32_t *dst;
	uiImageData dstd;

	uiImageGetData(img, &dstd);
	drs = dstd.rowstride / 4;
	dst = (uint32_t *)dstd.data;	// assumes good alignment.
	dw = dstd.width;
	dh = dstd.height;

	sw = width + x > dw ? dw : width;
	sh = height + y > dh ? dh : height;
	srs = rowstrideBytes / 4;

	if ((rowstrideBytes & 3) != 0)
		userbug("rowstride is not divisble by 4, your code must be actively hostile to generate that");

	pixmap32RawCopy(sw, sh, srs, data, fmt, drs, &dst[y * drs + x], dstd.fmt);
	cairo_surface_mark_dirty(img->s);
}

// TODO Since we're doing allocations now, we probably want a function to extract the byte array and its parameters.

void uiDrawImage(uiDrawContext *c, double x, double y, uiImage *img)
{
	cairo_surface_mark_dirty(img->s);		// This is the best place for this, I guess.
	cairo_set_source_surface(c->cr, img->s, x, y);
	cairo_paint(c->cr);
}
