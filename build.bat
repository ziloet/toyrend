@echo off

if not exist "build" (
	mkdir "build"
)

pushd "build"
set compiler_flags=/nologo /W3 /Zi /Fe:main.exe
set linker_flags=/incremental:no /subsystem:windows /WX /opt:icf /opt:ref
set libs_to_link=kernel32.lib user32.lib
call cl %compiler_flags% ../main.c /link %linker_flags% %libs_to_link%
popd
