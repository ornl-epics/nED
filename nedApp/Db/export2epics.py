#!/bin/env python
#
# The script generates EPICS databases files for the selected module.
#
# Parse lines like this and transform them into EPICS records:
# createConfigParam("LvdsRxNoEr5", 'E', 0x0,  1, 16, 0); // LVDS ignore errors (0=discard erronous packet,1=keep all packets)
# createStatusParam("UartByteErr", 0x0,  1, 29); // UART: Byte error              (0=no error,1=error)
# The description is truncated to match EPICS string specifications
#
# asyn device support doesn't support bi-directional EPICS records. We use a trick
# from Mark Rivers (http://www.aps.anl.gov/epics/tech-talk/2014/msg00057.php)
# and create 3 additional records to support reading and writing to a single
# record.
#
# Copyright (c) 2014 Oak Ridge National Laboratory.
# All rights reserved.
# See file LICENSE that is included with this distribution.
#
# @author Klemen Vodopivec

import sys
import re
from optparse import OptionParser

__version__ = "0.1.0"

def parse_one(type, params_str, desc_str, extra_str):
    names = {
        'status':  [ "name", "offset", "width", "bit_offset" ],
        'counter': [ "name", "offset", "width", "bit_offset" ],
        'config':  [ "name", "section", "section_offset", "width", "bit_offset", "default" ],
    }

    params = map(lambda x: x.strip(" \t\"\'"), params_str.split(","))
    param = dict(zip(names[type], params))

    for item in [ "width", "bit_offset", "default" ]:
        if item in param:
            param[item] = int(param[item], 0)

    param['desc'] = desc_str.strip(" \t\n").replace("\"", "\\\"")
    if param['desc'][:28] != param['desc']:
        param['desc'] = param['desc'][:28]
        sys.stderr.write("WARN: Truncating '{0}' record description to 28 chars".format(param['name']))

    if extra_str:
        extra_str = extra_str.strip(" \t\n()")
        for e in extra_str.split(","):
            e = e.strip(" \t")
            if e.startswith("calc:"):
                param['calc'] = e[5:].strip(" ")
            elif e.startswith("prec:"):
                param['prec'] = e[5:].strip(" ")
            elif e.startswith("unit:"):
                param['unit'] = e[5:].strip(" ")
            elif "=" in e:
                if "options" not in param:
                    param['options'] = []
                (key, value) = e.split("=")
                param['options'].append({
                    'key': int(key.strip(" ")),
                    'value': value.strip(" ")
                })

    if type is "config":
        param['direction'] = "inout"
    else:
        param['direction'] = "in"

    return param

def parse_src_file(path, verbose=False):

    params = []

    regexes = {
        'status':  re.compile("createStatusParam\s*\((.*)\);(.*)$"),
        'counter': re.compile("createCounterParam\s*\((.*)\);(.*)$"),
        'config':  re.compile("createConfigParam\s*\((.*)\);(.*)$"),
    }
    re_desc = re.compile("\s*//\s*([^\(]*)(.*)$")

    with open(path, "r") as infile:
        for line in infile:
            for type,regex in regexes.items():
                match = regex.search(line)
                if match:
                    if verbose:
                        sys.stdout.write("Found {0} parameter {1}\n".format(type, match.group(1).split(",")[0]))
                    match_d = re_desc.search(match.group(2))
                    desc = ""
                    extra = ""
                    if not match_d or not match_d.group(1):
                        sys.stderr.write("ERROR: No description for {0}\n".format(match.group(1).split(",")[0]))
                    else:
                        desc = match_d.group(1)
                        extra = match_d.group(2)

                    params.append( parse_one(type, match.group(1), desc, extra ) )

    return params

def _bibo_val(param, outfile):
    for i in range(0, 1):
        if param['options'][i]['key'] not in [ 0, 1 ]:
            sys.stderr.write("ERROR: Invalid 2-choice key: {0}".format(param['options'][i]['key']))
            return

    if param['options'][0]['key'] == 0:
        outfile.write("    field(ZNAM, \"{0}\")\n".format(param['options'][0]['value']))
        outfile.write("    field(ONAM, \"{0}\")\n".format(param['options'][1]['value']))
    else:
        outfile.write("    field(ZNAM, \"{0}\")\n".format(param['options'][1]['value']))
        outfile.write("    field(ONAM, \"{0}\")\n".format(param['options'][0]['value']))

def _mbbimbbo_val(param, outfile):
    fields = [
        { 'val': "ZRVL", 'string': "ZRST" },
        { 'val': "ONVL", 'string': "ONST" },
        { 'val': "TWVL", 'string': "TWST" },
        { 'val': "THVL", 'string': "THST" },
        { 'val': "FRVL", 'string': "FRST" },
        { 'val': "FVVL", 'string': "FVST" },
        { 'val': "SXVL", 'string': "SXST" },
        { 'val': "SVVL", 'string': "SVST" },
        { 'val': "EIVL", 'string': "EIST" },
        { 'val': "NIVL", 'string': "NIST" },
        { 'val': "TEVL", 'string': "TEST" },
        { 'val': "ELVL", 'string': "ELST" },
        { 'val': "TVVL", 'string': "TVST" },
        { 'val': "TTVL", 'string': "TTST" },
        { 'val': "FTVL", 'string': "FTST" },
        { 'val': "FFVL", 'string': "FFST" },
    ]
    for i in range(0,len(param['options'])):
        outfile.write("    field({0}, \"{1}\")\n".format(fields[i]['val'], param['options'][i]['key']))
        outfile.write("    field({0}, \"{1}\")\n".format(fields[i]['string'], param['options'][i]['value']))

def _longinlongout_val(param, outfile):
    if "prec" in param:
        outfile.write("    field(PREC, \"{0}\")\n".format(param['prec']))
    if "unit" in param:
        outfile.write("    field(EGU,  \"{0}\")\n".format(param['unit']))

def generate_out_db_record(param, outfile):
    if "options" in param:
        if len(param['options']) == 2 and param['width'] == 1:
            type="bo"
            intype="bi"
        else:
            type="mbbo"
            intype="mbbi"
    else:
        type="longout"
        intype="longin"

    outfile.write("record({0}, \"$(P){1}\")\n".format(type, param['name']))
    outfile.write("{\n")
    outfile.write("    info(autosaveFields, \"VAL\")\n")
    outfile.write("    field(ASG,  \"BEAMLINE\")\n")
    outfile.write("    field(DESC, \"{0}\")\n".format(param['desc'][:28]))
    outfile.write("    field(PINI, \"YES\")\n")
    outfile.write("    field(VAL,  \"{0}\")\n".format(param['default']))
    outfile.write("    field(OUT,  \"$(P){0}W PP\")\n".format(param['name']))
    if type == "bo":
        _bibo_val(param, outfile)
    elif type == "mbbo":
        _mbbimbbo_val(param, outfile)
    else:
        _longinlongout_val(param, outfile)
    outfile.write("}\n")

    outfile.write("record({0}, \"$(P){1}W\")\n".format(type, param['name']))
    outfile.write("{\n")
    outfile.write("    field(ASG,  \"BEAMLINE\")\n")
    outfile.write("    field(DESC, \"{0}\")\n".format(param['desc'][:28]))
    outfile.write("    field(DTYP, \"asynInt32\")\n")
    outfile.write("    field(OUT,  \"@asyn($(PORT)){0}\")\n".format(param['name']))
    outfile.write("    field(SDIS, \"$(P){0}S.PACT\")\n".format(param['name']))
    outfile.write("    field(DISV, \"1\")\n")
    if type == "bo":
        _bibo_val(param, outfile)
    elif type == "mbbo":
        _mbbimbbo_val(param, outfile)
    else:
        _longinlongout_val(param, outfile)
    outfile.write("}\n")

    outfile.write("record({0}, \"$(P){1}R\")\n".format(intype, param['name']))
    outfile.write("{\n")
    outfile.write("    field(DTYP, \"asynInt32\")\n")
    outfile.write("    field(DESC, \"{0}\")\n".format(param['desc'][:28]))
    outfile.write("    field(INP,  \"@asyn($(PORT)){0}\")\n".format(param['name']))
    outfile.write("    field(SCAN, \"I/O Intr\")\n")
    outfile.write("    field(FLNK, \"$(P){0}S\")\n".format(param['name']))
    if type == "bo":
        _bibo_val(param, outfile)
    elif type == "mbbo":
        _mbbimbbo_val(param, outfile)
    else:
        _longinlongout_val(param, outfile)
    outfile.write("}\n")

    outfile.write("record({0}, \"$(P){1}S\")\n".format(type, param['name']))
    outfile.write("{\n")
    outfile.write("    field(DOL,  \"$(P){0}R NPP\")\n".format(param['name']))
    outfile.write("    field(OMSL, \"closed_loop\")\n")
    outfile.write("    field(OUT,  \"$(P){0} PP\")\n".format(param['name']))
    if type == "bo":
        _bibo_val(param, outfile)
    elif type == "mbbo":
        _mbbimbbo_val(param, outfile)
    else:
        _longinlongout_val(param, outfile)
    outfile.write("}\n")

def generate_in_db_record(param, outfile):
    if "options" in param:
        if len(param['options']) == 2 and param['width'] == 1:
            type="bi"
        else:
            type="mbbi"
    else:
        type="longin"

    if "calc" in param:
        outfile.write("record(calc, \"$(P){0}\")\n".format(param['name']))
        outfile.write("{\n")
        outfile.write("    field(DESC, \"{0}\")\n".format(param['desc'][:28]))
        outfile.write("    field(INPA, \"$(P){0}_Raw NPP\")\n".format(param['name']))
        outfile.write("    field(CALC, \"{0}\")\n".format(param['calc']))
        if "unit" in param:
            outfile.write("    field(EGU,  \"{0}\")\n".format(param['unit']))
        if "prec" in param:
            outfile.write("    field(PREC, \"{0}\")\n".format(param['prec']))
        outfile.write("}\n")

        outfile.write("record({0}, \"$(P){1}_Raw\")\n".format(type, param['name']))
        outfile.write("{\n")
        outfile.write("    field(FLNK, \"$(P){0}\")\n".format(param['name']))
    else:
        outfile.write("record({0}, \"$(P){1}\")\n".format(type, param['name']))
        outfile.write("{\n")

    outfile.write("    field(DESC, \"{0}\")\n".format(param['desc'][:28]))
    outfile.write("    field(DTYP, \"asynInt32\")\n")
    outfile.write("    field(INP,  \"@asyn($(PORT)){0}\")\n".format(param['name']))
    outfile.write("    field(SCAN, \"I/O Intr\")\n")
#    outfile.write("    field(VAL,  \"0\")\n")
#    outfile.write("    field(PINI, \"YES\")\n")

    if "calc" not in param:
        if type == "bi":
            _bibo_val(param, outfile)
        elif type == "mbbi":
            _mbbimbbo_val(param, outfile)
        else:
            _longinlongout_val(param, outfile)
    outfile.write("}\n")

def main():
    usage = ("%prog -i <input .cpp file> -o <outdir>\n")

    parse = OptionParser(usage=usage, version='%prog '+str(__version__))
    parse.add_option("-i", dest="infile",  default=None, help="Input .cpp file")
    parse.add_option("-o", dest="outfile", default=None, help="Output .db file")
    parse.add_option("-v", dest="verbose", default=False, action="store_true", help="Verbose mode")
    (options, args) = parse.parse_args()

    if not options.infile or not options.outfile:
        parse.print_usage()
        sys.exit(1)

    outfile = open(options.outfile, "w")
    params = parse_src_file(options.infile, options.verbose)
    for param in params:
        if param['direction'] == "in":
            generate_in_db_record(param, outfile)
        else:
            generate_out_db_record(param, outfile)

if __name__ == "__main__":
    main()
