rem @echo off

REM Build TypeLib on Windows NT and Chicago systems
REM %1 = file name, including extension
REM %2 = bin path, e.g. c:\msvc\bin
REM %3 = file/project path
REM %4 = include path (opt), e.g. /Ic:\msvc\cdk16\include

rem cd %3
rem if not exist tlb16\nul md tlb16
rem if exist tlb16\%1 del tlb16\%1
rem %2\cl /c /DSS_SPREADOCX /D__MKTYPLIB__ %4 /E %1 >tlb16\%1
rem cd tlb16
rem %2\mktyplib /W0 /nocpp /nologo %4 %1

set FileName=ssocx.odl
set BinPath=e:\msvc15\bin
set ProjPath=d:\dev\ss\src\ssocx
set IncPath=/Ie:\msvc15\cdk16\include

cd %ProjPath%
if not exist tlb16\nul md tlb16
if exist tlb16\%FileName% del tlb16\%FileName%
%BinPath%\cl /c /DSS_SPREADOCX /D__MKTYPLIB__ %IncPath% /E %FileName% >tlb16\%FileName%
cd tlb16
%BinPath%\mktyplib /W0 /nocpp /nologo %IncPath% %FileName%
