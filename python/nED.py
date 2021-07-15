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
        print "{0}{1}: {2}".format(module.name, " "*(maxlen-len(module.name)), module.Status)

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
        print "Address  : {0}".format(module.HwId)
        print "Type     : {0}".format(module.HwType)
        print "Version  : {0} ({1})".format(module.FwVersion, module.FwDate)
        print "Status   : {0}".format(module.Status)
        print "Data mode: {0}".format(module.OutputMode)

        print "Plugin   : {0}".format(module.PluginId)
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
    """get modules param            Print parameter value for selected modules"""
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

def actionMonitor(args=[]):
    """monitor modules param        Monitor parameter value for selected modules"""
    if len(args) < 2:
        usage("save: missing parameter value")
        return 1

    expr = args[0] if args else ".*"
    param = args[1]

    pvs = []
    for module in nED.getModules(expr):
        pv = module.getPv(param)
        units = pv.units if pv.units else ""
        pvs.append( [pv, None, units, module.name] )

    # Do 1Hz loops, this could be improved with real CA monitor
    # but most nED PVs update at max 1Hz rate anyway
    while True:
        for pv in pvs:
            new_val = pv[0].get(as_string=True)
            if new_val != pv[1]:
                print "{0} {1}={2}{3}".format(pv[3], param, new_val, pv[2])
                pv[1] = new_val
        time.sleep(1)
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
    """save modules name            Save modules configuration as name"""
    if len(args) < 2:
        usage("save: missing parameter value")
        return 1

    expr = args[0] if args else ".*"
    name = args[1]

    for module in nED.getModules(expr):
        try:
            module.saveConfig(name)
            print "Saved '{0}' current configuration as '{1}'".format(module.name, name)
        except UserWarning, e:
            print e
    return 0

def actionRestore(args=[]):
    """restore modules name         Restore modules configuration from name"""
    if len(args) < 2:
        usage("save: missing parameter value")
        return 1

    expr = args[0] if args else ".*"
    name = args[1]

    for module in nED.getModuleNames(expr):
        try:
            module.restoreConfig(name)
            print "Restored '{0}' configuration '{1}'".format(module, name)
        except UserWarning, e:
            print e
    return 0

def actionCompare(args=[]):
    """compare modules name -d      Compare modules current configuration to saved one, only show diff if -d"""
    if len(args) < 2:
        usage("save: missing parameter name")
        return 1

    expr = args[0] if args else ".*"
    name = args[1]
    show_all = False if (len(args) > 2 and args[2]=="-d") else True

    for module in nED.getModules(expr):
        for param in module.getParams("Config"):
            current = module.getPv(param).get(as_string=True)
            saved = module.getPv(param + "_Saved").get(as_string=True)
            if current != saved or show_all:
                print "{0} {1} {2}=>{3}".format(module.name, param, saved, current)
    return 0

def actionConfig(args=[]):
    """config modules               Apply current configuration to module"""
    if len(args) < 1:
        usage("save: missing parameter value")
        return 1

    expr = args[0] if args else ".*"

    for module in nED.getModuleNames(expr):
        try:
            module.applyConfig()
            print "Applied current configuration to '{0}'".format(module)
        except UserWarning, e:
            print e
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
    print "  --verbose                    Print detailed progress"
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
        elif arg == "--verbose":
            nED.setVerbose(True)
        elif arg == "--pv-prefix":
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

