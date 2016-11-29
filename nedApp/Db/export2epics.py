#!/bin/env python
#
# The script generates EPICS databases files for the selected module.
#
# The description is truncated to match EPICS string specifications
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
    re_option = re.compile("^([^=]*)=([^\[]*)(.*)\]?")

    names = {
        'status':     [ "name", "offset", "width", "bit_offset" ],
        'counter':    [ "name", "offset", "width", "bit_offset" ],
        'config':     [ "name", "section", "section_offset", "width", "bit_offset", "default", "convert" ],
        'config_ch':  [ "name", "channel", "section", "section_offset", "width", "bit_offset", "default", "convert" ],
        'config_meta': [ "name", "width", "default", "convert" ],
        'upgrade':    [ "name", "offset", "width", "bit_offset" ],
        'temp':       [ "name", "offset", "width", "bit_offset" ],
        'preampcfg':  [ "name", "offset", "width", "bit_offset", "default" ],
        'preamptrig': [ "name", "offset", "width", "bit_offset", "default" ],
    }

    params = map(lambda x: x.strip(" \t\"\'"), params_str.split(","))
    param = dict(zip(names[type], params))

    for item in [ "width", "bit_offset", "default" ]:
        if item in param:
            param[item] = int(param[item], 0)
    if 'default' not in param:
        param['default'] = None

    param['desc'] = desc_str.strip(" \t\n").replace("\"", "\\\"")
    if param['desc'][:28] != param['desc']:
        param['desc'] = param['desc'][:28]
        sys.stderr.write("WARN: Truncating '{0}' record description to 28 chars\n".format(param['name']))

    if extra_str:
        extra_str = extra_str.strip(" \t\n()")
        for e in extra_str.split(","):
            e = e.strip(" \t")
            if e.startswith("calc:"):
                param['calcread'] = e[5:].strip(" ")
            elif e.startswith("calcread:"):
                param['calcread'] = e[9:].strip(" ")
            elif e.startswith("calcwrite:"):
                param['calcwrite'] = e[10:].strip(" ")
            elif e.startswith("calclink:"):
                param['calcread_link'] = e[9:].strip(" ")
            elif e.startswith("prec:"):
                param['prec'] = e[5:].strip(" ")
            elif e.startswith("unit:"):
                param['unit'] = e[5:].strip(" ")
            elif e.startswith("low:"):
                param['low'] = e[4:].strip(" ")
            elif e.startswith("high:"):
                param['high'] = e[5:].strip(" ")
            elif e.startswith("archive:"):
                param['archive'] = e[8:].strip(" ")
            elif e.startswith("scale:"):
                param['slope_scale'] = e[6:].strip(" ")
            elif e.startswith("offset:"):
                param['slope_offset'] = e[7:].strip(" ")
            elif "=" in e:
                if "options" not in param:
                    param['options'] = []
                match = re_option.search(e)
                if match:
                    d = {
                        'key': int(match.group(1).strip(" ")),
                        'value': match.group(2).strip(" " ),
                        'alarm': False
                    }
                    if "alarm" in match.group(3):
                        d['alarm'] = True
                    param['options'].append(d)

    if type in [ "config", "config_ch", "config_meta", "preampcfg", "preamptrig" ]:
        param['direction'] = "inout"
    else:
        param['direction'] = "in"

    return param

def parse_src_file(path, verbose=False):

    params = []

    regexes = {
        'status':     re.compile("createStatusParam\s*\((.*)\);(.*)$"),
        'counter':    re.compile("createCounterParam\s*\((.*)\);(.*)$"),
        'config_meta':re.compile("createMetaConfigParam\s*\((\s*\"[^\"]*\"\s*,[^\,]*,[^\,]*)\);(.*)$"),
        'config':     re.compile("createConfigParam\s*\((.*)\);(.*)$"),
        'config_ch':  re.compile("createChanConfigParam\s*\((.*)\);(.*)$"),
        'upgrade':    re.compile("createUpgradeParam\s*\((.*)\);(.*)$"),
        'temp':       re.compile("createTempParam\s*\((.*)\);(.*)$"),
        'preampcfg':  re.compile("createPreAmpCfgParam\s*\((.*)\);(.*)$"),
        'preamptrig': re.compile("createPreAmpTrigParam\s*\((.*)\);(.*)$"),
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
                        sys.stderr.write("WARNING: No description for {0}\n".format(match.group(1).split(",")[0]))
                    else:
                        desc = match_d.group(1)
                        extra = match_d.group(2)

                    params.append( parse_one(type, match.group(1), desc, extra) )

                    break

    return params

def _aiao_val(param, outfile, default_value=None):
    if default_value is not None:
        outfile.write("    field(VAL,  \"{0}\")\n".format(param['default']))
    if "prec" in param:
        outfile.write("    field(PREC, \"{0}\")\n".format(param['prec']))
    if "unit" in param:
        outfile.write("    field(EGU,  \"{0}\")\n".format(param['unit']))
    if "low" in param:
        outfile.write("    field(LOW,  \"{0}\")\n".format(param['low']))
        outfile.write("    field(LSV,  \"MAJOR\")\n")
    if "high" in param:
        outfile.write("    field(HIGH, \"{0}\")\n".format(param['high']))
        outfile.write("    field(HSV,  \"MAJOR\")\n")
    if "slope_scale" in param:
        outfile.write("    field(ESLO, \"{0}\")\n".format(param['slope_scale']))
        outfile.write("    field(LINR, \"SLOPE\")\n")
    if "slope_offset" in param:
        outfile.write("    field(EOFF, \"{0}\")\n".format(param['slope_offset']))
        if "scale" not in param:
            outfile.write("    field(LINR, \"SLOPE\")\n")

def _bibo_val(param, outfile, default_value=None):
    for i in range(0, 1):
        if param['options'][i]['key'] not in [ 0, 1 ]:
            sys.stderr.write("ERROR: Invalid 2-choice key: {0}\n".format(param['options'][i]['key']))
            return

    if param['options'][0]['key'] == 0:
        zero = param['options'][0]
        one  = param['options'][1]
    else:
        zero = param['options'][1]
        one  = param['options'][0]

    outfile.write("    field(ZNAM, \"{0}\")\n".format(zero['value']))
    if zero['alarm']:
        outfile.write("    field(ZSV,  \"MAJOR\")\n")

    outfile.write("    field(ONAM, \"{0}\")\n".format(one['value']))
    if one['alarm']:
        outfile.write("    field(OSV,  \"MAJOR\")\n")

    if default_value is not None:
        if default_value == zero['key']:
            outfile.write("    field(VAL,  \"0\")\n")
        elif default_value == one['key']:
            outfile.write("    field(VAL,  \"1\")\n")
        else:
            sys.stderr.write("ERROR: Default value '{0}' not found in record\n".format(default_value))

def _mbbimbbo_val(param, outfile, default_value=None):
    fields = [
        { 'val': "ZRVL", 'string': "ZRST", 'alarm': "ZRSV" },
        { 'val': "ONVL", 'string': "ONST", 'alarm': "ONSV" },
        { 'val': "TWVL", 'string': "TWST", 'alarm': "TWSV" },
        { 'val': "THVL", 'string': "THST", 'alarm': "THSV" },
        { 'val': "FRVL", 'string': "FRST", 'alarm': "FRSV" },
        { 'val': "FVVL", 'string': "FVST", 'alarm': "FVSV" },
        { 'val': "SXVL", 'string': "SXST", 'alarm': "SXSV" },
        { 'val': "SVVL", 'string': "SVST", 'alarm': "SVSV" },
        { 'val': "EIVL", 'string': "EIST", 'alarm': "EISV" },
        { 'val': "NIVL", 'string': "NIST", 'alarm': "NISV" },
        { 'val': "TEVL", 'string': "TEST", 'alarm': "TESV" },
        { 'val': "ELVL", 'string': "ELST", 'alarm': "ELSV" },
        { 'val': "TVVL", 'string': "TVST", 'alarm': "TVSV" },
        { 'val': "TTVL", 'string': "TTST", 'alarm': "TTSV" },
        { 'val': "FTVL", 'string': "FTST", 'alarm': "FTSV" },
        { 'val': "FFVL", 'string': "FFST", 'alarm': "FFSV" },
    ]
    val_field = None
    for i in range(0,len(param['options'])):
        outfile.write("    field({0}, \"{1}\")\n".format(fields[i]['val'], param['options'][i]['key']))
        outfile.write("    field({0}, \"{1}\")\n".format(fields[i]['string'], param['options'][i]['value']))

        # Check if this value needs to set an alarm
        if param['options'][i]['alarm']:
            outfile.write("    field({0}, \"MAJOR\")\n".format(fields[i]['alarm']))

        # Find the right index for default value in case they're in different order
        if default_value == param['options'][i]['key']:
            val_field = "    field(VAL,  \"{0}\")\n".format(i)

    if val_field:
        outfile.write(val_field)

def _longinlongout_val(param, outfile, default_value=None):
    if default_value is not None:
        outfile.write("    field(VAL,  \"{0}\")\n".format(param['default']))
    if "prec" in param:
        outfile.write("    field(PREC, \"{0}\")\n".format(param['prec']))
    if "unit" in param:
        outfile.write("    field(EGU,  \"{0}\")\n".format(param['unit']))
    if "low" in param:
        outfile.write("    field(LOW,  \"{0}\")\n".format(param['low']))
        outfile.write("    field(LSV,  \"MAJOR\")\n")
    if "high" in param:
        outfile.write("    field(HIGH, \"{0}\")\n".format(param['high']))
        outfile.write("    field(HSV,  \"MAJOR\")\n")

def _calc_record(param, outfile, link=None):
    flnk = None
    if 'calcwrite' in param:
        outfile.write("record(calcout, \"$(P){0}CW\")\n".format(param['name']))
        outfile.write("{\n")
        outfile.write("    field(ASG,  \"BEAMLINE\")\n")
        outfile.write("    field(INPA, \"$(P){0} NPP\")\n".format(param['name']))
        outfile.write("    field(CALC, \"{0}\")\n".format(param['calcwrite']))
        outfile.write("    field(OUT,  \"$(P){0}W PP\")\n".format(param['name']))
        outfile.write("}\n")

        outfile.write("record(longout, \"$(P){0}W\")\n".format(param['name']))
        outfile.write("{\n")
        outfile.write("    field(ASG,  \"BEAMLINE\")\n")
        outfile.write("    field(DTYP, \"asynInt32\")\n")
        outfile.write("    field(OUT,  \"@asyn($(PORT)){0}\")\n".format(param['name']))
        if 'calcread' in param:
            outfile.write("    field(SDIS, \"$(P){0}CR.PACT\")\n".format(param['name']))
            outfile.write("    field(DISV, \"1\")\n")
        outfile.write("}\n")
        flnk = "$(P){0}CW".format(param['name'])

    if 'calcread' in param:
        outfile.write("record(longin, \"$(P){0}R\")\n".format(param['name']))
        outfile.write("{\n")
        outfile.write("    field(ASG,  \"BEAMLINE\")\n")
        outfile.write("    field(DTYP, \"asynInt32\")\n")
        outfile.write("    field(INP,  \"@asyn($(PORT)){0}\")\n".format(param['name']))
        outfile.write("    field(SCAN, \"I/O Intr\")\n")
        outfile.write("    field(FLNK, \"$(P){0}CR\")\n".format(param['name']))
        if 'calcwrite' in param:
            outfile.write("    field(SDIS, \"$(P){0}CW.PACT\")\n".format(param['name']))
            outfile.write("    field(DISV, \"1\")\n")
        outfile.write("}\n")

        outfile.write("record(calcout, \"$(P){0}CR\")\n".format(param['name']))
        outfile.write("{\n")
        outfile.write("    field(ASG,  \"BEAMLINE\")\n")
        outfile.write("    field(INPA, \"$(P){0}R NPP\")\n".format(param['name']))
        if link:
            outfile.write("    field(INPB, \"$(P){0} NPP\")\n".format(link))
        outfile.write("    field(CALC, \"{0}\")\n".format(param['calcread']))
        outfile.write("    field(OUT,  \"$(P){0} PP\")\n".format(param['name']))
        outfile.write("}\n")
    return flnk
    
def generate_db_record(param, outfile):
    if "options" in param:
        if len(param['options']) == 2 and param['width'] == 1:
            type="bi" if param['direction'] == "in" else "bo"
        else:
            type="mbbi" if param['direction'] == "in" else "mbbo"
    elif "slope_scale" in param or "slope_offset" in param:
        type="ai" if param['direction'] == "in" else "ao"
    else:
        type="longin" if param['direction'] == "in" else "longout"

    if "calcread" in param or "calcwrite" in param:
        calclink = param['calcread_link'] if "calcread_link" in param else None
        flnk = _calc_record(param, outfile, calclink)
        outfile.write("record(ao, \"$(P){0}\")\n".format(param['name']))
        outfile.write("{\n")
        outfile.write("    field(ASG,  \"BEAMLINE\")\n")
        if "prec" not in param:
            param['prec'] = 0
        outfile.write("    field(PREC, \"{0}\")\n".format(param['prec']))

        if param['direction'] == 'in':
            outfile.write("    field(OUT,  \"0\")\n")
        elif flnk:
            outfile.write("    info(autosaveFields, \"VAL\")\n")
            outfile.write("    field(PINI, \"YES\")\n")
            outfile.write("    field(FLNK, \"{0}\")\n".format(flnk))
        _longinlongout_val(param, outfile, param['default'])

    else:
        outfile.write("record({0}, \"$(P){1}\")\n".format(type, param['name']))
        outfile.write("{\n")
        outfile.write("    field(ASG,  \"BEAMLINE\")\n")
        outfile.write("    field(DTYP, \"asynInt32\")\n")

        if type == "bi" or type == "bo":
            _bibo_val(param, outfile, param['default'])
        elif type == "mbbi" or type == "mbbo":
            _mbbimbbo_val(param, outfile, param['default'])
        elif type == "ai" or type == "ao":
            _aiao_val(param, outfile, param['default'])
        else:
            high = 2**param['width'] - 1
            low = 0
            # Check for signed converters, not so strict search as callback
            # can be specified in multiple ways
            if 'convert' in param:
                if "CONV_SIGN_MAGN" in param['convert']:
                    high = 2**(param['width'] - 1) - 1
                    low = -1 * high
                elif "CONV_SIGN_2COMP" in param['convert']:
                    high = 2**(param['width'] - 1) - 1
                    low = -1 * high + 1
            outfile.write("    field(LOPR, \"{0}\")\n".format(low))
            outfile.write("    field(HOPR, \"{0}\")\n".format(high))
            _longinlongout_val(param, outfile, param['default'])

        if param['direction'] == "in":
            outfile.write("    field(INP,  \"@asyn($(PORT)){0}\")\n".format(param['name']))
            outfile.write("    field(SCAN, \"I/O Intr\")\n")
        else:
            outfile.write("    info(autosaveFields, \"VAL\")\n")
            outfile.write("    info(asyn:READBACK, \"1\")\n")
            outfile.write("    field(PINI, \"YES\")\n")
            outfile.write("    field(OUT,  \"@asyn($(PORT)){0}\")\n".format(param['name']))

    if "archive" in param and param['archive'] == "monitor":
        outfile.write("    info(archive, \"Monitor, 00:00:01, VAL\")\n")

    outfile.write("    field(ASG,  \"BEAMLINE\")\n")
    outfile.write("    field(DESC, \"{0}\")\n".format(param['desc'][:28]))
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
        generate_db_record(param, outfile)

if __name__ == "__main__":
    main()
