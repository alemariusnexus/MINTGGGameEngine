#include "MemDrawSurface.h"

#include <algorithm>
#include <cmath>

#include "util/Util.h"


LOG_USE_TAG("MemDrawSurface")


namespace MINTGGGameEngine
{


MemDrawSurface::MemDrawSurface (
    uint8_t* buffer, uint16_t width, uint16_t height,
    const delete_fn_t& del,
    bool swapEndianness
)
    : buf(buffer), width(width), height(height), del(del), clipX(0), clipY(0), clipW(width), clipH(height),
      swapEndianness(swapEndianness)
{
    // TODO: Check buffer alignment (16 bits for RGB565)
}

MemDrawSurface::~MemDrawSurface()
{
    del(buf);
    buf = nullptr;
}

uint16_t MemDrawSurface::getWidth() const
{
    return width;
}

uint16_t MemDrawSurface::getHeight() const
{
    return height;
}

void MemDrawSurface::fill(const Color& color, bool honorClipRegion)
{
    const auto sw = getWidth();

    uint16_t curClipX, curClipY, curClipW, curClipH;
    if (honorClipRegion) {
        curClipX = clipX;
        curClipY = clipY;
        curClipW = clipW;
        curClipH = clipH;
    } else {
        curClipX = 0;
        curClipY = 0;
        curClipW = sw;
        curClipH = getHeight();
    }

    uint16_t rgb565 = color.toRGB565();
    if (swapEndianness) {
        rgb565 = SwapEndianness(rgb565);
    }

    const uint16_t ex = curClipX + curClipW;
    const uint16_t ey = curClipY + curClipH;
    for (uint16_t y = curClipY ; y < ey ; y++) {
        uint16_t* buf16 = reinterpret_cast<uint16_t*>(buf) + y*sw + curClipX;
        for (uint16_t x = curClipX ; x < ex ; x++) {
            *buf16++ = rgb565;
        }
    }
}

void MemDrawSurface::drawPixel(int32_t x, int32_t y, const Color& color)
{
    if (x < clipX  ||  y < clipY) {
        return;
    }
    if (x >= clipX+clipW  ||  y >= clipY+clipH) {
        return;
    }
    uint16_t* buf16 = reinterpret_cast<uint16_t*>(buf);
    uint16_t rgb565 = color.toRGB565();
    if (swapEndianness) {
        rgb565 = SwapEndianness(rgb565);
    }
    buf16[y*static_cast<size_t>(getWidth()) + x] = rgb565;
}

void MemDrawSurface::drawHLine(int32_t x0, int32_t y0, int32_t w, const Color& color)
{
    // Handle negative width
    if (w < 0) {
        x0 += w;
        w = -w;
    }

    // Vertically out of frame
    if (y0 < clipY  ||  y0 >= clipY+clipH) {
        return;
    }

    // Clip low side
    if (x0 < clipX) {
        w -= (clipX-x0);
        x0 = clipX;
    }
    // Clip high side
    if (x0+w > clipX+clipW) {
        w = clipX+clipW-x0;
    }

    if (w <= 0) {
        return;
    }

    assert(x0 >= clipX);
    assert(x0+w <= clipX+clipW);
    assert(y0 >= clipY);
    assert(y0 < clipY+clipH);

    uint16_t rgb565 = color.toRGB565();
    if (swapEndianness) {
        rgb565 = SwapEndianness(rgb565);
    }

    uint16_t* buf16 = reinterpret_cast<uint16_t*>(buf) + y0*getWidth() + x0;
    const uint16_t* buf16End = buf16 + w;
    while (buf16 != buf16End) {
        *buf16++ = rgb565;
    }
}

void MemDrawSurface::drawVLine(int32_t x0, int32_t y0, int32_t h, const Color& color)
{
    // Handle negative height
    if (h < 0) {
        y0 += h;
        h = -h;
    }

    // Horizontally out of frame
    if (x0 < clipX  ||  x0 >= clipX+clipW) {
        return;
    }

    // Clip low side
    if (y0 < clipY) {
        h -= (clipY-y0);
        y0 = clipY;
    }
    // Clip high side
    if (y0+h > clipY+clipH) {
        h = clipY+clipH-y0;
    }

    if (h <= 0) {
        return;
    }

    assert(x0 >= clipX);
    assert(x0 < clipX+clipW);
    assert(y0 >= clipY);
    assert(y0+h <= clipY+clipH);

    uint16_t rgb565 = color.toRGB565();
    if (swapEndianness) {
        rgb565 = SwapEndianness(rgb565);
    }

    const auto sw = getWidth();

    uint16_t* buf16 = reinterpret_cast<uint16_t*>(buf) + y0*sw + x0;
    const uint16_t* buf16End = buf16 + h*sw;
    while (buf16 != buf16End) {
        *buf16 = rgb565;
        buf16 += sw;
    }
}

void MemDrawSurface::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color)
{
    if (x0 == x1) {
        drawVLine(x0, y0, y1-y0, color);
        return;
    }
    if (y0 == y1) {
        drawHLine(x0, y0, x1-x0, color);
        return;
    }


    // ********** CLIP LINE **********

    // Avoid float calculations if both points lie entirely within the clipping region -> trivial no-clipping case
    if (
            x0 < clipX  ||  x0 >= clipX+clipW
        ||  y0 < clipY  ||  y0 >= clipY+clipH
        ||  x1 < clipX  ||  x1 >= clipX+clipW
        ||  y1 < clipY  ||  y1 >= clipY+clipH
    ) {
        auto fx0 = static_cast<float>(x0);
        auto fy0 = static_cast<float>(y0);
        auto fx1 = static_cast<float>(x1);
        auto fy1 = static_cast<float>(y1);

        if (!clipLine(fx0, fy0, fx1, fy1)) {
            // Entirely outside of clip region
            return;
        }

        x0 = static_cast<int32_t>(std::round(fx0));
        y0 = static_cast<int32_t>(std::round(fy0));
        x1 = static_cast<int32_t>(std::round(fx1));
        y1 = static_cast<int32_t>(std::round(fy1));
    }


    // ********** BRESENHAM'S ALGORITHM **********

    const auto sw = getWidth();
    uint16_t rgb565 = color.toRGB565();
    if (swapEndianness) {
        rgb565 = SwapEndianness(rgb565);
    }

    const int32_t dx = std::abs(x1 - x0);
    const int32_t sx = x0 < x1 ? 1 : -1;
    const int32_t dy = -std::abs(y1 - y0);
    const int32_t sy = y0 < y1 ? 1 : -1;
    int32_t error = dx + dy;

    auto* buf16 = reinterpret_cast<uint16_t*>(buf);

    while (true) {
        buf16[y0*sw + x0] = rgb565;

        const int32_t e2 = 2 * error;
        if (e2 >= dy) {
            if (x0 == x1) {
                break;
            }
            error = error + dy;
            x0 = x0 + sx;
        }
        if (e2 <= dx) {
            if (y0 == y1) {
                break;
            }
            error = error + dx;
            y0 = y0 + sy;
        }
    }
}

void MemDrawSurface::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled)
{
    if (filled) {
        // Handle cases of negative width and height
        if (w < 0) {
            w = -w;
            x = x-w;
        }
        if (h < 0) {
            h = -h;
            y = y-h;
        }

        // Handle full rectangle out of bounds (towards high side)
        if (x >= clipX+clipW  ||  y >= clipY+clipH) {
            return;
        }
        // Handle full rectangle out of bounds (towards low side)
        if (x+w <= clipX  ||  y+h <= clipY) {
            return;
        }

        // Clip lowest corner to view area
        if (x < clipX) {
            w -= (clipX-x);
            x = clipX;
        }
        if (y < clipY) {
            h -= (clipY-y);
            y = clipY;
        }

        // Clip highest corner to view area
        if (x+w > clipX+clipW) {
            w = clipX+clipW - x;
        }
        if (y+h > clipY+clipH) {
            h = clipY+clipH - y;
        }

        if (w <= 0  ||  h <= 0) {
            return;
        }

        assert(x >= clipX);
        assert(y >= clipY);
        assert(w > 0);
        assert(h > 0);
        assert(x+w <= clipX+clipW);
        assert(y+h <= clipY+clipH);

        const auto sw = getWidth();

        uint16_t rgb565 = color.toRGB565();
        if (swapEndianness) {
            rgb565 = SwapEndianness(rgb565);
        }

        const uint16_t ey = y+h;
        const uint16_t ex = x+w;
        for (uint16_t py = y ; py < ey ; py++) {
            uint16_t* buf16 = reinterpret_cast<uint16_t*>(buf) + py*sw + x;
            for (uint16_t px = x ; px < ex ; px++) {
                *buf16++ = rgb565;
            }
        }
    } else {
        drawHLine(x, y, w, color);
        drawHLine(x, y+h, w, color);
        drawVLine(x, y, h, color);
        drawVLine(x+w, y, h, color);
    }
}

void MemDrawSurface::drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled)
{
    // This was adapted from HAGL. Apparently it's the Midpoint circle algorithm.

    if (filled) {
        int32_t x = 0;
        int32_t y = r;
        int32_t d = 3 - 2 * r;

        while (y >= x) {
            drawHLine(cx - x, cy + y, x * 2 + 1, color);
            drawHLine(cx - x, cy - y, x * 2 + 1, color);
            drawHLine(cx - y, cy + x, y * 2 + 1, color);
            drawHLine(cx - y, cy - x, y * 2 + 1, color);

            if (d <= 0) {
                d = d + 4 * x + 6;
                x++;
            } else {
                d = d + 4 * (x - y) + 10;
                x++;
                y--;
            }
        }
    } else {
        if (r == 0) {
            drawPixel(cx, cy, color);
            return;
        }

        int32_t x = 0;
        int32_t y = r;
        int32_t d = 3 - 2 * r;

        drawPixel(cx + x, cy + y, color);
        drawPixel(cx - x, cy + y, color);
        drawPixel(cx + x, cy - y, color);
        drawPixel(cx - x, cy - y, color);
        drawPixel(cx + y, cy + x, color);
        drawPixel(cx - y, cy + x, color);
        drawPixel(cx + y, cy - x, color);
        drawPixel(cx - y, cy - x, color);

        while (y >= x) {
            if (d > 0) {
                d = d + 4 * (x - y) + 10;
                y--;
                x++;
            } else {
                d = d + 4 * x + 6;
                x++;
            }

            drawPixel(cx + x, cy + y, color);
            drawPixel(cx - x, cy + y, color);
            drawPixel(cx + x, cy - y, color);
            drawPixel(cx - x, cy - y, color);
            drawPixel(cx + y, cy + x, color);
            drawPixel(cx - y, cy + x, color);
            drawPixel(cx + y, cy - x, color);
            drawPixel(cx - y, cy - x, color);
        }
    }
}

void MemDrawSurface::drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir)
{
    DrawBitmapContext ctx = {
        .buf = reinterpret_cast<uint16_t*>(buf),
        .width = getWidth()
    };
    if (swapEndianness) {
        drawBitmapHelper (
            x, y,
            bitmap,
            flipDir,
            &MemDrawSurface::drawBitmapHelper_drawPixelSwapped,
            &MemDrawSurface::drawBitmapHelper_drawPixelsSwapped,
            &ctx
            );
    } else {
        drawBitmapHelper (
            x, y,
            bitmap,
            flipDir,
            &MemDrawSurface::drawBitmapHelper_drawPixel,
            &MemDrawSurface::drawBitmapHelper_drawPixels,
            &ctx
            );
    }
}

void MemDrawSurface::setClipRegion(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch)
{
    const auto sw = getWidth();
    const auto sh = getHeight();

    clipX = std::min(cx, sw);
    clipY = std::min(cy, sh);
    clipW = std::min(cw, static_cast<uint16_t>(sw-cx));
    clipH = std::min(ch, static_cast<uint16_t>(sh-cy));
}

void MemDrawSurface::getClipRegion(uint16_t* cx, uint16_t* cy, uint16_t* cw, uint16_t* ch)
{
    if (cx) {
        *cx = clipX;
    }
    if (cy) {
        *cy = clipY;
    }
    if (cw) {
        *cw = clipW;
    }
    if (ch) {
        *ch = clipH;
    }
}

Color MemDrawSurface::readPixel(int32_t x, int32_t y)
{
    if (x < 0  ||  y < 0) {
        return Color::BLACK;
    }
    const auto w = getWidth();
    const auto h = getHeight();
    if (x >= w  ||  y >= h) {
        return Color::BLACK;
    }
    uint16_t rgb565 = reinterpret_cast<uint16_t*>(buf)[y*w + x];
    if (swapEndianness) {
        rgb565 = SwapEndianness(rgb565);
    }
    return Color(rgb565);
}



// ********** LINE CLIPPING **********

// The following is an implementation of the Cohen-Sutherland algorithm, adapted directly from code given on the
// English Wikipedia. It's the same principle used by HAGL.

constexpr int INSIDE = 0b0000;
constexpr int LEFT   = 0b0001;
constexpr int RIGHT  = 0b0010;
constexpr int BOTTOM = 0b0100;
constexpr int TOP    = 0b1000;

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)
// ASSUME THAT xmax, xmin, ymax and ymin are global constants.
int MemDrawSurface::cohenSutherlandComputeOutCode(float x, float y, const LineClipContext& ctx)
{
	int code = INSIDE;          // initialised as being inside of clip window

	if (x < ctx.xMin)           // to the left of clip window
		code |= LEFT;
	else if (x > ctx.xMax)      // to the right of clip window
		code |= RIGHT;
	if (y < ctx.yMin)           // below the clip window
		code |= BOTTOM;
	else if (y > ctx.yMax)      // above the clip window
		code |= TOP;

	return code;
}

bool MemDrawSurface::clipLine(float& x0, float& y0, float& x1, float& y1) const
{
    const LineClipContext ctx = {
        .xMin = static_cast<float>(clipX),
        .yMin = static_cast<float>(clipY),
        .xMax = static_cast<float>(clipX+clipW-1),
        .yMax = static_cast<float>(clipY+clipH-1)
    };

	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	auto outcode0 = cohenSutherlandComputeOutCode(x0, y0, ctx);
	auto outcode1 = cohenSutherlandComputeOutCode(x1, y1, ctx);
	bool accept = false;

	while (true) {
		if (!(outcode0 | outcode1)) {
			// bitwise OR is 0: both points inside window; trivially accept and exit loop
			accept = true;
			break;
		} else if (outcode0 & outcode1) {
			// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
			// or BOTTOM), so both must be outside window; exit loop (accept is false)
			break;
		} else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			float x, y;

			// At least one endpoint is outside the clip rectangle; pick it.
			int outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

			// Now find the intersection point;
			// use formulas:
			//   slope = (y1 - y0) / (x1 - x0)
			//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
			//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
			// No need to worry about divide-by-zero because, in each case, the
			// outcode bit being tested guarantees the denominator is non-zero
			if (outcodeOut & TOP) {           // point is above the clip window
				x = x0 + (x1 - x0) * (ctx.yMax - y0) / (y1 - y0);
				y = ctx.yMax;
			} else if (outcodeOut & BOTTOM) { // point is below the clip window
				x = x0 + (x1 - x0) * (ctx.yMin - y0) / (y1 - y0);
				y = ctx.yMin;
			} else if (outcodeOut & RIGHT) {  // point is to the right of clip window
				y = y0 + (y1 - y0) * (ctx.xMax - x0) / (x1 - x0);
				x = ctx.xMax;
			} else {   // point is to the left of clip window
				y = y0 + (y1 - y0) * (ctx.xMin - x0) / (x1 - x0);
				x = ctx.xMin;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				x0 = x;
				y0 = y;
				outcode0 = cohenSutherlandComputeOutCode(x0, y0, ctx);
			} else {
				x1 = x;
				y1 = y;
				outcode1 = cohenSutherlandComputeOutCode(x1, y1, ctx);
			}
		}
	}

	return accept;
}


}
