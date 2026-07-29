/*==========================================================================
 *
 *  File:       wave.c (stub implementation)
 *  Content:    Stub for DirectSound wave file loading
 *
 *  Note:       Sound functionality disabled - original wave.c file missing
 *
 ***************************************************************************/

#include <windows.h>
#include <mmsystem.h>
#include "WAVE.H"

#ifdef __cplusplus
extern "C"
{
#endif

/* Stub implementation - returns error to disable sound loading */
int WaveLoadFile(const char *pszFileName,
                 UINT *pnSize,
                 DWORD *pdwSamples,
                 WAVEFORMATEX **ppWaveFormat,
                 BYTE **ppWaveData)
{
    /* Return error code to indicate file cannot be loaded */
    /* This will cause GetSoundBuffer() to return NULL, disabling sound */
    return ER_CANNOTOPEN;
}

/* Stub implementations for other wave functions */
int WaveOpenFile(const char *pszFileName,
                 HMMIO *phmmioIn,
                 WAVEFORMATEX **ppWaveFormat,
                 MMCKINFO *pckInRIFF)
{
    return ER_CANNOTOPEN;
}

int WaveStartDataRead(HMMIO *phmmioIn,
                      MMCKINFO *pckIn,
                      MMCKINFO *pckInRIFF)
{
    return ER_CANNOTREAD;
}

int WaveReadFile(HMMIO hmmioIn,
                 UINT cbRead,
                 BYTE *pbDest,
                 MMCKINFO *pckIn,
                 UINT *pcbActualRead)
{
    return ER_CANNOTREAD;
}

int WaveCloseReadFile(HMMIO *phmmio,
                      WAVEFORMATEX **ppWaveFormat)
{
    return 0;
}

int WaveCreateFile(const char *pszFileName,
                   HMMIO *phmmioOut,
                   WAVEFORMATEX *pWaveFormat,
                   MMCKINFO *pckOut,
                   MMCKINFO *pckOutRIFF)
{
    return ER_CANNOTOPEN;
}

int WaveStartDataWrite(HMMIO *phmmioOut,
                       MMCKINFO *pckOut,
                       MMIOINFO *pmmioinfoOut)
{
    return ER_CANNOTWRITE;
}

int WaveWriteFile(HMMIO hmmioOut,
                  UINT cbWrite,
                  BYTE *pbSrc,
                  MMCKINFO *pckOut,
                  UINT *pcbActualWrite,
                  MMIOINFO *pmmioinfoOut)
{
    return ER_CANNOTWRITE;
}

int WaveCloseWriteFile(HMMIO *phmmioOut,
                       MMCKINFO *pckOut,
                       MMCKINFO *pckOutRIFF,
                       MMIOINFO *pmmioinfoOut,
                       DWORD cSamples)
{
    return 0;
}

int WaveSaveFile(const char *pszFileName,
                 UINT cbSize,
                 DWORD cSamples,
                 WAVEFORMATEX *pWaveFormat,
                 BYTE *pbData)
{
    return ER_CANNOTWRITE;
}

int WaveCopyUselessChunks(HMMIO *phmmioIn,
                          MMCKINFO *pckIn,
                          MMCKINFO *pckInRIFF,
                          HMMIO *phmmioOut,
                          MMCKINFO *pckOut,
                          MMCKINFO *pckOutRIFF)
{
    return 0;
}

BOOL riffCopyChunk(HMMIO hmmioSrc,
                   HMMIO hmmioDst,
                   const LPMMCKINFO lpck)
{
    return FALSE;
}

#ifdef __cplusplus
}
#endif
