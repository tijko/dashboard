dashboard
=========

[![Build Status](https://travis-ci.org/tijko/dashboard.svg?branch=master)](https://travis-ci.org/tijko/dashboard)

Emulation of top command line tool

![ScreenShot](/screenshots/dashboard.jpg)

#### Building

    $ make

#### Installation

    $ sudo make install

    $ make clean

#### Uninstall

    $ sudo make uninstall

#### Usage

While dashboard is running the fields can be sorted in descending order by 
entering on your keyboard the fields matching keybinding:

1.  `c` - The number of cores the process is allowed to run on.

2.  `d` - The total number of open file descriptors.

3.  `e` - The amount in kB of page table entries of the process.

4.  `i` - Current total of bytes input written by the process.

5.  `m` - The current amount of memory percentage the process is using.

6.  `n` - The niceness of the process.

7.  `o` - Current total of bytes output read by the process.

8.  `p` - The process id.

9.  `r` - The current resident set size in kB.

10. `s` - Total number of involuntary context switches.

11. `t` - Processes current thread count.

12. `v` - The processes current virtual memory size in kB. 



###### NOTE:  

In order to obtain and display the i/o stats (READ/WRITE fields in the header 
bar) you will need elavated privileges, run dashboard as root.
