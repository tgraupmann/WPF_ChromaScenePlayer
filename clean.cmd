del /s /q "bin"
del /s /q "obj"
del /s /q "Debug"
del /s /q "x64"
del /s /q "DLL_ChromaScenePlayer\Debug"
del /s /q "DLL_ChromaScenePlayer\Release"
del /s /q "DLL_ChromaScenePlayer\x64"

rmdir /s /q "bin"
rmdir /s /q "obj"
rmdir /s /q "Debug"
rmdir /s /q "x64"
rmdir /s /q "DLL_ChromaScenePlayer\Debug"
rmdir /s /q "DLL_ChromaScenePlayer\Release"
rmdir /s /q "DLL_ChromaScenePlayer\x64"

xcopy /D /E /C /I /F /Y Animations bin\Debug\Animations
xcopy /D /E /C /I /F /Y Animations bin\Release\Animations
