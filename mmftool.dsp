# Microsoft Developer Studio Project File - Name="mmftool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=mmftool - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "mmftool.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "mmftool.mak" CFG="mmftool - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "mmftool - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "mmftool - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mmftool - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib version.lib comctl32.lib /nologo /subsystem:windows /pdb:"mmftool.pdb" /machine:I386 /nodefaultlib /out:"mmftool.exe" /pdbtype:sept
# SUBTRACT LINK32 /incremental:yes /debug

!ELSEIF  "$(CFG)" == "mmftool - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /Gs999999 /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib version.lib /nologo /subsystem:windows /pdb:"mmftool_debug.pdb" /debug /machine:I386 /nodefaultlib /out:"mmftool_debug.exe" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "mmftool - Win32 Release"
# Name "mmftool - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\src\bit.c
# End Source File
# Begin Source File

SOURCE=.\src\console.c
# End Source File
# Begin Source File

SOURCE=.\src\cuimode.c
# End Source File
# Begin Source File

SOURCE=.\src\detail.c
# End Source File
# Begin Source File

SOURCE=.\src\emusmw5.c
# End Source File
# Begin Source File

SOURCE=.\src\exlayer.c
# End Source File
# Begin Source File

SOURCE=.\src\main.c
# End Source File
# Begin Source File

SOURCE=.\src\proll.c
# End Source File
# Begin Source File

SOURCE=.\src\runtime.c
# End Source File
# Begin Source File

SOURCE=.\src\smaf.c
# End Source File
# Begin Source File

SOURCE=.\src\version.c
# End Source File
# Begin Source File

SOURCE=.\src\voice.c
# End Source File
# End Group
# Begin Group "Header"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\src\bit.h
# End Source File
# Begin Source File

SOURCE=.\src\console.h
# End Source File
# Begin Source File

SOURCE=.\src\cuimode.h
# End Source File
# Begin Source File

SOURCE=.\src\detail.h
# End Source File
# Begin Source File

SOURCE=.\src\emusmw5.h
# End Source File
# Begin Source File

SOURCE=.\src\exlayer.h
# End Source File
# Begin Source File

SOURCE=.\src\main.h
# End Source File
# Begin Source File

SOURCE=.\src\proll.h
# End Source File
# Begin Source File

SOURCE=.\src\runtime.h
# End Source File
# Begin Source File

SOURCE=.\src\smaf.h
# End Source File
# Begin Source File

SOURCE=.\src\version.h
# End Source File
# Begin Source File

SOURCE=.\src\voice.h
# End Source File
# End Group
# Begin Group "Resource"

# PROP Default_Filter "rc;bmp;ico"
# Begin Source File

SOURCE=.\res\logo.bmp
# End Source File
# Begin Source File

SOURCE=.\res\main.ico
# End Source File
# Begin Source File

SOURCE=.\res\maintb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\oldmain.ico
# End Source File
# Begin Source File

SOURCE=.\res\res.rc
# End Source File
# End Group
# Begin Group "Memo"

# PROP Default_Filter "txt"
# Begin Source File

SOURCE=.\memo\FinalTuned.txt
# End Source File
# Begin Source File

SOURCE=.\memo\Kukeiha.txt
# End Source File
# Begin Source File

SOURCE=.\memo\M5Emu2.txt
# End Source File
# Begin Source File

SOURCE=.\memo\M5EmuSw.txt
# End Source File
# Begin Source File

SOURCE=.\memo\MA2exSum.txt
# End Source File
# Begin Source File

SOURCE=.\memo\MA5FM.txt
# End Source File
# Begin Source File

SOURCE=.\memo\MA5PCM.txt
# End Source File
# Begin Source File

SOURCE=.\memo\OpePCM.txt
# End Source File
# Begin Source File

SOURCE=.\memo\OpeSws.txt
# End Source File
# Begin Source File

SOURCE=.\memo\OpPCMsum.txt
# End Source File
# Begin Source File

SOURCE=.\memo\opsw_sum.txt
# End Source File
# Begin Source File

SOURCE=.\memo\pcmattdt.txt
# End Source File
# Begin Source File

SOURCE=.\memo\vm3.txt
# End Source File
# Begin Source File

SOURCE=.\memo\wt.txt
# End Source File
# End Group
# Begin Source File

SOURCE=.\mmftool.txt
# End Source File
# Begin Source File

SOURCE=.\todo.txt
# End Source File
# End Target
# End Project
