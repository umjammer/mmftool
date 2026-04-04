#include <windows.h>
#include <stdio.h>
#include "runtime.h"
#include "smaf.h"
#include "emusmw5.h"
#include "console.h"
#include "cuimode.h"
#include "version.h"

volatile BOOL g_bInterrupted = FALSE;

BOOL WINAPI ConsoleHandler(DWORD dwType)
{
    if (dwType == CTRL_C_EVENT || dwType == CTRL_BREAK_EVENT)
    {
        g_bInterrupted = TRUE;
        return TRUE;
    }
    return FALSE;
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        if (!lstrcmp(argv[1], "-filtering"))
        {
            FilterMode(argc > 2 ? argv[2] : "");
            return 0;
        }
        else if (!lstrcmp(argv[1], "-ctls"))
        {
            ContentTypeLSMode(argc > 2 ? argv[2] : "");
            return 0;
        }
        else
        {
            SMAF* smaf = NULL;
            
            SetConsoleCtrlHandler(ConsoleHandler, TRUE);
            
            if (!EmuInitialize())
            {
                printf("Failed to initialize emulator.\n");
                return 1;
            }

            // Check for environment variable to override master volume
            {
                TCHAR envVol[32];
                if (GetEnvironmentVariable("MMFTOOL_MASTER_VOLUME", envVol, sizeof(envVol)) > 0) {
                    int vol = strtoint(envVol);
                    if (vol >= 0 && vol <= 127) {
                        EmuSetVolume(vol);
                    }
                }
            }

            smaf = LoadSmaf(argv[1]);
            if (!smaf)
            {
                printf("Failed to load SMAF file: %s\n", argv[1]);
                EmuUninitialize();
                return 1;
            }

            printf("Playing: %s (Press Ctrl+C to stop)\n", argv[1]);

            if (EmuSMAFPlay(smaf, 0))
            {
                // Playback loop
                while (EmuStatus() == EMUSTATUS_PLAYING && !g_bInterrupted)
                {
                    Sleep(10);
                }
                EmuSMAFStop();
                
                if (g_bInterrupted)
                {
                    printf("\nPlayback interrupted by user.\n");
                }
            }
            else
            {
                printf("Playback failed.\n");
            }

            FreeSmaf(smaf);
            EmuUninitialize();
            return 0;
        }
    }
    
    printf("Usage: %s <file.mmf>\n", argv[0]);
    printf("       %s -filtering [dir]\n", argv[0]);
    printf("       %s -ctls [dir]\n", argv[0]);
    return 0;
}
