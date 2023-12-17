#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tqrencode.h"


/*
 *----------------------------------------------------------------------
 *
 * Tclqrencode_Init --
 *
 *	Initialize the new package. 
 *
 * Results:
 *	A standard Tcl result
 *
 *----------------------------------------------------------------------
 */

int
Tclqrencode_Init(Tcl_Interp *interp)
{
    if (Tcl_InitStubs(interp, "8.4", 0) == NULL) {
	return TCL_ERROR;
    }
    if (Tcl_PkgRequire(interp, "Tcl", "8.4", 0) == NULL) {
	return TCL_ERROR;
    }
    if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) {
	return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, "qrencode::set8bit_mode", SETMODE_8BIT, (ClientData) NULL, NULL);    
    Tcl_CreateObjCommand(interp, "qrencode::setcasesensitive", SETCASESENSITIVE, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, "qrencode::setkanji", SETKANJI, (ClientData) NULL, NULL);    
    Tcl_CreateObjCommand(interp, "qrencode::setmicro", SETMICRO, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, "qrencode::setdpi", SETDPI, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, "qrencode::setlevel", SETLEVEL, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, "qrencode::setsize", SETSIZE, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, "qrencode::setstructured", SETSTRUCTURED, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, "qrencode::setfiletype", SETFILETYPE, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, "qrencode::setversion", SETVERSION, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, "qrencode::setforeground", SETFOREGROUND, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, "qrencode::setbackground", SETBACKGROUND, (ClientData) NULL, NULL);
    
    Tcl_CreateObjCommand(interp, "qrencode::encode", QRENCODE, (ClientData) NULL, NULL);

    return TCL_OK;
}
