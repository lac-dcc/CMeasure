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
#
# Setup flags for compiling and linking
#
WIN_CL_FLAGS=/EHsc /c /DWIN32 /I"$(WIN_NIDAQMXBASE_INCLUDE)"
WIN_LINK_FLAGS=/MACHINE:I386


all:
	mkdir -p bin obj
	cl $(WIN_CL_FLAGS) src\stdafx.cpp /Foobj\stdafx.obj
	cl $(WIN_CL_FLAGS) src\measurements.cpp /Foobj\measurements.obj
	cl $(WIN_CL_FLAGS) src\Cmeasure.cpp /Foobj\cmeasure.obj
	link /OUT:bin\CMeasure.exe obj\cmeasure.obj obj\measurements.obj obj\stdafx.obj "$(WIN_NILIB)"


clean:	
	del bin\* obj\*


