#!/usr/bin/env python

import argparse
import nED
import sys
import time

def getModuleConfigParams(module):
    ver = nED.getModuleVersion(module)
    typ = nED.getModuleType(module)
    

def actionList(args=[]):
    """list [modules]               Lists all registered modules"""
    expr = args[0] if args else ".*"
    for module in nED.getModuleNames(expr):
        print module

def actionStatus(args=[]):
    """status modules               Prints modules status"""
    expr = args[0] if args else ".*"
    modules = nED.getModules(expr)

    maxlen = 0
    for module in modules:
        maxlen = max(maxlen, len(module.name))
    for module in modules:
        print "{0}{1}: {2}".format(module.name, " "*(maxlen-len(module.name)), module.status)

    # TODO: print other plugins
    return 0

def actionInfo(args=[]):
    """info modules                 Prints information about each module"""
    expr = args[0] if args else ".*"
    first = True
    for module in nED.getModules(expr):
        if not first:
            print ""
        print "Name     : {0}".format(module.name)
        print "Address  : {0}".format(module.address)
        print "Type     : {0}".format(module.type)
        print "Version  : {0} ({1})".format(module.version, module.date)
        print "Status   : {0}".format(module.status)
        print "Data mode: {0}".format(module.datamode)

        print "Plugin   : {0}".format(module.pluginid)
        first = False

    return 0

def actionSet(args=[]):
    """set modules param value      Set new parameter value for selected modules"""
    if not args:
        print "Select from these modules:"
        lambda x: sys.stdout.write(x), nED.getModuleNames()
        return 2
    if len(args) < 2:
        modules = nED.getModules(args[0])
        if not modules:
            print "ERROR: No such module"
            return 1

        print "Select from these parameters:"
        module = modules.pop()
        for param in module.getParams("config"):
            print param
        return 2
    if len(args) < 3:
        usage("set: missing parameter value")

    expr = args[0] if args else ".*"
    param = args[1]
    value = args[2]

    for module in nED.getModuleNames(expr):
        pv = nED.getPv(module, param)
        units = pv.units if pv.units else ""
        print "Old: {0} {1}={2}".format(module, param, pv.get(as_string=True), pv.units)
        pv.put(value)
        print "New: {0} {1}={2}".format(module, param, pv.get(as_string=True, use_monitor=False), pv.units)
    return 0

def actionGet(args=[]):
    """get modules param            Print current parameter value for selected modules"""
    if len(args) < 2:
        usage("get: missing arguments")
        return 1

    expr = args[0] if args else ".*"
    param = args[1]

    for module in nED.getModuleNames(expr):
        pv = nED.getPv(module, param)
        val = pv.get(as_string=True)
        units = pv.units if pv.units else ""
        print "{0} {1}={2}{3}".format(module, param, val, units)
    return 0

def autocompleteGet(args=[]):
    try:
        if len(args) == 0:
            for module in nED.getModuleNames():
                print module,
        if len(args) == 1:
            module = nED.Module(args[0])
            params = []
            params += module.getParams("status")
            params += module.getParams("config")
            params += module.getParams("counters")
            for param in sorted(params, key=lambda x: x.lower()):
                print param,
    except:
        pass
    return 0

def actionSave(args=[]):
    """save modules name            Save current module configuration as name"""
    if len(args) < 2:
        usage("save: missing parameter value")
        return 1

    expr = args[0] if args else ".*"
    name = args[1]

    for module in nED.getModuleNames(expr):
        nED.getPv(module, "SaveConfigName").put(name)
        nED.getPv(module, "SaveConfigTrig").put(1)
        print "Saved '{0}' configuration as '{1}'".format(module, name)
    return 0

def autocompleteSave(args=[]):
    try:
        modules = nED.getModuleNames()
        if len(args) == 0:
            for module in modules:
                print module
        elif len(args) == 1 and args[0] not in modules:
            for module in nED.getModuleNames(args[0] + "*"):
                print module
    except:
        pass
    return 0

def usage(error=None):
    if error:
        print "ERROR: {0}".format(error)
        print ""
    print "Usage: {0} [options] action [action params]".format(sys.argv[0])
    print ""
    print "Options:"
    print "  --pv-prefix                  nED PV prefix, defaults to {0}".format(nED.getPvPrefix())
    print ""
    print "Actions:"
    for name,cb in globals().iteritems():
        if name.startswith("action"):
            print "  " + cb.__doc__
    return 2

def main():
    args = sys.argv[1:]
    autocomplete = False
    while args:
        arg = args[0]
        if not arg.startswith("-"):
            break
        args = args[1:]
        val = args[0] if len(args) > 0 else None

        if arg == "--autocomplete":
            autocomplete = True
        if arg == "--pv-prefix":
            if not val:
                print "ERROR: --pv-prefix argument requires a value"
                return 1
            nED.setPvPrefix(val)
            args = args[1:]

    actions = []
    for name in globals().keys():
        if name.startswith("action"):
            actions.append( name.replace("action", "").lower() )
    if len(args) == 0:
        if autocomplete:
            for action in actions:
                print action,
            return 0
        return usage()
    if len(args) == 1 and args[0] not in actions and autocomplete:
        for action in actions:
            if action.startswith(args[0]):
                print action
        return 0

    if autocomplete:
        func = "autocomplete" + args[0].title()
    else:
        func = "action" + args[0].title()
    if func not in globals():
        return usage("Invalid action '{0}'".format(args[0]))

    return globals()[func](args[1:])

if __name__ == "__main__":
    main()

