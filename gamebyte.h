#ifndef GAMEBYTE
#define GAMEBYTE "GB-b1.0.0"
#include <Windows.h>
#include <stdio.h>
#include <iostream>

double dt;

int windowClosed = 0;

//Util
#define MAX_FILE_SIZE 1048576

ULONGLONG input[256];

void GUpdateInput();

typedef struct {
	LPCSTR path;
	LPCSTR data;
	BOOL read;
} GFile;

GFile GReadFile(LPCSTR path);
void GWriteFile(LPCSTR path, LPCSTR data);

typedef struct {
	MSG Msg;
	HWND hwnd;

	char focused;

	int srcWidth;
	int srcHeight;

	int width;
	int height;

	float scale;
	int xOffset;

	char fullscreen;
} GWindow;

GWindow winEmpty = {};

LPCSTR GIntLPCSTR(int value);
LPCSTR GCombineLPCSTR(LPCSTR s1, LPCSTR s2, LPCSTR s3, LPCSTR s4);
LPCWSTR GConvertToLPCWSTR(LPCSTR str);

inline void GHandleMSG(GWindow *win);

GWindow GCreateWindow(HINSTANCE hInstance, LPCSTR name, int width, int height);
void GSetWindowIcon(GWindow* win, int icoIndex);

void GSetWindowScale(GWindow* win, int scale);
void GSetFullScreen(GWindow* win);

inline void GLoop(BOOL(*updateFunc)(), void(*renderFunc)(HDC hdc), GWindow* win, int mscap);

int WINAPI GStart(HINSTANCE hInstance);

//Render
typedef enum {
	GRF_NORMAL = 0,
	GRF_TRANSPARENT = 1,
	GRF_FLIPPED = 2
} GRenderFlags;

void GDrawRect(HDC hdc, GWindow win, POINT pos, POINT size, HBRUSH color);

HBITMAP GGetSprite(const wchar_t* path, int resource);
void GDrawBitmap(HDC hdc, GWindow win, HBITMAP bmp, GRenderFlags GRF, int x, int y, int w, int h);
void GDrawSprite(HDC hdc, GWindow win, const wchar_t* path, GRenderFlags GRF, int x, int y, int w, int h);
void GDrawText(HDC hdc, GWindow win, LPCSTR text, int x, int y);
void GDrawBorder(HDC hdc, GWindow win);

//Sound
#define MAX_WAV_SIZE 33554432

typedef struct {
	WAVEFORMATEX fmt;
	WAVEHDR waveHeader;

	char data[MAX_WAV_SIZE];
	DWORD dataSize;
	ULONGLONG duration;

	char isPlaying;
	ULONGLONG playTime;

	HWAVEOUT hWaveOut;
	int volume;
} GSound;

typedef enum {
	GSF_WAV = 0,
	GSF_OGG = 1
} GSoundFormat;

GSound* GLoadSound(const char* filename, GSoundFormat format);
void GPlaySound(GSound* s);
void GStopSound(GSound* s);
void GSetSoundVolume(GSound* s, float v);
void GFreeSounds(GSound* s1, GSound* s2, GSound* s3, GSound* s4, GSound* s5, GSound* s6, GSound* s7, GSound* s8);

#include "gamebyte_util.cpp"
#include "gamebyte_render.cpp"
#include "gamebyte_sound.cpp"
#endif