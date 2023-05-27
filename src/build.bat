@echo off

set common_compiler_flags= -nologo -Od -W4 -Zi -wd4100 -wd4201 -wd4189 -wd4505
set common_linker_flags= user32.lib Gdi32.lib Winmm.lib

REM figure out why ..\..\build makes compiling in vim funny
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
del *.pdb > NUL 2> NUL
cl %common_compiler_flags% ..\src\win32_software_renderer.cpp -Fmwin32_software_renderer.map /link %common_linker_flags% -incremental:no -opt:ref /PDB:win32_software_renderer%random%.pdb
popd

REM  ---------------------------COMPILER FLAGS------------------------------
REM
REM  -Mtd		creates debug multithreaded exe file
REM  -nolog		suppresses display of sign-on banner??
REM  -Od		disables optimization
REM  -GR		uses the __fastcall calling convention x86 only
REM  -EHa		enable c++ exception handling
REM  -Oi		generates intrinsinc functions
REM  -wd		disables specific warning e.g. -wd4189 disables warning 4189
REM  -W4		sets ouput warning lvl to 4
REM  -D			defines constants and macros
REM  -FC		displays full path of src files passed to cl.exe in diagnostic test??
REM  -Zi		generates complete debugging info
REM 
REM  NOTE to self cant have space
REM 		WRONG:		set CommonCompilerFlags =
REM 		CORRECT:	set CommonCompilerFlags=
REM 
REM 
REM  ---------------------LINKER FLAGS------------------------------
