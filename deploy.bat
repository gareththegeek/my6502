SET EXITCODE=0
call cl65 -o bin/rom.bin -C 6502.cfg -t bbc --cpu 65c02 --no-target-lib asm/main.s asm/led.s asm/maths.s
IF ERRORLEVEL 1 GOTO FAIL
call npm start --prefix nodeprog COM6 ../bin/rom.bin f000
IF ERRORLEVEL 1 GOTO FAIL
GOTO SUCCESS

:FAIL
SET EXITCODE=1
:SUCCESS
EXIT /B %EXITCODE%
