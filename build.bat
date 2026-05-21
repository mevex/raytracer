@echo off

set warningsOptions=-WX -W4 -wd4100 -wd4189 -wd4239 -wd4201 -wd4505 -wd4702 -wd4700
set compilerFlags=-O2i -Zi -nologo -FC -MT -fp:fast -GR- %warningsOptions%

pushd \build

cl %compilerFlags% ..\code\main.cpp -link -nologo -opt:ref -incremental:no

popd

REM -Fe[name] is the compiler flag to rename the executable
REM -Ox instead of -Od for the optimized build