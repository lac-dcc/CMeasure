
# CMeasure

[![LAC](/docs/powered.png)](http://lac.dcc.ufmg.br)

[![Build Status](https://travis-ci.org/joemccann/dillinger.svg?branch=master)](https://travis-ci.org/joemccann/dillinger)



CMeasure is C++ tool designed to interact and collect data produced by the
National Instruments DAQ Device [USB-6009](http://www.ni.com/pt-br/support/model.usb-6009.html).

This software can be used to collect and proccess thousands samples of 
instantaneous power measurements per second. By integrating over these
samples, we can calculate the total energy (in Joules) spent by a target
device. 

This repository also contains a simple R script that may be used to produce power charts
out of the output produced by CMeasure.

### Power chart examples:
![Sample 1](/docs/sample1.png)
<br/>
![Sample 2](/docs/sample2.png)
<br/>

## Building CMeasure
---
### Pre-Requisites

1. National Instruments NI-DAQmx driver:

<br/>

** After installing the driver you must restart your computer before proceeding!
[https://www.ni.com/pt-br/support/downloads/drivers/download.ni-daqmx.html#311818](https://www.ni.com/pt-br/support/downloads/drivers/download.ni-daqmx.html#311818)

<br/>
Version 15.5.1 is the one supporting the widest range of Operating Systems, going from Microsoft Windows XP to Windows 10. However, the most recent version, 19, also works very well for modern OSs.

<br/><br/>

2. Microsoft Visual Studio C++ Building Tools
[https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/)

<br/>

** Scroll down to the *All Downloads* section and then to the *Tools for Visual Studio*. You should find a link to the **Build Tools for Visual Studio**. 

<br/>

### Setting paths for headers and libraries
- Open the **Makefile** with you text editor. <br/> ex:
```sh
notepad Makefile
```
- Update the **WIN_NIDAQMXBASE_INCLUDE**, setting it to the include dir inside the National Instrumments install folder. <br/> ex:
```
WIN_NIDAQMXBASE_INCLUDE = C:\Program Files (x86)\National Instruments\NI-DAQ\DAQmx ANSI C Dev\include
```
- Update the **WIN_NILIB**, setting it to the *NIDAQmx.lib* library inside the National Instrumments lib dir. <br/> ex:
```
WIN_NILIB = C:\Program Files (x86)\National Instruments\NI-DAQ\DAQmx ANSI C Dev\lib\msvc\NIDAQmx.lib
```
- Update paths for MS Visual Studio Building Tools:
```
WIN_CPPINCLUDE = C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.22.27905\include
WIN_CRTINCLUDE = C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\ucrt
WIN_SHAREINCLUDE = C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\shared
WIN_CPPLIB = C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.22.27905\lib\onecore\x86\*lib
```

> **WIN_CPPINCLUDE** contains headers for C/C++ standard library and others. <br/>
> **WIN_CRTINCLUDE** contains headers for extra libraries, like string.h, assert.h, tchar.h and others. <br/>
> **WIN_SHAREINCLUDE** contains headers for specific MS Windows libraries, for example the sdkddkver.h header. <br/>
> **WIN_CPPLIB** contains static libraries required for building the tool, for example the libcpmt.lib. <br/>

<br/>

### Building
Just type the command below. 
```sh
nmake 
```
If you already set all the environment variables described above, you should have an executable inside the **bin** dir.
## Release Version

The release version of CMeasure should work out-of-the-box in most computers. It is compiled in a MS Windows 10 machine, targetting the x86 arch. with 32-bits. The NI-DAQmx driver is still **REQUIRED**. So, you must install it before using any release version provided in [https://github.com/lac-dcc/CMeasure/releases](https://github.com/lac-dcc/CMeasure/releases)

## Using CMeasure

The Tool is designed to work with both *[Relay-based](http://cuda.dcc.ufmg.br/jetson/download/User_Manual.pdf)* and *[Trigger-based](http://cuda.dcc.ufmg.br/jetson/download/Manual_Trigger_Circuit.pdf)* circuits.

### Command-line options
```sh
CMeasure.exe RunningType -s Samples -o Output <OPTIONS>
```
   where:
   - RunningType is one of -AP -LV -NT or -TE
> -AP = Triggered by All Points (Continuous Sample Collection) (default) <br/>
> -LV = Triggered by Last Value <br/>
> -NT = No Trigger <br/>
> -TE = Triggered by Total Energy <br/>
> -AT = Triggered Automatically <br/>

**CMeasure + RELAY-based circuit must be executed with the Triggered All Points Options**

- Samples is maximum number of samples to be collected **(Only for NT, LV, AT)**
- Output is the name for the output file           
- OPTIONS may be any of  **-v -sg -sr**

> Use this to set the board operating voltage, <br/>
> SHUNT GAIN of the resistor and sampling rate. <br/>
> (ex: use -v 12 for 12 Volts) <br/>
> (ex: use -sg 10 for a gain of 10)) <br/>
> (ex: use -sr 48000 for rate of 48KHz))" << endl; <br/>

#### Example Usage
```sh
CMeasure.exe -AP -o log.txt -v 5 -sg 10 -sr 12000
```

<br/>


## Target Operating System Preparation
In order to manipulate states, enabling or disabling energy measurement, you must first prepare the operating system of the target embedded device. Let's supose you are using and Odroid Xu4 device with the GNU/Linux Ubuntu 18.04 OS.  
```
echo 31 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio165/direction
echo 0 > /sys/class/gpio/gpio156/value
```
**For detailed information access the [Examples folder](https://github.com/lac-dcc/CMeasure/tree/master/examples)**


## Target Source-code manipulation

**For complete C/C++ or JAVA libraries access the [CodeLibraries folder](https://github.com/lac-dcc/CMeasure/tree/master/CodeLibraries)**

```C
int gpioSetValue (unsigned int gpio, unsigned int value) {
 int fileDescriptor;
 char commandBuffer[64];
  snprintf(commandBuffer, sizeof(commandBuffer),
    "/sys/class/gpio/gpio%d/value", gpio);
  fileDescriptor = open(commandBuffer, 01);
  if (fileDescriptor < 0) {
     char errorBuffer[128] ;
     snprintf(errorBuffer,sizeof(errorBuffer),
       "gpioSetValue unable to open gpio%d",gpio) ;
     perror(errorBuffer);
     return fileDescriptor;
  }
  if (value && write(fileDescriptor, "1", 2) != 2) {
     perror("gpioSetValue");
     return fileDescriptor;
  } else if (write(fileDescriptor, "0", 2) != 2) {
     perror("gpioSetValue");
     return fileDescriptor;
  }
  close(fileDescriptor);
  return 0;
```

```C
int main(int argc, char *argv[]) {
  // Do something without measuring energy
  // We are not measuring energy consumption
  // in foo().
  foo();
  // Start measuring energy:
  gpioSetValue(31, 0);
  // Do something measuring energy, e.g.:
  // Measure the energy consumed by bar();
  bar();
  // Stop measuring energy.
  gpioSetValue(31, 1);
  // Function baz() runs without energy
  // Measurement
  baz();
  return 0;
}
```
NOTE: As illustrated in the code above the energy measurement starts with the value **0** and is disabled with the value **1**.
@Example taken from *cuda.dcc.ufmg.br/jetson*


License
----

GPL


**Free Software, Hell Yeah!**
