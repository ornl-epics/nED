#!/bin/env python
#
# Copyright (c) 2014 Oak Ridge National Laboratory.
# All rights reserved.
# See file LICENSE that is included with this distribution.
#
# @author Klemen Vodopivec

# The script parses C++ files and look for create*Param function calls. It
# generates .dox files to be included in the final doxygen generated
# documentation.

import sys
import re
import os
import errno 
from optparse import OptionParser

__version__ = "0.1.0"

def format_register(section, offset, width, shift):
    if section:
        if width == 1:
            return "Sec '{0}' Off {1} Bit {2}".format(section, offset, shift)
        else:
            return "Sec '{0}' Off {1} Bits {2}:{3}".format(section, offset, shift, int(shift) + int(width))
    else:
        if width == 1:
            return "Off {1} Bit {2}".format(section, offset, shift)
        else:
            return "Off {1} Bits {2}:{3}".format(section, offset, shift, int(shift) + int(width))

re_params  = re.compile("^\s*createParam\s*\(\s*\"([^\"]+)\"\s*,\s*asynParam([A-Za-z0-9]+)\s*,\s*\&[A-Za-z0-9_]+\s*(,\s*(.*))?\)\s*;\s*(.*)$")
re_config  = re.compile("^\s*createConfigParam\s*\(\s*\"([^\"]+)\"\s*,\s*'(\w)'\s*,\s*(0x[0-9A-Fa-f]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*\)\s*;\s*(.*)$")
re_status  = re.compile("^\s*createStatusParam\s*\(\s*\"([^\"]+)\"\s*,\s*(0x[0-9A-Fa-f]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*\)\s*;\s*(.*)$")
re_counter = re.compile("^\s*createCounterParam\s*\(\s*\"([^\"]+)\"\s*,\s*(0x[0-9A-Fa-f]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*\)\s*;\s*(.*)$")
re_desc    = re.compile("^\s*\/\/\s*([^\(]*)(\(.*\))?$")

def parse_desc(text):
    desc = ""
    values = ""
    if text:
        m = re_desc.search(text)
        if m:
            desc = m.group(1)
            if m.group(2):
                values = m.group(2).strip(" \r\n\t()")
        
    return desc, values

def parse_cpp(filename):
    params = []
    configs = []
    statuses = []
    counters = []
    with open(filename, "r") as f:
        for line in f:
            m = re_params.search(line)
            if m:
                desc, values = parse_desc(m.group(4))
                params.append({
                    'name': m.group(1),
                    'type': m.group(2) + " " + m.group(1),
                    'init_value': m.group(3),
                    'desc': desc,
                    'allowed_values': values
                })
                continue
            m = re_config.search(line)
            if m:
                reg = format_register(m.group(2), m.group(3), m.group(4), m.group(5))
                desc, values = parse_desc(m.group(7))
                configs.append({
                    'name': m.group(1),
                    'register': reg,
                    'init_value': m.group(6),
                    'desc': desc,
                    'allowed_values': values
                })
                continue
            m = re_status.search(line)
            if m:
                reg = format_register(None, m.group(2), m.group(3), m.group(4))
                desc, values = parse_desc(m.group(5))
                statuses.append({
                    'name': m.group(1),
                    'register': reg,
                    'desc': desc,
                    'allowed_values': values
                })
                continue
            m = re_counter.search(line)
            if m:
                reg = format_register(None, m.group(2), m.group(3), m.group(4))
                desc, values = parse_desc(m.group(5))
                counters.append({
                    'name': m.group(1),
                    'register': reg,
                    'desc': desc,
                    'allowed_values': values
                })
                continue

    return params, configs, statuses, counters

def print_params(f, header, params, text):
    f.write(" *\n")
    if text:
        f.write(" * " + text + "\n")

    # Write header
    f.write(" * |")
    for (k, v) in header:
        f.write(" {0} |".format(v))
    f.write("\n")
    f.write(" * | " + "---------- |" * len(header) + "\n")

    # Write params
    for param in params:
        f.write(" * |")
        for (k, v) in header:
            f.write(" {0} |".format(param[k]))
        f.write("\n")

def main():
    usage = "%prog [options] <input files>"

    parse  = OptionParser(usage=usage, version='%prog '+str(__version__))
    parse.add_option("-v", dest="verbose", default=False, action="store_true", help="Verbose mode")
    parse.add_option("-o", dest="outdir", default="tmp/", help="Output directory")
    (options, args) = parse.parse_args()

    if not args or len(args) == 0:
        parse.print_usage()
        sys.exit(1)

    # Create out directory
    try:
        os.makedirs(options.outdir)
    except OSError, e:
        if e.errno != errno.EEXIST:
            raise

    header = [
        ( 'name',           'Parameter name' ),
        ( 'register',       'HW register' ),
        ( 'type',           'Type' ),
        ( 'init_value',     'Default value' ),
        ( 'desc',           'Description' ),
        ( 'allowed_values', 'Valid values' )
    ]


    # Iterate over every input file
    for file in args:
        m = re.match("^([a-zA-Z]+)(_(v[0-9]+))?\.", os.path.basename(file))
        if not m:
            if options.verbose:
                print "Skip non C++ file " + file
            continue
        classname = m.group(1)
        classver = m.group(3)

        (params, configs, statuses, counters) = parse_cpp(file)

        if (len(params) + len(configs) + len(statuses) + len(counters)) > 0:
            outfile = os.path.basename(file.replace('.cpp', '.dox'))
            outfile = os.path.join(options.outdir, outfile)

            if options.verbose:
                print '{0} -> {1}'.format(file, outfile)

            f = open(outfile, "w")

            f.write("/**\n")
            f.write(" * \\class " + classname + "\n")

            if len(params) > 0:
                hdr = filter(lambda x: x[0] in params[0].keys(), header)
                print_params(f, hdr, params, classname + " parameters")

            if len(configs) > 0:
                hdr = filter(lambda x: x[0] in configs[0].keys(), header)
                text = "{0} {1} configuration parameters".format(classname, classver)
                print_params(f, hdr, configs, text)

            if len(statuses) > 0:
                hdr = filter(lambda x: x[0] in statuses[0].keys(), header)
                text = "{0} {1} status parameters".format(classname, classver)
                print_params(f, hdr, statuses, text)

            if len(counters) > 0:
                hdr = filter(lambda x: x[0] in counters[0].keys(), header)
                text = "{0} {1} counter parameters".format(classname, classver)
                print_params(f, hdr, counters, text)

            f.write(" */\n")

            f.close()

if __name__ == "__main__":
    main()
