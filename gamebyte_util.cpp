#include <Windows.h>
#include <stdio.h>
#include <iostream>

void GUpdateInput() {
	for (unsigned short int i = 0; i < 256; i++) {
		if (GetKeyState(i) & 0x8000) {
			input[i]++;
		}
		else {
			input[i] = 0;
		}
	}
}

GFile GReadFile(LPCSTR path) {
	GFile gfile;
	FILE* file = fopen(path, "rb");
	if (!file) {
		return {};
	}
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (size == -1) {
		fclose(file);
		return {};
	}
	char* buffer = new char(size + 1);
	size_t byteRead = fread(buffer, 1, size, file);
	fclose(file);
	if (byteRead != size) {
		delete[] buffer;
		return {};
	}
	buffer[size] = '\0';
	gfile.path = path;
	gfile.data = static_cast<LPCSTR>(buffer);
	gfile.read = true;
	delete[] buffer;
	return gfile;
}

void GWriteFile(LPCSTR path, LPCSTR data) {
    FILE* file = fopen(path, "w");

    if (!file) {
        return;
    }

    fputs(data, file);
    fclose(file);
}

LPCSTR* GSplitLPCSTR(LPCSTR str, char splitter) {
	int count = 1;
	for (int i = 0; str[i]; i++) if (str[i] == splitter) count++;

	LPCSTR* result = new LPCSTR[count];
	int start = 0, index = 0;
	int len = strlen(str);
	for (int i = 0; i <= len; i++) {
		if (str[i] == splitter || str[i] == '\0') {
			int partLen = i - start;
			char* part = new char[partLen + 1];
			memcpy(part, &str[start], partLen);
			part[partLen] = '\0';
			result[index++] = part;
			start = i + 1;
		}
	}
	
	return result;
}

LPCSTR GIntLPCSTR(int value) {
	char* buffer = (char*)malloc(12);
	if (!buffer) return nullptr;

	char temp[12];
	int i = 0, j = 0;
	int negative = 0;

	if (value == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return buffer;
	}

	if (value < 0) {
		negative = 1;
		unsigned int uval = (unsigned int)(-(long long)value);
		value = (int)uval;
	}

	while (value > 0) {
		temp[i++] = (value % 10) + '0';
		value /= 10;
	}

	if (negative)
		temp[i++] = '-';

	while (i > 0) {
		buffer[j++] = temp[--i];
	}

	buffer[j] = '\0';
	return buffer; // Caller must free this memory
}

LPCSTR GCombineLPCSTR(LPCSTR s1, LPCSTR s2 = nullptr, LPCSTR s3 = nullptr, LPCSTR s4 = nullptr) {
	size_t len = 0;
	if (s1) len += strlen(s1);
	if (s2) len += strlen(s2);
	if (s3) len += strlen(s3);
	if (s4) len += strlen(s4);

	char* result = (char*)malloc(len + 1);
	if (!result) return nullptr;

	result[0] = '\0';

	if (s1) strcat(result, s1);
	if (s2) strcat(result, s2);
	if (s3) strcat(result, s3);
	if (s4) strcat(result, s4);

	return result;
}

LPCWSTR GConvertToLPCWSTR(LPCSTR str) {
	if (!str) return L"";

	size_t len = strlen(str) + 1;
	wchar_t* result = (wchar_t*)malloc(len * sizeof(wchar_t));
	if (!result) return L"";

	for (size_t i = 0; i < len; ++i) {
		result[i] = (wchar_t)(unsigned char)str[i];
	}

	return result;
}

__attribute__((always_inline)) inline void GHandleMSG(GWindow *win) {
	if (PeekMessage(&win->Msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&win->Msg);
		DispatchMessage(&win->Msg);
	}
}

LRESULT CALLBACK windowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
		case WM_CLOSE:
			windowClosed += 1;
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_MOVE:
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

GWindow GCreateWindow(HINSTANCE hInstance, LPCSTR name, int width, int height) {
	GWindow winClass = GWindow();

	winClass.srcWidth = width;
	winClass.srcHeight = height;

	winClass.width = width;
	winClass.height = height;
	
	hInstance = GetModuleHandle(NULL);
	
	WNDCLASSEX wc;
	winClass.Msg;

	wc.lpszClassName = "GWINDOW";
	wc.lpfnWndProc = windowProcedure;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		return winClass;
	}

	winClass.hwnd = CreateWindow(
		wc.lpszClassName,
		name,
		(WS_OVERLAPPEDWINDOW | WS_SYSMENU) & ~WS_SIZEBOX,
		200,
		200,
		width,
		height,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (winClass.hwnd == NULL) {
		return winClass;
	}

	ShowWindow(winClass.hwnd, SW_SHOW);
	UpdateWindow(winClass.hwnd);

	winClass.scale = 1;
	winClass.xOffset = 0;

	return winClass;
}

void GSetWindowIcon(GWindow* win, int icoIndex) {
	if (icoIndex <= 0) return;

	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(icoIndex));
	if (hIcon) {
		SendMessage(win->hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessage(win->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	}
}

void GSetWindowScale(GWindow* win, int scale) {
	SetWindowLong(win->hwnd, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_SYSMENU) & ~WS_SIZEBOX);
	win->scale = scale;
	//SetWindowPos(win->hwnd, HWND_TOP, 0, 0, win->srcWidth * scale, win->srcHeight * scale, SWP_SHOWWINDOW);
	win->xOffset = 0;
	win->fullscreen = 0;
	win->width = win->srcWidth * scale;
	win->height = win->srcHeight * scale;

	int frameX = GetSystemMetrics(SM_CXSIZEFRAME);
	int frameY = GetSystemMetrics(SM_CYSIZEFRAME);
	int caption = GetSystemMetrics(SM_CYCAPTION);

	int outerW = win->width + frameX - 2;
	int outerH = win->height + frameY - 2 + caption;

	SetWindowPos(win->hwnd, HWND_TOP, 0, 0,
		outerW, outerH, SWP_SHOWWINDOW);
}

void GSetFullScreen(GWindow* win) {
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	SetWindowLong(win->hwnd, GWL_STYLE, WS_POPUP);
	SetWindowPos(win->hwnd, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_SHOWWINDOW);

	win->scale = (float)screenHeight / (float)win->srcHeight;

	int scaledWidth = (int)(win->srcWidth * win->scale);
	win->xOffset = (screenWidth - scaledWidth) / 2;

	win->width = screenWidth;
	win->height = screenHeight;

	win->fullscreen = 1;
}

void GAttachConsole() {
	if (!AttachConsole(ATTACH_PARENT_PROCESS)) AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
}

__attribute__((always_inline)) inline void GLoop(BOOL(*updateFunc)(), void(*renderFunc)(HDC hdc), GWindow* win, int mscap) {
	HDC hdc = GetDC(win->hwnd);
	HDC hdcM = CreateCompatibleDC(hdc);
	HBITMAP hbm = CreateCompatibleBitmap(hdc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	HBITMAP old = (HBITMAP)SelectObject(hdcM, hbm);

	while (true) {
		int msc = mscap;

		if ((!win->focused) && msc < 33) msc = 33;

		ULONGLONG startTime = GetTickCount64();

		win->focused = (GetForegroundWindow() == win->hwnd);

		GHandleMSG(win);
		GUpdateInput();

		if (!updateFunc()) {
			break;
		}

		HDC hdc = GetDC(win->hwnd);

		SetStretchBltMode(hdcM, COLORONCOLOR);
		renderFunc(hdcM);

		BitBlt(hdc, 0, 0, win->width, win->height, hdcM, 0, 0, SRCCOPY);
		ReleaseDC(win->hwnd, hdc);

		ULONGLONG elapsed = GetTickCount64() - startTime;

		if (elapsed < mscap) {
			ULONGLONG wait = mscap - elapsed;
			Sleep(wait);
			elapsed = GetTickCount64() - startTime;
		}

		dt = elapsed;
		dt /= 1000.f;
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	GGetSprite(L"missing", 2);
	return GStart(hInstance);
}