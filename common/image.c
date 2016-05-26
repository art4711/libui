#include <string.h>
#include "../ui.h"
#include "uipriv.h"

// the *rs arguments are rowstride in 32 bit pixels, not bytes.
void pixmap32RawCopy(int width, int height, int srs, uint32_t *src, uiPixmap32Format sfmt, int drs, uint32_t *dst, uiPixmap32Format dfmt)
{
	int drow, srow;
	int srowinc;
	/*
	 * Destination is always written from row 0 to height. Source
	 * read direction depends on if we need to flip the image.
	 */
	if ((sfmt & uiPixmap32FormatZeroRowBottom) == (dfmt & uiPixmap32FormatZeroRowBottom)) {
		srow = 0;
		srowinc = 1;
	} else {
		srow = height - 1;
		srowinc = -1;
	}
	if ((sfmt & ~uiPixmap32FormatZeroRowBottom) == (dfmt & ~uiPixmap32FormatZeroRowBottom)) {
		/* Simple case, at most only the row order is different, the rest is easy. */
		for (drow = 0; drow < height; drow++, srow += srowinc)
			memcpy(&dst[drow * drs], &src[srow * srs], width * 4);
	} else if ((sfmt & (uiPixmap32FormatHasAlpha|uiPixmap32FormatAlphaPremultiplied)) == (sfmt & (uiPixmap32FormatHasAlpha|uiPixmap32FormatAlphaPremultiplied))) {
		for (drow = 0; drow < height; drow++, srow += srowinc) {
			uint8_t *sa = ((uint8_t *)&src[srow * srs]) + uiPixmap32FormatOffsetA(sfmt);
			uint8_t *sr = ((uint8_t *)&src[srow * srs]) + uiPixmap32FormatOffsetR(sfmt);
			uint8_t *sg = ((uint8_t *)&src[srow * srs]) + uiPixmap32FormatOffsetG(sfmt);
			uint8_t *sb = ((uint8_t *)&src[srow * srs]) + uiPixmap32FormatOffsetB(sfmt);
			uint8_t *da = ((uint8_t *)&dst[drow * drs]) + uiPixmap32FormatOffsetA(dfmt);
			uint8_t *dr = ((uint8_t *)&dst[drow * drs]) + uiPixmap32FormatOffsetR(dfmt);
			uint8_t *dg = ((uint8_t *)&dst[drow * drs]) + uiPixmap32FormatOffsetG(dfmt);
			uint8_t *db = ((uint8_t *)&dst[drow * drs]) + uiPixmap32FormatOffsetB(dfmt);
			int col;
			for (col = 0; col < width; col++) {
				*da = *sa; sa += 4; da += 4;
				*dr = *sr; sr += 4; dr += 4;
				*dg = *sg; sg += 4; dg += 4;
				*db = *sb; sb += 4; db += 4;				
			}
		}
	} else {
		implbug("alpha mismatch not implemented yet");
	}
}
