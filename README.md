Libqrencode
=====

Libqrencode is a C library for encoding data in a QR Code symbol, a kind of 2D 
symbology that can be scanned by handy terminals such as a mobile phone with 
CCD.

Download libqrencode from: 
[http://fukuchi.org/works/qrencode/index.html.en](http://fukuchi.org/works/qrencode/index.html.en)


Tclqrencode commands and variables
=====

Tclqrencode is using libqrencode to encode string to be a EPS/PNG/SVG/XPM file.

Implement Tcl commands:  
::qrencode::set8bit_mode  
::qrencode::setcasesensitive  
::qrencode::setkanji  
::qrencode::setmicro  
::qrencode::setdpi  
::qrencode::setlevel  
::qrencode::setsize  
::qrencode::setstructured  
::qrencode::setfiletype  
::qrencode::setversion  
::qrencode::setforeground  
::qrencode::setbackground  
::qrencode::encode  


Install
=====

## UNIX BUILD

Build Linux version please install Development files for
libpng (for example, libpng-devel for openSUSE) first.

Then:  
./configure  
make  
sudo make install


## WINDOWS BUILD

The recommended method to build extensions under windows is to use the
MSYS2 + MinGW-w64 build process. This provides a Unix-style build while
generating native Windows binaries. Using the MSYS2 + MinGW-w64 build tools
means that you can use the same configure script as per the Unix build
to create a Makefile.

User needs to install MinGW-w64 libpng package before to build this
extension.


Example
=====

Output to file

    package require tclqrencode

    ::qrencode::setmicro 0
    ::qrencode::setsize  5
    ::qrencode::setlevel 2
    ::qrencode::setdpi   720
    ::qrencode::setstructured  0
    ::qrencode::setkanji 0
    ::qrencode::set8bit_mode 1
    ::qrencode::setfiletype png
    ::qrencode::setversion 1
    ::qrencode::setforeground  000000
    ::qrencode::setbackground  fff9ff

    ::qrencode::encode https://github.com/ray2501/tclqrencode tclqrencode.png

Output to stdout

    package require tclqrencode

    ::qrencode::setmicro 0
    ::qrencode::setsize  5
    ::qrencode::setlevel 2
    ::qrencode::setdpi   720
    ::qrencode::setstructured  0
    ::qrencode::setkanji 0
    ::qrencode::set8bit_mode 1
    ::qrencode::setfiletype png
    ::qrencode::setversion 1
    ::qrencode::setforeground  000000
    ::qrencode::setbackground  fff9ff

    ::qrencode::encode https://github.com/ray2501/tclqrencode -
