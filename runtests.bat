@echo off

setlocal
set RC=0
set PATH=..\lib;%PATH%

:File
if "%1" == "" goto :EndOfFiles
if not exist bin\%1.exe (
        echo bin\%1.exe not found
        set RC=1
) else (
        echo running %1
        bin\%1.exe
        if errorlevel 1 set RC=1
)
shift
goto :File

:EndOfFiles
if %RC% == 1 echo Some tests failed
exit /B %RC%
