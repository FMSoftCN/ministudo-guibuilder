REM copy config to guibuilder-etc

mkdir bin\config\guibuilder-etc

xcopy ..\guibuilder\etc bin\config\guibuilder-etc /E /R /EXCLUDE:uncopy-config.txt