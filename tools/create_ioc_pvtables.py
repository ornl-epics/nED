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
import argparse

__version__ = "0.2.0"

PVTABLE_DIR_TMPL = "/home/controls/<beamline>/pvtable/<iocname>/"
BL = os.environ['BL']

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
    re_plugin = re.compile("^dbLoadRecords.*\".*/(\w+_v\d+)\.db\".*P=([^,\"]*[:\)](\w+):)")
    re_include = re.compile("^< *(\S*)")

    # First parse the st.cmd, remember any included files
    if verbose:
        print "Parsing '{0}' for macros and device plugins".format(st_cmd_filepath)
    with open(st_cmd_filepath, "r") as f:
        for line in f:
            match = re_plugin.search(line.replace(",undefined", ""))
            if match:
                prefix = match.group(2).replace("$(PREFIX)", bl_prefix).replace(bl_prefix + ":", bl_prefix)
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
        if verbose:
            print "Parsing '{0}' for device plugins".format(include)
        with open(os.path.join(basedir, include), "r") as f:
            for line in f:
                match = re_plugin.search(line.replace(",undefined", ""))
                if match:
                    prefix = match.group(2).replace("$(PREFIX)", bl_prefix)
                    plugins.append({ 'name': match.group(1),
                                     'pv_prefix': prefix,
                                     'device': match.group(3)
                                   })

    return plugins

vars_cache = dict()
def parse_src_file(path, mode):
    """ Parses nED .cpp file to get register definition for a given register set.
    Returns a dictionary of register names with defaults for values. """

    ignore = [ "upgrade", "version" ]

    types = {
      'version': re.compile("createVersionParam\s*\(\s*\"([^\"]+)\"\s*,"),
      'upgrade': re.compile("createUpgradeParam\s*\(\s*\"([^\"]+)\"\s*,"),
      'status':  re.compile("createStatusParam\s*\(\s*\"([^\"]+)\"\s*,"),
      'counter': re.compile("createCounterParam\s*\(\s*\"([^\"]+)\"\s*,"),
      'config':  re.compile("createConfigParam\s*\(\s*\"([^\"]+)\"[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,\s*(\S+)\s*[,\)].*"),
      'config_ch': re.compile("createChanConfigParam\s*\(\s*\"([^\"]+)\"[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,\s*(\S+)\s*[,\)].*"),
      'config_meta': re.compile("createMetaConfigParam\s*\(\s*\"([^\"]+)\",[^,]*,\s*(\S+)\s*[,\)].*"),
      'temp':    re.compile("createTempParam\s*\(\s*\"([^\"]+)\"\s*,"),
      'preampcfg': re.compile("createPreAmpCfgParam\s*\(\s*\"([^\"]+)\"\s*,"),
      'preamptrig':re.compile("createPreAmpTrigParam\s*\(\s*\"([^\"]+)\"\s*,"),
    }

    if path not in vars_cache:
        with open(path, "r") as infile:

            vars_cache[path] = dict([ (k,[]) for k in types.keys() ])

            for line in infile:
                line = line.strip(" \t\n")
                matched = False
                for type,regex in types.items():
                    if type in [ "config_ch", "config_meta" ]:
                        type = "config"
                    match = regex.search(line)
                    if match:
                        matched = True
                        if type not in ignore:
                            val = 0
                            if type == "config":
                                val = match.group(2)
                            vars_cache[path][type].append({ 'name': match.group(1), 'val': val })
                        break
                if not matched and line.strip(" \t").startswith("create"):
                    raise RuntimeError("Line '{0}' not parsed".format(line))

    return vars_cache[path][mode]

def update_pvs_file(outpath, pv_prefix, vars):
    name_regex = re.compile(".*<name>(.*)</name>.*")
    xml_lines = []
    xml_parts = {}
    # Read existing PVs from XML files and remember them
    with open(outpath, "r") as f:
        while True:
            line = f.readline()
            if not line:
                break
            if "<pv>" in line:
                xml_buffer = ""
                # Eat all lines that belong to <pv>...</pv> group
                while not "</pv>" in line:
                    xml_buffer += line
                    line = f.readline()
                xml_buffer += line

                m = name_regex.search(xml_buffer)
                if m:
                    xml_parts[ m.group(1) ] = xml_buffer
                else:
                    raise RuntimeError("Invalid xml, <name> tag missing")
            else:
                xml_lines.append(line)

    with open(outpath, "w") as f:
        # Match the beginning of XML file
        while xml_lines:
            line = xml_lines.pop(0)
            f.write(line)
            if "<pvlist>" in line:
                break

        # Now iterate over the new PVs, use existing XML parts in place
        # or add new XML code
        for var in vars:
            pv_name = pv_prefix + var['name']
            if pv_name in xml_parts:
                f.write(xml_parts[pv_name])
            else:
                f.write("        <pv>\n")
                f.write("            <selected>true</selected>\n")
                f.write("            <name>{0}{1}</name>\n".format(pv_prefix, var["name"]))
                f.write("            <tolerance>0.1</tolerance>\n")
                f.write("            <saved_value>{0}</saved_value>\n".format(var["val"]))
                f.write("        </pv>\n")

        # Finally the XML tail
        for line in xml_lines:
            f.write(line)


def write_pvs_file(outpath, pv_prefix, vars):
    with open(outpath, "w") as outfile:

        outfile.write("<?xml version=\"1.0\"?>\n")
        outfile.write("<pvtable version=\"2.0\">\n")
        outfile.write("    <pvlist>\n")

        for var in vars:
            outfile.write("        <pv>\n")
            outfile.write("            <selected>true</selected>\n")
            outfile.write("            <name>{0}{1}</name>\n".format(pv_prefix, var["name"]))
            outfile.write("            <tolerance>0.1</tolerance>\n")
            outfile.write("            <saved_value>{0}</saved_value>\n".format(var["val"]))
            outfile.write("        </pv>\n")

        outfile.write("    </pvlist>\n")
        outfile.write("</pvtable>\n")

def main():
    desc = "Create or update .pvs files for all detector and other modules. " \
           "Based on st.cmd file it tries to figure out all settings automatically, " \
           "with optional parameters to override those. By default it will not " \
           "overwrite existing files unless --force is specified."

    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument("st_cmd", help="Path to nED IOC st.cmd file")
    parser.add_argument("-b", "--bl-prefix", help="Override beamline PV prefix")
    parser.add_argument("-f", "--force", default=False, help="Overwrite existing files", action="store_true")
    parser.add_argument("-n", "--ned-dir", default=None, help="Path to nED root directory")
    parser.add_argument("-o", "--outdir", default=None, help="Override output directory")
    parser.add_argument("-v", "--verbose", default=False, action="store_true", help="Verbose mode")
    args = parser.parse_args()

    # Verify IOC startup file
    if not os.path.isfile(args.st_cmd):
        print "ERROR: Invalid IOC startup file path '{0}'".format(args.st_cmd)
        sys.exit(1)

    # Try to read EPICS macros from startup file
    env = parse_st_cmd_env(args.st_cmd)
    if args.verbose:
        print "Found {0} EPICS macros in IOC startup file".format(len(env))
        for k,v in env.items():
            print " {0} = {1}".format(k, v)

    # Verify beamline prefix, command line overrides one from st.cmd
    if args.bl_prefix:
        re_bl = re.compile("^BL[0-9]{1,2}.?:\w+$")
        if not re_bl.search(args.bl_prefix):
            raise UserWarning("Invalid BL prefix")
    elif 'PREFIX' in env:
        args.bl_prefix = env['PREFIX']
    else:
        raise UserWarning("No BL PVs prefix found, must specify one with -b")

    if args.bl_prefix[-1] != ":":
        args.bl_prefix += ":"
    if args.verbose:
        print "Using BL prefix: {}".format(args.bl_prefix)

    # Verify output directory
    if args.outdir:
        outdir = args.outdir
    elif 'IOCNAME' in env:
        if args.verbose:
            print "Found IOC name '{0}' in {1}".format(env['IOCNAME'], args.st_cmd)
        beamline = args.bl_prefix.split(":")[0].lower()
        outdir = os.path.normpath(PVTABLE_DIR_TMPL)
        outdir = outdir.replace("<beamline>", beamline)
        outdir = outdir.replace("<iocname>", env['IOCNAME'])
        if args.verbose:
            print "Set output directory to '{0}'".format(outdir)
    else:
        print "ERROR: Can't detect output directory from IOC startup file, use -o parameter to specify it"
        sys.exit(1)

    outdir = os.path.normpath(outdir)
    if not os.path.isdir(outdir):
        print "Creating output directory '{0}'".format(outdir)
        os.makedirs(outdir)

    # Verify nED directory
    if args.ned_dir:
        ned_dir = os.path.join(args.ned_dir, "nedApp", "src")
        if not os.path.isdir(ned_dir):
            print "ERROR: {0} is not valid nED top level directory".format(args.ned_dir)
            sys.exit(1)
    else:
        ned_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "nedApp", "src")
        if not os.path.isdir(ned_dir):
            print "ERROR: Can't detect nED root directory, use -n parameter to specify it"
            sys.exit(1)
    ned_dir = os.path.normpath(ned_dir)

    plugins = parse_st_cmd_plugins(args.st_cmd, args.bl_prefix, args.verbose)
    if not plugins:
        print "No device plugins found in '{0}'".format(args.st_cmd)
    elif args.verbose:
        print "Found {0} device plugins".format(len(plugins))

    all_config = []
    for plugin in plugins:

        if args.verbose:
            print "Found {0} plugin configuration named {1}".format(plugin["name"], plugin["device"])
        inpath = os.path.join(ned_dir, plugin["name"] + ".cpp")

        for mode in [ "status", "config", "counter", "temp" ]:

            try:
                vars = parse_src_file(inpath, mode)
            except RuntimeError, e:
                print "ERROR:", e
                sys.exit(1)
            except IOError:
                print "ERROR: Unsupported plugin '{0}' found in IOC startup file".format(plugin["name"])
                sys.exit(1)

            if vars:
                outpath = os.path.join(outdir, plugin["device"] + "_" + mode + ".pvs")

                file_exists = os.path.isfile(outpath)
                if file_exists:
                    if not args.force:
                        print "File exists, skipping '{0}'".format(outpath)
                        continue

                    if mode == "config":
                        update_pvs_file(outpath, plugin['pv_prefix'], vars)
                    else:
                        write_pvs_file(outpath, plugin['pv_prefix'], vars)
                    print "Updated {0} from {1}".format(outpath, inpath)
                else:
                    write_pvs_file(outpath, plugin['pv_prefix'], vars)
                    print "Created {0} from {1}".format(outpath, inpath)

                if mode == "config":
                    for var in vars:
                        all_config.append({
                            'name': plugin['pv_prefix'] + var['name'],
                            'val': var['val']
                        })

    if all_config:
        outpath = os.path.join(outdir, "all_config.pvs")
        if not os.path.isfile(outpath) or args.force:
            write_pvs_file(outpath, "", all_config)
            print "Created {0}".format(outpath)
        else:
            print "File exists, skipping {0}".format(outpath)

if __name__ == "__main__":
    main()
