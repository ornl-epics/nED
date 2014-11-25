#!/bin/env python
#
# The script creates .pvs files from .pvst templates, replacing PV name prefix
# macros on the way.
#
# Based on IOC startup file given through the command line, the script resolves
# PV name prefixes and replaces them in generated .pvs file. The name of the
# output file is generated from the device name part of PV name.
#
# For instance, BL99:Det:roc1: prefix is used to create roc1_config.pvs file.
# Inside new file all $(P) macros are replaced with BL99:Det:roc1
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

__version__ = "0.1.0"

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
    """ Parse st.cmd file and extract plugin name, name from .db declarations """
    plugins = []
    re_plugin = re.compile("^dbLoadRecords.*\".*/(\w+)\.db\".*P=([^,\"]*[:\)](\w+):)")
    with open(st_cmd_filepath, "r") as f:
        for line in f:
            match = re_plugin.search(line)
            if match:
                prefix = match.group(2).replace("$(PREFIX)", bl_prefix)
                plugins.append([match.group(1), prefix, match.group(3)])

    return plugins

def replace_in_file(old_file, new_file, pattern, replacement):
    """ Replace pattern in old_file with replacement stirng and write to new_file """
    with open(old_file, "r") as infile:
        with open(new_file, "w") as outfile:
            for line in infile:
                outfile.write(line.replace(pattern, replacement))

def main():
    usage = "%prog [options] /path/to/st.cmd list-of-pvst-files"

    parse  = OptionParser(usage=usage, version='%prog '+str(__version__))
    parse.add_option("-b", dest="bl_prefix", default=None, help="Beamline PREFIX, example BL99:Det:")
    parse.add_option("-o", dest="outdir", default=".", help="Where to store generated .pvs files")
    parse.add_option("-v", dest="verbose", default=False, action="store_true", help="Verbose mode")
    (options, args) = parse.parse_args()

    if not args or not args[1:]:
        parse.print_usage()
        sys.exit(1)

    st_cmd = args.pop(0)
    env = parse_st_cmd_env(st_cmd)

    bl_prefix = None
    if 'PREFIX' in env:
        bl_prefix = env['PREFIX']
        if options.verbose:
            print "Found beamline prefix '{0}' in {1}".format(bl_prefix, st_cmd)
    if options.bl_prefix:
        if bl_prefix and options.verbose:
            print "Overriding beamline prefix with '{0}'".format(options.bl_prefix)
        bl_prefix = options.bl_prefix
    if not bl_prefix:
        print "ERROR: Can not find beamline prefix, must specify one with -b option"
        parse.print_help()
        sys.exit(1)
    if bl_prefix[-1] != ":":
        bl_prefix.append(":")

    plugins = parse_st_cmd_plugins(st_cmd, bl_prefix)

    # Go through files passed through command line and replace PV name prefix
    for file in args:
        for (plugin,prefix,name) in plugins:
            match = re.match(".*{0}.*\.pvst".format(plugin), file)
            if match:
                new_file = os.path.basename(file).replace(plugin, name).replace(".pvst", ".pvs")
                new_file = os.path.normpath(options.outdir + "/" + new_file)

                if options.verbose:
                    print "{0} -> {1}".format(file, new_file)
                replace_in_file(file, new_file, "$(P)", prefix)
    if len(plugins) == 0:
        print "No suitable plugins found"

if __name__ == "__main__":
    main()
