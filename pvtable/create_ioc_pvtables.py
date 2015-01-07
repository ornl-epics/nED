#!/bin/env python
#
# The script creates .pvs files for all detector devices configured in st.cmd.
#
# For each detector found in IOC startup file (ROCs, DSPs, FEMs, etc.) up to 3
# PVTable files are created. There are 3 register types recognized, status,
# configuration and counter and one .pvs file is created for each register type
# supported by device. Device type and version information is extracted from
# st.cmd file and is used to find corresponding .cpp file where registers
# mappings are defined. One row in .pvs file is added for every register
# found. Full PV names are created based on the beamline prefix name found in
# st.cmd file or given through command line.
# Include files from IOC startup file are parsed the same way automatically.
#
# Example:
# This line in st.cmd file:
# dbLoadRecords("../../db/RocPlugin_v52.db","P=$(PREFIX)roc1:,PORT=roc1")
#
# produces 2 files since ROC v5.2 only supports status and configuration
# registers:
# * roc1_status.pvs
# * roc1_config.pvs
#
# Copyright (c) 2014 Oak Ridge National Laboratory.
# All rights reserved.
# See file LICENSE that is included with this distribution.
#
# @author Klemen Vodopivec

import sys
import re
import os
from optparse import OptionParser

__version__ = "0.2.0"

DEFAULT_NED_DIR = "/home/controls/nED/master"
PVTABLE_DIR_TMPL = "/home/controls/<beamline>/opi/pvtable/<iocname>/"

def parse_st_cmd_env(st_cmd_filepath):
    """ Parse st.cmd file and extract environment variables """
    re_env = re.compile("^epicsEnvSet[ \t\"\(]*([\w]+)[, \t\"\)]*([^\"\n]*)")
    env = {}
    with open(st_cmd_filepath, "r") as f:
        for line in f:
            match = re_env.search(line)
            if match:
                env[match.group(1)] = match.group(2)
    return env

def parse_st_cmd_plugins(st_cmd_filepath, bl_prefix):
    """ Parse st.cmd file and extract plugin name, pv prefix and device name
        from .db declarations """
    plugins = []
    includes = []
    re_plugin = re.compile("^dbLoadRecords.*\".*/(\w+_v\d+)\.db\".*P=([^,\"]*[:\)](\w+):)")
    re_include = re.compile("^< *(\S*)")

    # First parse the st.cmd, remember any included files
    with open(st_cmd_filepath, "r") as f:
        for line in f:
            match = re_plugin.search(line)
            if match:
                prefix = match.group(2).replace("$(PREFIX)", bl_prefix)
                plugins.append({ 'name': match.group(1),
                                 'pv_prefix': prefix,
                                 'device': match.group(3)
                               })

            match = re_include.search(line)
            if match:
                includes.append(match.group(1))

    # Now process include files the same way
    basedir = os.path.dirname(st_cmd_filepath)
    for include in includes:
        with open(os.path.join(basedir, include), "r") as f:
            for line in f:
                match = re_plugin.search(line)
                if match:
                    prefix = match.group(2).replace("$(PREFIX)", bl_prefix)
                    plugins.append({ 'name': match.group(1),
                                     'pv_prefix': prefix,
                                     'device': match.group(3)
                                   })

    return plugins

vars_cache = dict()
def parse_src_file(path, mode):

    types = {
      'status':  re.compile("createStatusParam *\( *\"([\w:]+)\" *,"),
      'counter': re.compile("createCounterParam *\( *\"([\w:]+)\" *,"),
      'config':  re.compile("createConfigParam *\( *\"([\w:]+)\" *,")
    }

    if path not in vars_cache:
        with open(path, "r") as infile:

            vars_cache[path] = dict([ (k,[]) for k in types.keys() ])

            for line in infile:
                for type,regex in types.items():
                    match = regex.search(line)
                    if match:
                        vars_cache[path][type].append(match.group(1))

    return vars_cache[path][mode]

def write_pvs_file(outpath, pv_prefix, vars):
    with open(outpath, "w") as outfile:

        outfile.write("<?xml version=\"1.0\"?>\n")
        outfile.write("<pvtable version=\"2.0\">\n")
        outfile.write("    <pvlist>\n")

        for var in vars:
            outfile.write("        <pv>\n")
            outfile.write("            <name>{0}{1}</name>\n".format(pv_prefix, var))
            outfile.write("            <selected>false</selected>\n")
            outfile.write("        </pv>\n")

        outfile.write("    </pvlist>\n")
        outfile.write("</pvtable>\n")

def main():
    usage = ("%prog [options] /path/to/st.cmd\n"
             "\n"
             "Create .pvs files for all detector devices configured in st.cmd.")

    parse = OptionParser(usage=usage, version='%prog '+str(__version__))
    parse.add_option("-b", dest="bl_prefix", default=None, help="Override beamline PREFIX, example BL99:Det:")
    parse.add_option("-n", dest="ned_dir", default=None, help="Path to nED root directory")
    parse.add_option("-o", dest="outdir", default=None, help="Override output directory")
    parse.add_option("-v", dest="verbose", default=False, action="store_true", help="Verbose mode")
    (options, args) = parse.parse_args()

    # Verify IOC startup file
    if not args[0:]:
        parse.print_usage()
        sys.exit(1)
    st_cmd = args[0]
    if not os.path.isfile(st_cmd):
        print "ERROR: Invalid IOC startup file path '{0}'".format(st_cmd)
        sys.exit(1)

    # Try to read EPICS macros from startup file
    env = parse_st_cmd_env(st_cmd)
    if options.verbose:
        print "Found {0} EPICS macros in IOC startup file".format(len(env))
        for k,v in env.items():
            print " {0} = {1}".format(k, v)

    # Verify beamline prefix, command line overrides one from st.cmd
    bl_prefix = None
    if 'PREFIX' in env:
        bl_prefix = env['PREFIX']
        if options.verbose:
            print "Found beamline prefix '{0}' in {1}".format(bl_prefix, st_cmd)
    if options.bl_prefix:
        if bl_prefix and options.verbose:
            print "Overriding beamline prefix '{0}' -> '{1}'".format(bl_prefix, options.bl_prefix)
        bl_prefix = options.bl_prefix
    if not bl_prefix:
        print "ERROR: Can't find beamline prefix, use -b parameter to specify it"
        parse.print_help()
        sys.exit(1)
    if bl_prefix[-1] != ":":
        if options.verbose:
            print "Adjusting beamline prefix '{0}' -> '{0}:'".format(bl_prefix)
        bl_prefix += ":"

    # Verify output directory
    if options.outdir:
        outdir = options.outdir
        if not os.path.isdir(outdir):
            print "ERROR: Output directory '{0}' doesn't exist".format(outdir)
            sys.exit(1)
    elif 'IOCNAME' in env:
        if options.verbose:
            print "Found IOC name '{0}' in {1}".format(env['IOCNAME'], st_cmd)
        beamline = bl_prefix.split(":")[0].lower()
        outdir = os.path.normpath(PVTABLE_DIR_TMPL)
        outdir = outdir.replace("<beamline>", beamline)
        outdir = outdir.replace("<iocname>", env['IOCNAME'])
        if not os.path.isdir(outdir):
            print "ERROR: Output directory '{0}' doesn't exist".format(outdir)
            sys.exit(1)
        if options.verbose:
            print "Set output directory to '{0}'".format(outdir)
    else:
        print "ERROR: Can't detect output directory from IOC startup file, use -o parameter to specify it"
        sys.exit(1)

    # Verify nED directory
    if options.ned_dir:
        ned_dir = os.path.join(options.ned_dir, "nedApp", "src")
        if not os.path.isdir(ned_dir):
            print "ERROR: {0} is not valid nED top level directory".format(options.ned_dir)
            sys.exit(1)
    else:
        ned_dir = os.path.join(DEFAULT_NED_DIR, "nedApp", "src")
        if not os.path.isdir(ned_dir):
            print "ERROR: Can't detect nED root directory, use -n parameter to specify it"
            sys.exit(1)

    plugins = parse_st_cmd_plugins(st_cmd, bl_prefix)

    for plugin in plugins:

        if options.verbose:
            print "Found {0} plugin configuration named {1}".format(plugin["name"], plugin["device"])
        inpath = os.path.join(ned_dir, plugin["name"] + ".cpp")

        for mode in [ "status", "config", "counter" ]:

            try:
                vars = parse_src_file(inpath, mode)
            except:
                print "ERROR: Unsupported plugin '{0}' found in IOC startup file".format(plugin["name"])
                sys.exit(1)

            if vars:
                outpath = os.path.join(outdir, plugin["device"] + "_" + mode + ".pvs")
                write_pvs_file(outpath, plugin["pv_prefix"], vars)

                print "Created {0} from {1}".format(outpath, inpath)

if __name__ == "__main__":
    main()
