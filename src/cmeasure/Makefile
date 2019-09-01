# ----------------------------------------------------------------------
# Visual C++ / GNU MAKE MakeFile for CMeasure module
# ----------------------------------------------------------------------
#
#
# Win32 build commands (must use VC++ 2.0 [32-bit] or later)
#

#
# Windows specific locations for include headers and daqmxbase lib
#
WIN_NIDAQMXBASE_INCLUDE = C:\Program Files (x86)\National Instruments\NI-DAQ\DAQmx ANSI C Dev\include
WIN_NILIB = C:\Program Files (x86)\National Instruments\NI-DAQ\DAQmx ANSI C Dev\lib\msvc\NIDAQmx.lib

WIN_CPPINCLUDE = C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.22.27905\include
WIN_CPPINCLUDE2 = C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\ucrt
WIN_CPPINCLUDE3 = C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\shared
WIN_CPPLIB = C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.22.27905\lib\onecore\x86\*lib
#
# Setup flags for compiling and linking
#
WIN_CL_FLAGS=/EHsc /c /DWIN32 /I"$(WIN_NIDAQMXBASE_INCLUDE)" /I"$(WIN_CPPINCLUDE)" /I"$(WIN_CPPINCLUDE2)" /I"$(WIN_CPPINCLUDE3)"
WIN_LINK_FLAGS=/MACHINE:I386


all:
	if not exist bin mkdir bin obj	
	@SET LIB=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x86;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x86
	cl $(WIN_CL_FLAGS) src\stdafx.cpp /Foobj\stdafx.obj
	cl $(WIN_CL_FLAGS) src\Channel.cpp /Foobj\Channel.obj
	cl $(WIN_CL_FLAGS) src\ControlChannel.cpp /Foobj\ControlChannel.obj
	cl $(WIN_CL_FLAGS) src\Measurement.cpp /Foobj\Measurement.obj
	cl $(WIN_CL_FLAGS) src\Cmeasure.cpp /Foobj\cmeasure.obj	
	link /OUT:bin\CMeasure.exe obj\cmeasure.obj obj\Measurement.obj obj\ControlChannel.obj obj\Channel.obj obj\stdafx.obj "$(WIN_NILIB)"  "$(WIN_CPPLIB)" 


clean:	
	del bin\* obj\*


