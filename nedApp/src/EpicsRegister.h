/* EpicsRegister.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef EPICS_REGISTER_H
#define EPICS_REGISTER_H

#include <iocsh.h>
#include <epicsExport.h>

#define XSTR(s) STR(s)
#define STR(s) #s
#define EPICS_TYPE_string   iocshArgString
#define EPICS_VAL_string    sval
#define EPICS_CTYPE_string  const char*
#define EPICS_TYPE_int      iocshArgInt
#define EPICS_VAL_int       ival
#define EPICS_CTYPE_int     int

/**
 * Registers any class creation function with EPICS.
 *
 * User must manually add support for registered class into nedSupport.dbd file. For
 * example, append the following line with \<name\> expanded:
 * registrar("register\<name\>")
 *
 * @param[in] name Name of the function available from EPICS, that creates a new instance
 *            of the object. The actual function name is \<name\>Configure.
 * @param[in] class Class to instantiate.
 * @param[in] numargs Number of plugin parameters supported by plugin constructor.
 * @param[in] ... Pairs of (argument name, argument type) parameters. For each parameter
 *            there should be exactly one such pair. Supported argument types are
 *            string, int.
 */
#define EPICS_REGISTER(name, class, numargs, ...) EPICS_REGISTER_##numargs(name, class, __VA_ARGS__)

#define EPICS_REGISTER_1(name, class, arg0name, arg0type) \
static const iocshArg initArg0 = { arg0name, EPICS_TYPE_##arg0type}; \
static const iocshArg * const initArgs[] = {&initArg0}; \
static const iocshFuncDef initFuncDef = { XSTR(init ## name), 1, initArgs}; \
extern "C" { \
    int init##name(EPICS_CTYPE_##arg0type arg0) { new class(arg0); return 0; } \
    static void initCallFunc(const iocshArgBuf *args) { init##name(args[0].EPICS_VAL_##arg0type); } \
    static void register##name(void) { iocshRegister(&initFuncDef,initCallFunc); } \
    epicsExportRegistrar(register##name); \
}

#define EPICS_REGISTER_2(name, class, arg0name, arg0type, arg1name, arg1type) \
static const iocshArg initArg0 = { arg0name, EPICS_TYPE_##arg0type}; \
static const iocshArg initArg1 = { arg1name, EPICS_TYPE_##arg1type}; \
static const iocshArg * const initArgs[] = {&initArg0,&initArg1}; \
static const iocshFuncDef initFuncDef = { XSTR(init ## name), 2, initArgs}; \
extern "C" { \
    int init##name(EPICS_CTYPE_##arg0type arg0, EPICS_CTYPE_##arg1type arg1) { new class(arg0, arg1); return 0; } \
    static void initCallFunc(const iocshArgBuf *args) { init##name(args[0].EPICS_VAL_##arg0type, args[1].EPICS_VAL_##arg1type); } \
    static void register##name(void) { iocshRegister(&initFuncDef,initCallFunc); } \
    epicsExportRegistrar(register##name); \
}

#define EPICS_REGISTER_3(name, class, arg0name, arg0type, arg1name, arg1type, arg2name, arg2type) \
static const iocshArg initArg0 = { arg0name, EPICS_TYPE_##arg0type}; \
static const iocshArg initArg1 = { arg1name, EPICS_TYPE_##arg1type}; \
static const iocshArg initArg2 = { arg2name, EPICS_TYPE_##arg2type}; \
static const iocshArg * const initArgs[] = {&initArg0,&initArg1,&initArg2}; \
static const iocshFuncDef initFuncDef = { XSTR(init ## name), 3, initArgs}; \
extern "C" { \
    int init##name(EPICS_CTYPE_##arg0type arg0, EPICS_CTYPE_##arg1type arg1, EPICS_CTYPE_##arg2type arg2) { new class(arg0, arg1, arg2); return 0; } \
    static void initCallFunc(const iocshArgBuf *args) { init##name(args[0].EPICS_VAL_##arg0type, args[1].EPICS_VAL_##arg1type, args[2].EPICS_VAL_##arg2type); } \
    static void register##name(void) { iocshRegister(&initFuncDef,initCallFunc); } \
    epicsExportRegistrar(register##name); \
}

#define EPICS_REGISTER_4(name, class, arg0name, arg0type, arg1name, arg1type, arg2name, arg2type, arg3name, arg3type) \
static const iocshArg initArg0 = { arg0name, EPICS_TYPE_##arg0type}; \
static const iocshArg initArg1 = { arg1name, EPICS_TYPE_##arg1type}; \
static const iocshArg initArg2 = { arg2name, EPICS_TYPE_##arg2type}; \
static const iocshArg initArg3 = { arg3name, EPICS_TYPE_##arg3type}; \
static const iocshArg * const initArgs[] = {&initArg0,&initArg1,&initArg2,&initArg3}; \
static const iocshFuncDef initFuncDef = { XSTR(init##name), 4, initArgs}; \
extern "C" { \
    int init##name(EPICS_CTYPE_##arg0type arg0, EPICS_CTYPE_##arg1type arg1, EPICS_CTYPE_##arg2type arg2, EPICS_CTYPE_##arg3type arg3) { new class(arg0, arg1, arg2, arg3); return 0; } \
    static void initCallFunc(const iocshArgBuf *args) { init##name(args[0].EPICS_VAL_##arg0type, args[1].EPICS_VAL_##arg1type, args[2].EPICS_VAL_##arg2type, args[3].EPICS_VAL_##arg3type); } \
    static void register##name(void) { iocshRegister(&initFuncDef,initCallFunc); } \
    epicsExportRegistrar(register##name); \
}

#define EPICS_REGISTER_5(name, class, arg0name, arg0type, arg1name, arg1type, arg2name, arg2type, arg3name, arg3type, arg4name, arg4type) \
static const iocshArg initArg0 = { arg0name, EPICS_TYPE_##arg0type}; \
static const iocshArg initArg1 = { arg1name, EPICS_TYPE_##arg1type}; \
static const iocshArg initArg2 = { arg2name, EPICS_TYPE_##arg2type}; \
static const iocshArg initArg3 = { arg3name, EPICS_TYPE_##arg3type}; \
static const iocshArg initArg4 = { arg4name, EPICS_TYPE_##arg4type}; \
static const iocshArg * const initArgs[] = {&initArg0,&initArg1,&initArg2,&initArg3,&initArg4}; \
static const iocshFuncDef initFuncDef = { XSTR(init ## name), 5, initArgs}; \
extern "C" { \
    int init##name(EPICS_CTYPE_##arg0type arg0, EPICS_CTYPE_##arg1type arg1, EPICS_CTYPE_##arg2type arg2, EPICS_CTYPE_##arg3type arg3, EPICS_CTYPE_##arg4type arg4) { new class(arg0, arg1, arg2, arg3, arg4); return 0; } \
    static void initCallFunc(const iocshArgBuf *args) { init##name(args[0].EPICS_VAL_##arg0type, args[1].EPICS_VAL_##arg1type, args[2].EPICS_VAL_##arg2type, args[3].EPICS_VAL_##arg3type, args[4].EPICS_VAL_##arg4type); } \
    static void register##name(void) { iocshRegister(&initFuncDef,initCallFunc); } \
    epicsExportRegistrar(register##name); \
}

#define EPICS_REGISTER_6(name, class, arg0name, arg0type, arg1name, arg1type, arg2name, arg2type, arg3name, arg3type, arg4name, arg4type, arg5name, arg5type) \
static const iocshArg initArg0 = { arg0name, EPICS_TYPE_##arg0type}; \
static const iocshArg initArg1 = { arg1name, EPICS_TYPE_##arg1type}; \
static const iocshArg initArg2 = { arg2name, EPICS_TYPE_##arg2type}; \
static const iocshArg initArg3 = { arg3name, EPICS_TYPE_##arg3type}; \
static const iocshArg initArg4 = { arg4name, EPICS_TYPE_##arg4type}; \
static const iocshArg initArg5 = { arg5name, EPICS_TYPE_##arg5type}; \
static const iocshArg * const initArgs[] = {&initArg0,&initArg1,&initArg2,&initArg3,&initArg4,&initArg5}; \
static const iocshFuncDef initFuncDef = { XSTR(init ## name), 6, initArgs}; \
extern "C" { \
    int init##name(EPICS_CTYPE_##arg0type arg0, EPICS_CTYPE_##arg1type arg1, EPICS_CTYPE_##arg2type arg2, EPICS_CTYPE_##arg3type arg3, EPICS_CTYPE_##arg4type arg4, EPICS_CTYPE_##arg5type arg5) { new class(arg0, arg1, arg2, arg3, arg4, arg5); return 0; } \
    static void initCallFunc(const iocshArgBuf *args) { init##name(args[0].EPICS_VAL_##arg0type, args[1].EPICS_VAL_##arg1type, args[2].EPICS_VAL_##arg2type, args[3].EPICS_VAL_##arg3type, args[4].EPICS_VAL_##arg4type, args[5].EPICS_VAL_##arg5type); } \
    static void register##name(void) { iocshRegister(&initFuncDef,initCallFunc); } \
    epicsExportRegistrar(register##name); \
}

#endif // EPICS_REGISTER_H
