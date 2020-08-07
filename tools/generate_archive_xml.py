#!/bin/env python
#
# Parse st.cmd file, extract archived records from DB template files and 
# produce archiver compatible section of XML.
#
# Copyright (c) 2015 Oak Ridge National Laboratory.
# All rights reserved.
# See file LICENSE that is included with this distribution.
#
# @author Klemen Vodopivec

import sys
import re
import os
from optparse import OptionParser

__version__ = "0.2.0"

def do_substitutions(str, macros):
    for macro,substitution in macros.iteritems():
        str = str.replace("$(" + macro + ",undefined)", substitution)
        str = str.replace("$(" + macro + ")",           substitution)
    return str

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

def parse_st_cmd_plugins(st_cmd_filepath, bl_prefix, verbose):
    """ Parse st.cmd file and extract plugin name, pv prefix and device name
        from .db declarations """
    plugins = []
    includes = []
    re_plugin = re.compile("^dbLoadRecords\s*\(\"\s*([^\"]*)\".*P=([^,\"]*[:\)](\w+):)")
    re_include = re.compile("^< *(\S*)")
    macros = { 'PREFIX': bl_prefix }

    # First parse the st.cmd, remember any included files
    if verbose:
        print "Parsing '{0}' for macros and device plugins".format(st_cmd_filepath)
    with open(st_cmd_filepath, "r") as f:
        for line in f:
            match = re_plugin.search( do_substitutions(line, macros) )
            if match:
                prefix = match.group(2).replace(bl_prefix + ":", bl_prefix)
                plugins.append({ 'file': match.group(1),
                                 'pv_prefix': prefix,
                                 'device': match.group(3)
                               })

            match = re_include.search(line)
            if match:
                includes.append(match.group(1))

    # Now process include files the same way
    basedir = os.path.dirname(st_cmd_filepath)
    for include in includes:
        if verbose:
            print "Parsing '{0}' for device plugins".format(include)
        with open(os.path.join(basedir, include), "r") as f:
            for line in f:
                match = re_plugin.search( do_substitutions(line, macros) )
                if match:
                    prefix = match.group(2).replace(bl_prefix + ":", bl_prefix)
                    plugins.append({ 'file': match.group(1),
                                     'pv_prefix': prefix,
                                     'device': match.group(3)
                                   })

    return plugins

def parse_archive_records(db_path, macros={}, verbose=False):
    re_record  = re.compile('\s*record\s*\([^,]*,\s*"?([^"]*)"?\s*\)')
    re_archive = re.compile('\s*info\s*\(\s*archive\s*,\s*"?([^"]*)"?\s*\)')

    archived = dict()

    with open(db_path, "r") as f:
        if verbose:
            print "Parsing file " + db_path
        for line in f:

            # Look for EPICS db record, identified by:
            #
            # record(<type>, <name>) {
            match = re_record.search(do_substitutions(line, macros))
            if match:
                name = match.group(1)

                while not re.search('\{', line):
                    line = f.next()

                # Parse record attributes, look for:
                #    info(archive, "<description>")
                while not re.search('\}', line):
                    line = f.next()
                    match = re_archive.search(do_substitutions(line, macros))
                    if match:
                        if verbose:
                            print "Found archived record: " + name

                        archive_desc = match.group(1).split(",")
                        if len(archive_desc) > 2:
                            scan, period, fields = archive_desc
                        else:
                            scan, period = archive_desc
                            fields = None

                        archived[name] = { 'monitor': (scan == "Monitor"),
                                           'period': period.strip(),
                                           'fields': fields }

                        break
    if verbose:
        print ""

    return archived

def print_xml(iocname, records):
    #    <group>
    #        <name>Detectors</name>
    #        <channel>
    #            <name>BL16B:Det:occ:FpgaTemp</name>
    #            <period>00:01:00</period>
    #            <monitor/>
    #        </channel>
    #    </group>

    print "    <group>"
    print "        <name>{0}</name>".format(iocname)
    for channel,record in records.iteritems():
        print "        <channel>"
        print "            <name>{0}</name>".format(channel)
        print "            <period>{0}</period>".format(record['period'])
        if record['monitor']:
            print "            <monitor/>"
        else:
            print "            <scan/>"
        print "        </channel>"
    print "    </group>"


def main():
    usage = ("%prog [options] /path/to/st.cmd\n"
             "\n"
             "Create .pvs files for all detector devices configured in st.cmd.")

    parse = OptionParser(usage=usage, version='%prog '+str(__version__))
    parse.add_option("-b", dest="bl_prefix", default=None, help="Override beamline PREFIX, example BL99:Det:")
    parse.add_option("-n", dest="ned_dir", default=None, help="Path to nED root directory")
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

    # Retrieve or generate IOC name
    if 'IOCNAME' in env:
        iocname = env['IOCNAME']
    else:
        iocname = bl_prefix.rstrip(":").replace(":", "_") + "_nED"

    # Verify nED directory
    if options.ned_dir:
        ned_dir = os.path.realpath(options.ned_dir)
        if not os.path.isdir(ned_dir):
            print "ERROR: {0} is not valid nED top level directory".format(options.ned_dir)
            sys.exit(1)
    else:
        ned_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")
        if not os.path.isdir(ned_dir):
            print "ERROR: Can't detect nED root directory, use -n parameter to specify it"
            sys.exit(1)

    plugins = parse_st_cmd_plugins(st_cmd, bl_prefix, options.verbose)
    if not plugins:
        print "No plugins found in '{0}'".format(st_cmd)

    archived = dict()
    macros = {
        'NED': ned_dir,
        'TOP': os.path.dirname( os.path.dirname( os.path.dirname(st_cmd) ) )
    }
    for plugin in plugins:
        macros['P'] = plugin['pv_prefix']
        macros['G'] = bl_prefix+"HVG"
        inpath = os.path.realpath(do_substitutions(plugin['file'], macros ))
        archived.update(parse_archive_records(inpath, macros, options.verbose))

    print_xml(iocname, archived)

if __name__ == "__main__":
    main()
