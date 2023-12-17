# -*- tcl -*-
# Tcl package index file, version 1.1
#
if {[package vsatisfies [package provide Tcl] 9.0-]} {
    package ifneeded tclqrencode 1.1 \
	    [list load [file join $dir libtcl9tclqrencode1.1.so] [string totitle tclqrencode]]
} else {
    package ifneeded tclqrencode 1.1 \
	    [list load [file join $dir libtclqrencode1.1.so] [string totitle tclqrencode]]
}
