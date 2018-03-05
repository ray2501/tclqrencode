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


int SETMODE_8BIT _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETCASESENSITIVE _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETKANJI _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETMICRO _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETDPI _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETLEVEL _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETSIZE _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETSTRUCTURED _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETFILETYPE _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETVERSION _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETFOREGROUND _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int SETBACKGROUND _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));
int QRENCODE _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST obj[]));

#endif
