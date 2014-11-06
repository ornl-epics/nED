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

def parse_st_cmd(filepath):
    """ Parse st.cmd file and extract plugin name, prefix and detector name from .db declarations """
    plugins = []
    epicsEnv = {}
    re_plugin = re.compile("^dbLoadRecords.*\".*/([\w]+)\.db\".*P=([^,\"]*:(\w*):)")
    re_env    = re.compile("^epicsEnvSet[ \t\"\(]*([\w]+)[, \t\"\)]*([^\"\n]*)")
    with open(filepath, "r") as f:
        for line in f:
            match = re_env.search(line)
            if match:
                epicsEnv[match.group(1)] = match.group(2)

            match = re_plugin.search(line)
            if match:
                # Replace macros
                # Hopefully variables are listed on the top of the file
                prefix = match.group(2)
                for envkey, envval in epicsEnv.items():
                    prefix = prefix.replace("$("+envkey+")", envval)

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
    parse.add_option("-v", dest="verbose", default=False, action="store_true", help="Verbose mode")
    parse.add_option("-o", dest="outdir", default=".", help="Where to store generated .pvs files")
    (options, args) = parse.parse_args()

    if not args or not args[1:]:
        parse.print_usage()
        sys.exit(1)

    plugins = parse_st_cmd(args.pop(0))

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

if __name__ == "__main__":
    main()
