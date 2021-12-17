SET EXITCODE=0
call cl65 -o bin/test.bin -C 6502.cfg --cpu 65c02 --no-target-lib asm/test.s
call vasm -Fbin -dotdir ./asm/%1.s -o ./bin/%1.bin
IF ERRORLEVEL 1 GOTO FAIL
call npm start --prefix ./nodeprog COM6 ../bin/%1.bin ff00
IF ERRORLEVEL 1 GOTO FAIL
GOTO SUCCESS

:FAIL
SET EXITCODE=1
:SUCCESS
EXIT /B %EXITCODE%
