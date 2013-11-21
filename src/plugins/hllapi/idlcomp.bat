@echo off

midl.exe /I "%WINSDK%\Include\um" /I "%WINSDK%\Include\shared" /cpp_cmd "cl.exe" pw3270.idl
