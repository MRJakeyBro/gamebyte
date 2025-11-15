#include <Windows.h>
#include <stdio.h>
#ifdef STB_VORBIS_IMPLEMENTATION
#include "stb_vorbis.c"
#endif

GSound* GLoadSound(const char* filename, GSoundFormat format) {
	GSound* s = (GSound*)malloc(sizeof(GSound));
	if (!s) return NULL;
	ZeroMemory(s, sizeof(GSound));

	if (format == GSF_WAV) {
		FILE* f = fopen(filename, "rb");
		if (!f) return NULL;

		char id[5] = { 0 };
		DWORD size = 0;

		fread(id, 1, 4, f);
		if (strncmp(id, "RIFF", 4) != 0) { fclose(f); free(s); return NULL; }
		fread(&size, 4, 1, f);
		fread(id, 1, 4, f);
		if (strncmp(id, "WAVE", 4) != 0) { fclose(f); free(s); return NULL; }

		while (!feof(f)) {
			if (fread(id, 1, 4, f) != 4) break;
			fread(&size, 4, 1, f);

			if (strncmp(id, "fmt ", 4) == 0) {
				fread(&s->fmt, 1, size, f);
			}
			else if (strncmp(id, "data", 4) == 0) {
				if (size > MAX_WAV_SIZE) size = MAX_WAV_SIZE;
				s->dataSize = fread(s->data, 1, size, f);
			}
			else {
				fseek(f, size, SEEK_CUR);
			}
		}
		fclose(f);
	}
	else if (format == GSF_OGG) {
#ifdef STB_VORBIS_IMPLEMENTATION
		int channels, sampleRate;
		short* output;
		int samples = stb_vorbis_decode_filename(filename, &channels, &sampleRate, &output);

		if (samples <= 0) {
			free(s);
			return NULL;
		}

		s->fmt.wFormatTag = WAVE_FORMAT_PCM;
		s->fmt.nChannels = channels;
		s->fmt.nSamplesPerSec = sampleRate;
		s->fmt.wBitsPerSample = 16;
		s->fmt.nBlockAlign = channels * (s->fmt.wBitsPerSample / 8);
		s->fmt.nAvgBytesPerSec = s->fmt.nSamplesPerSec * s->fmt.nBlockAlign;

		s->dataSize = samples * s->fmt.nBlockAlign;
		if (s->dataSize > MAX_WAV_SIZE) s->dataSize = MAX_WAV_SIZE;

		memcpy(s->data, output, s->dataSize);
		free(output);
#else
		return NULL;
#endif
	}

	double durationSec = (double)s->dataSize / (s->fmt.nAvgBytesPerSec);
	s->duration = (ULONGLONG)(durationSec * 1000);
	GSetSoundVolume(s, 0.67f);
	return s;
}

DWORD WINAPI GSoundThread(LPVOID param) {
	GSound* snd = (GSound*)param;

	ULONGLONG start = GetTickCount64();

	while (snd->isPlaying) {
		ULONGLONG elapsed = GetTickCount64() - start;
		snd->playTime = elapsed;
		if (elapsed >= snd->duration) {
			Sleep(2);
			snd->playTime = 0;
			snd->isPlaying = false;
			break;
		}
	}

	return 0;
}

void GPlaySound(GSound* s) {
	if (!s || s->isPlaying) return;

	if (waveOutOpen(&s->hWaveOut, WAVE_MAPPER, &s->fmt, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		return;

	ZeroMemory(&s->waveHeader, sizeof(WAVEHDR));
	s->waveHeader.lpData = (LPSTR)s->data;
	s->waveHeader.dwBufferLength = s->dataSize;

	waveOutPrepareHeader(s->hWaveOut, &s->waveHeader, sizeof(WAVEHDR));
	waveOutWrite(s->hWaveOut, &s->waveHeader, sizeof(WAVEHDR));

	s->isPlaying = true;
	s->playTime = 0;

	DWORD dwVolume = s->volume; //0x0000FFFF
	dwVolume += s->volume * 65536; //0xFFFFFFFF

	waveOutSetVolume(
		s->hWaveOut,
		dwVolume
	);

	CreateThread(NULL, 0, GSoundThread, s, 0, NULL);
}

void GStopSound(GSound* s) {
	if (!s || !s->isPlaying || !s->hWaveOut) return;

	waveOutReset(s->hWaveOut);
	waveOutClose(s->hWaveOut);
	s->hWaveOut = NULL;

	s->isPlaying = false;
	s->playTime = 0;
}

void GSetSoundVolume(GSound* s, float v) {
	v *= 65535;
	s->volume = v;

	if (s->isPlaying) {
		DWORD dwVolume = s->volume; //0x0000FFFF
		dwVolume += s->volume * 65536; //0xFFFFFFFF

		waveOutSetVolume(
			s->hWaveOut,
			dwVolume
		);
	}
}

void GFreeSounds(GSound* s1, GSound* s2 = nullptr, GSound* s3 = nullptr, GSound* s4 = nullptr, GSound* s5 = nullptr, GSound* s6 = nullptr, GSound* s7 = nullptr, GSound* s8 = nullptr) {
	if (!s1) return;
	free(s1);
	if (!s2) return;
	free(s2);
	if (!s3) return;
	free(s3);
	if (!s4) return;
	free(s4);
	if (!s5) return;
	free(s5);
	if (!s6) return;
	free(s6);
	if (!s7) return;
	free(s7);
	if (!s8) return;
	free(s8);
}