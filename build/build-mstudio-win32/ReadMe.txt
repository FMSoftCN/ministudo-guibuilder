How to build the guibuilder

Files:
	mstudio.sln : the solution file of VS2005
	minigui\ mgutils\ mgplus\ mgncs\ guibuilder : include the .vcprj file and the config.h files
	include\ :  the depend include files and the output dir of minigui, mgutils ... etc
	lib\ : the depend libraries and the output library dir of minigui, ... etc, only the import file(.lib)
	bin\ : the depend libraries and the output dir, only include the .dll and .exe files
	copy-include.bat : script for copy minigui, mgutils, mgplus and mgncs's include file into include dir
	uncopy-include.txt : the file list exclude by copy-include.bat
	copy-config.bat : script for copy guibuilder etc into bin/config
	uncopy-config.txt : the file list exclude by copy-config.bat
	clean.bat : the script clean the solution

Build:
	1. prepare the source code:
		goto the ../mstudio-build
		donwload the sources by svn, the directories must be "minigui", "mgncs", "mgutils", "mgplus" and "guibuilder"
	2. run copy-include.bat to prepare the include file
	3. open mstudio.sln by VS2005, and build all solution
	4. run copy-config.bat to prepare the guibuilder config files
	5. OK

By the way, the minigui-res is not include in bin/config, you should copy the minigui resources from other place

!!The mstudio.sln use the source files by releative path : ../<component-name>", so, you must put the right source code.


