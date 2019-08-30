# CMeasure

CMeasure is C++ tool designed to interact and collect data produced by the
National Instruments DAQ Device [USB-6009](http://www.ni.com/pt-br/support/model.usb-6009.html).

This software can be used to collect and proccess thousands samples of 
instantaneous power measurements per second. By integrating over these
samples, we can calculate the total energy (in Joules) spent by a target
device. 

The original code was produced to work in Microsoft Windows OS and must
be compiled with Microsoft Visual Studio. It also requires National Instruments
DAQmx Driver to work.

This repository also contains a simple R script that may be used to produce power charts
out of the output produced by CMeasure.

### Power chart examples:
![Sample 1](/docs/sample1.png)
<br/>
![Sample 2](/docs/sample2.png)
<br/>
![Sample 3](/docs/sample3.png)
<br/>
![Sample 4](/docs/sample4.png)



