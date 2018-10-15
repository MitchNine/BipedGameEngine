@echo off

for /r %%i in (*.obj) do (
	echo %%i
	start meshconvert %%i -cmo -nodds -y
)