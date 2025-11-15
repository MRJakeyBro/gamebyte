#include <Windows.h>

BOOL isOnScreen(GWindow win, int x, int y, int w, int h) {
	if (win.srcWidth == 0 || win.srcHeight == 0) return true;
	if (x >= win.srcWidth || y >= win.srcHeight) return false;
	if (x + w <= 0 || y + h <= 0) return false;
	return true;
}

void GDrawLine(HDC hdc, GWindow win, int x1, int y1, int x2, int y2, COLORREF color) {
	if (!win.scale) win.scale = 1;

	float minx = x1;
	float maxx = x2;
	if (x2 < minx) {
		minx = x2;
		maxx = x1;
	}

	float miny = y1;
	float maxy = y2;
	if (y2 < miny) {
		miny = y2;
		maxy = y1;
	}

	float rx = maxx - minx;
	float ry = maxy - miny;

	if (!isOnScreen(win, minx, miny, rx, ry)) return;

	x1 *= win.scale;
	y1 *= win.scale;
	x2 *= win.scale;
	y2 *= win.scale;

	x1 += win.xOffset;
	x2 += win.xOffset;

	HPEN pen = CreatePen(PS_SOLID, win.scale, RGB(255, 255, 255));
	SelectObject(hdc, pen);

	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);

	DeleteObject(pen);
}

void GDrawRect(HDC hdc, GWindow win, POINT pos, POINT size, HBRUSH brush) {
	if (!win.scale) win.scale = 1;

	if (!isOnScreen(win, pos.x, pos.y, size.x, size.y)) return;
	
	pos.x *= win.scale;
	pos.y *= win.scale;
	size.x *= win.scale;
	size.y *= win.scale;

	pos.x += win.xOffset;

	RECT rect = { pos.x, pos.y, pos.x + size.x, pos.y + size.y };
	FillRect(hdc, &rect, brush);
}

struct SpriteEntry {
	const wchar_t* path;
	HBITMAP handle;
};

SpriteEntry spriteCache[2048];
int spriteCount = 0;
SpriteEntry fSpriteCache[2048];

HBITMAP GGetSprite(const wchar_t* path, int resource) {
	for (int i = 0; i < 2048; i++) {
		if (i > spriteCount) break;
		if (!_wcsicmp(spriteCache[i].path, path))
			return spriteCache[i].handle;
	}

	HBITMAP hSprite = NULL;

	if (resource == 0) {
		hSprite = (HBITMAP)LoadImageW(
			NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_SHARED
		);
	} 
	else {
		hSprite = (HBITMAP)LoadImageA(
			GetModuleHandle(NULL), MAKEINTRESOURCE(resource), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_SHARED
		);
	}

	if (!hSprite) return spriteCache[0].handle;

	spriteCache[spriteCount++] = { path, hSprite };

	if (spriteCount > 2048) {
		MessageBox(NULL,
			"ERROR: Too many sprites! Reseting cache.",
			"GAMEBYTE : ERROR",
			MB_ICONEXCLAMATION | MB_OK);
		spriteCount = 1;
	}

	return hSprite;
}

HBITMAP flipBitmap(const wchar_t* path, HBITMAP src, BOOL flipH, BOOL flipV) {
	for (int i = 0; i < 2048; i++) {
		if (i > spriteCount) break;
		if (!_wcsicmp(fSpriteCache[i].path, path))
			return fSpriteCache[i].handle;
	}

	int fi = 0;
	
	for (int i = 0; i < 2048; i++) {
		if (i > spriteCount) break;
		if (!_wcsicmp(spriteCache[i].path, path))
			fi = i;
	}
	
	BITMAP bm;
	GetObject(src, sizeof(BITMAP), &bm);

	HDC srcDC = CreateCompatibleDC(NULL);
	HDC memDC = CreateCompatibleDC(NULL);

	HBITMAP oldSrc = (HBITMAP)SelectObject(srcDC, src);
	HBITMAP flipped = CreateCompatibleBitmap(srcDC, bm.bmWidth, bm.bmHeight);
	HBITMAP oldMem = (HBITMAP)SelectObject(memDC, flipped);

	for (int y = 0; y < bm.bmHeight; ++y) {
		int srcY = flipV ? bm.bmHeight - 1 - y : y;
		for (int x = 0; x < bm.bmWidth; ++x) {
			int srcX = flipH ? bm.bmWidth - 1 - x : x;
			COLORREF color = GetPixel(srcDC, srcX, srcY);
			SetPixel(memDC, x, y, color);
		}
	}

	SelectObject(srcDC, oldSrc);
	SelectObject(memDC, oldMem);
	DeleteDC(srcDC);
	DeleteDC(memDC);

	fSpriteCache[fi] = { path, flipped };

	return flipped;
}

void GDrawBitmap(HDC hdc, GWindow win, const wchar_t* path, HBITMAP bmp, GRenderFlags GRF, int x, int y, int w, int h) {
	if (!isOnScreen(win, x, y, w, h)) return;
	if (!win.scale) win.scale = 1;

	x *= win.scale;
	y *= win.scale;
	w *= win.scale;
	h *= win.scale;
	
	x += win.xOffset;

	if (GRF & GRF_FLIPPED) {
		bmp = flipBitmap(path, bmp, true, false);
	}

	HDC mem = CreateCompatibleDC(hdc);
	HBITMAP old = (HBITMAP)SelectObject(mem, bmp);

	BITMAP bm;
	GetObject(bmp, sizeof(BITMAP), &bm);

	int srcW = bm.bmWidth;
	int srcH = bm.bmHeight;

	if (GRF & GRF_TRANSPARENT) {
		GdiTransparentBlt(
			hdc, x, y, w, h,
			mem, 0, 0, srcW, srcH,
			RGB(255, 0, 255));
	}
	else {
		StretchBlt(
			hdc, x, y, w, h,
			mem, 0, 0, srcW, srcH,
			SRCCOPY
		);
	}

	SelectObject(mem, old);
	DeleteDC(mem);
}

void GDrawSprite(HDC hdc, GWindow win, const wchar_t* path, GRenderFlags GRF, int x, int y, int w, int h) {
	if (!isOnScreen(win, x, y, w, h)) return;
	HBITMAP hSprite = GGetSprite(path, 0);
	GDrawBitmap(hdc, win, path, hSprite, GRF, x, y, w, h);
}

void GDrawText(HDC hdc, GWindow win, LPCSTR text, int x, int y) {
	TextOutA(
		hdc,
		x * win.scale,
		y * win.scale,
		text,
		strlen(text)
	);
}

void GDrawBorder(HDC hdc, GWindow win) {
	RECT rect = { 0, 0, win.xOffset, win.height };
	FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

	RECT rect2 = { win.width-win.xOffset, 0, win.width, win.height };
	FillRect(hdc, &rect2, (HBRUSH)GetStockObject(BLACK_BRUSH));
}