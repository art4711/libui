#import "uipriv_darwin.h"

uiPixmap32Format uiImagePreferedPixmap32Format(void)
{
	/* Always big endian, because why not. */
	return uiPixmap32FormatHasAlpha | uiPixmap32FormatAlphaPremultiplied | uiPixmap32FormatZeroRowBottom | uiPixmap32FormatOffsets(0, 1, 2, 3);
}

uiImage *uiNewImage(int w, int h)
{
	uiImage *img = uiNew(uiImage);

	/*
	 * Round up to the nearest multiple of 16 since that's the
	 * optimal rowstride according to Quartz documentation.
	 */
	img->rowstride = (w * 4) + 0xf & ~0xf;
	img->w = w;
	img->h = h;
	img->bmapdata = uiAlloc(h * img->rowstride, "imgdata");

	img->c = CGBitmapContextCreate(img->bmapdata, w, h, 8, img->rowstride, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big);

	return img;
}

void uiFreeImage(uiImage *img)
{
	uiFree(img->bmapdata);
	CGContextRelease(img->c);
	uiFree(img);
}

void uiImageLoadPixmap32Raw(uiImage *img, int x, int y, int width, int height, int rowstrideBytes, uiPixmap32Format fmt, void *data)
{
	int dw = img->w;
	int dh = img->h;
	int sw = width + x > dw ? dw : width;
	int sh = height + y > dh ? dh : height;
	int drs = img->rowstride / 4;
	int srs = rowstrideBytes / 4;
	uint32_t *dst = img->bmapdata;

	if ((rowstrideBytes & 3) != 0)
		userbug("rowstride is not divisble by 4, your code must be actively hostile to generate that");

	pixmap32RawCopy(sw, sh, srs, data, fmt, drs, &dst[y * drs + x], uiImagePreferedPixmap32Format());
}
