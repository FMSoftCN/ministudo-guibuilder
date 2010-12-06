del /Q mstudio.sln.bak
del /Q mstudio.ncb
del /Q mstudio.suo

cd minigui
del /Q Release
del /Q Debug
del /Q *.bak
cd ..

cd mgncs
del /Q Release
del /Q Debug
del /Q *.bak
cd ..

cd mgutils
del /Q Release
del /Q Debug
del /Q *.bak
cd ..

cd mgplus
del /Q Release
del /Q Debug
del /Q *.bak
cd ..

cd guibuilder
del /Q Release
del /Q Debug
del /Q *.bak
cd ..

cd include
del /Q minigui mgncs mgplus mgutils
cd ..

cd lib
del /Q *_msd.lib
del /Q *exp
cd ..

cd bin
del /Q *_msd.dll
del /Q *.pdb
del guibuilder.exe
cd config
del /Q guibuilder-etc
cd ..
cd ..

