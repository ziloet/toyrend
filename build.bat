@echo off

if not exist "build" (
	mkdir "build"
)

pushd "build"
set compiler_flags=/nologo /W3 /Zi /Fe:toyrend_win32.exe
set linker_flags=/incremental:no /subsystem:windows /WX /opt:icf /opt:ref
set libs_to_link=kernel32.lib user32.lib gdi32.lib
call cl %compiler_flags% ../toyrend_main_win32.c /link %linker_flags% %libs_to_link%
popd
