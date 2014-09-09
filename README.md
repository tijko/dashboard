dashboard
=========

Emulation of top command line tool

#### Building

    $ make

#### Installation

    $ sudo make install

    $ make clean

#### Uninstall

    $ sudo make uninstall

#### Usage

While dashboard is running the fields can be sorted by using their associated 
keybinding from largest to smallest:

1. `c` - will order the cpuset field, this is the current allowable CPU's.

2. `e` - orders the pte field, this is the amount in kB of page table entries 
         of the process.

3. `m` - the current amount of memory percentage the process is using.

4. `n` - the niceness of the process.

5. `p` - order by the value of the process id.

6. `r` - the processes current rss size in kB.

7. `v` - the processes current virtual memory size in kB. 

8. `o` - current total of bytes read by the process.

9. `i` - current total of bytes written by the process.

###### NOTE:  

In order to obtain and display the i/o stats (READ/WRITE fields in the header 
bar) you will need elavated privileges, run dashboard as root.
