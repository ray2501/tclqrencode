#ifndef _QRENCODE
#define _QRENCODE

#include <tcl.h>

/*
 * Windows needs to know which symbols to export.
 */

#ifdef BUILD_tclqrencode
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif /* BUILD_tclqrencode */

/*
 * Only the _Init function is exported.
 */

EXTERN int	Tclqrencode_Init(Tcl_Interp * interp);


int SETMODE_8BIT (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETCASESENSITIVE (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETKANJI (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETMICRO (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETDPI (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETLEVEL (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETSIZE (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETSTRUCTURED (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETFILETYPE (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETVERSION (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETFOREGROUND (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int SETBACKGROUND (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);
int QRENCODE (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]);

#endif
