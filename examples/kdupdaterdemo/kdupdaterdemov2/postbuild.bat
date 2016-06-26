rem @echo off
rem parameter order %1 = out of source build, %2 srcrepodir, %3 destrepodir, %4 destdir
if "%1"=="true" ( 
  if EXIST %4\data ( rmdir /s /q %4\data )
  mkdir %3\kdupdaterdemo_win
  copy %2\Updates.xml %3\Updates.xml
  copy %2\kdupdaterdemo_win\UpdateInstructions.xml %3\kdupdaterdemo_win\UpdateInstructions.xml
)
copy %4\kdupdaterdemov2.exe %3\kdupdaterdemo_win\kdupdaterdemo.exe
%4\ufcreator.exe %3\kdupdaterdemo_win
del /f %3\kdupdaterdemo_win\kdupdaterdemo.exe
move %4\..\examples\kdupdaterdemo\kdupdaterdemov2\kdupdaterdemo_win.kvz %3\kdupdaterdemo_win.kvz
del /f %4\kdupdaterdemov2.exe
echo "Script run successfull"