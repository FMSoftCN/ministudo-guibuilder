REM copy include and lib

mkdir include\minigui
mkdir include\mgutils
mkdir include\mgplus
mkdir include\mgncs

xcopy ..\minigui\include include\minigui /E /R /EXCLUDE:uncopy-include.txt
copy minigui\mgconfig.h include\minigui

xcopy ..\mgutils\include include\mgutils /E /R /EXCLUDE:uncopy-include.txt

xcopy ..\mgplus\include include\mgplus /E /R /EXCLUDE:uncopy-include.txt
copy mgplus\mgplusconfig.h include\mgplus

xcopy ..\mgncs\include include\mgncs /E /R /EXCLUDE:uncopy-include.txt
copy mgncs\mgncsconfig.h include\mgncs