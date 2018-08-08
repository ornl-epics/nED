#!/bin/env python
#
# The script generates CSS screens for each module type/version.
#
# Copyright (c) 2018 Oak Ridge National Laboratory.
# All rights reserved.
# See file LICENSE that is included with this distribution.
#
# @author Klemen Vodopivec

import argparse
import os
import re
import sys

__version__ = "0.1.0"

def parse_src_file(path, verbose=False):
    types = {
        'createStatusParam': "status",
        'createConfigParam': "config",
        'createChanConfigParam': "config",
        'createCounterParam': "counters",
        'createTempParam': "temp"
    }
    regex = re.compile("(create.*Param)\s*\(\"([^\"]*)\"[^;]*;\s*/?/?([^\(]*)(.*)$")

    params = {}
    with open(path, "r") as infile:
        for line in infile:
            match = regex.search(line)
            if match and match.group(1) in types:
                typ_ = types[match.group(1)]
                if typ_ not in params:
                    params[typ_] = []

                # Detect mbb[io] options
                props = match.group(4).strip()
                dropdown = False
                if props.startswith("("):
                    for token in props.strip("(").split(","):
                        if "=" in token and token[0].isdigit():
                            dropdown = True
                            break

                params[typ_].append({
                    'name': match.group(2),
                    'desc': match.group(3),
                    'selectable': dropdown,
                })

    return params

def xml_escape(text):
    xml_escape_table = {
        "&": "&amp;",
        '"': "&quot;",
        "'": "&apos;",
        ">": "&gt;",
        "<": "&lt;",
    }
    return "".join(xml_escape_table.get(c,c) for c in text)

def generate_config_file(outfile, title, params):
    outfile.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
    outfile.write("<display version=\"2.0.0\">\n")
    outfile.write("  <name>{0}</name>\n".format(title))
    outfile.write("  <width>650</width>\n")

    row = 0
    for param in params:
        outfile.write("  <widget type=\"rectangle\" version=\"2.0.0\">\n")
        outfile.write("    <name>Rectangle</name>\n")
        outfile.write("    <x>1</x>\n")
        outfile.write("    <y>{0}</y>\n".format(row*24))
        outfile.write("    <width>440</width>\n")
        outfile.write("    <height>25</height>\n")
        outfile.write("    <line_width>1</line_width>\n")
        outfile.write("    <line_color>\n")
        outfile.write("      <color name=\"Grid\" red=\"128\" green=\"128\" blue=\"128\"></color>\n")
        outfile.write("    </line_color>\n")
        outfile.write("    <background_color>\n")
        outfile.write("      <color name=\"Background\" red=\"255\" green=\"255\" blue=\"255\"></color>\n")
        outfile.write("    </background_color>\n")
        outfile.write("  </widget>\n")

        outfile.write("  <widget type=\"label\" version=\"2.0.0\">\n")
        outfile.write("    <x>5</x>\n")
        outfile.write("    <y>{0}</y>\n".format(row*24+2))
        outfile.write("    <width>145</width>\n")
        outfile.write("    <height>20</height>\n")
        outfile.write("    <text>{0}</text>\n".format(param['name']))
        outfile.write("    <tooltip>{0}</tooltip>\n".format(xml_escape(param['desc'])))
        outfile.write("    <horizontal_alignment>0</horizontal_alignment>")
        outfile.write("  </widget>\n")

        outfile.write("  <widget type=\"{0}\" version=\"2.0.0\">\n".format("combo" if param['selectable'] else "textentry"))
        outfile.write("    <pv_name>$(P){0}</pv_name>\n".format(param['name']))
        outfile.write("    <x>150</x>\n")
        outfile.write("    <y>{0}</y>\n".format(row*24+2))
        outfile.write("    <width>130</width>\n")
        outfile.write("    <height>20</height>\n")
        outfile.write("    <background_color>\n")
        outfile.write("      <color name=\"Read_Background\" red=\"240\" green=\"240\" blue=\"240\">\n")
        outfile.write("      </color>\n")
        outfile.write("    </background_color>\n")
        outfile.write("    <rules>\n")
        outfile.write("      <rule name=\"Color\" prop_id=\"background_color\" out_exp=\"false\">\n")
        outfile.write("        <exp bool_exp=\"pv0!=pv1\">\n")
        outfile.write("          <value>\n")
        outfile.write("            <color red=\"255\" green=\"220\" blue=\"20\">\n")
        outfile.write("            </color>\n")
        outfile.write("          </value>\n")
        outfile.write("        </exp>\n")
        outfile.write("        <pv_name>$(P){0}</pv_name>\n".format(param['name']))
        outfile.write("        <pv_name>$(P){0}_Saved</pv_name>\n".format(param['name']))
        outfile.write("      </rule>\n")
        outfile.write("    </rules>\n")
        outfile.write("  </widget>\n")

        outfile.write("  <widget type=\"textupdate\" version=\"2.0.0\">\n")
        outfile.write("    <pv_name>$(P){0}_Saved</pv_name>\n".format(param['name']))
        outfile.write("    <x>305</x>\n")
        outfile.write("    <y>{0}</y>\n".format(row*24+2))
        outfile.write("    <width>130</width>\n")
        outfile.write("    <height>20</height>\n")
        outfile.write("  </widget>\n")

        row += 1

    outfile.write("  <height>{0}</height>\n".format(row*24+6))
    outfile.write("</display>\n")

def generate_table_file(outfile, title, params):
    outfile.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
    outfile.write("<display version=\"2.0.0\">\n")
    outfile.write("  <name>{0}</name>\n".format(title))
    outfile.write("  <width>650</width>\n")

    row = 0
    for param in params:
        outfile.write("  <widget type=\"rectangle\" version=\"2.0.0\">\n")
        outfile.write("    <name>Rectangle</name>\n")
        outfile.write("    <x>1</x>\n")
        outfile.write("    <y>{0}</y>\n".format(row*24))
        outfile.write("    <width>282</width>\n")
        outfile.write("    <height>25</height>\n")
        outfile.write("    <line_width>1</line_width>\n")
        outfile.write("    <line_color>\n")
        outfile.write("      <color name=\"Grid\" red=\"128\" green=\"128\" blue=\"128\"></color>\n")
        outfile.write("    </line_color>\n")
        outfile.write("    <background_color>\n")
        outfile.write("      <color name=\"Background\" red=\"255\" green=\"255\" blue=\"255\"></color>\n")
        outfile.write("    </background_color>\n")
        outfile.write("  </widget>\n")

        outfile.write("  <widget type=\"label\" version=\"2.0.0\">\n")
        outfile.write("    <x>5</x>\n")
        outfile.write("    <y>{0}</y>\n".format(row*24+2))
        outfile.write("    <width>145</width>\n")
        outfile.write("    <height>20</height>\n")
        outfile.write("    <text>{0}</text>\n".format(param['name']))
        outfile.write("    <tooltip>{0}</tooltip>\n".format(xml_escape(param['desc'])))
        outfile.write("    <horizontal_alignment>0</horizontal_alignment>")
        outfile.write("  </widget>\n")

        outfile.write("  <widget type=\"textupdate\" version=\"2.0.0\">\n")
        outfile.write("    <pv_name>$(P){0}</pv_name>\n".format(param['name']))
        outfile.write("    <x>150</x>\n")
        outfile.write("    <y>{0}</y>\n".format(row*24+2))
        outfile.write("    <width>130</width>\n")
        outfile.write("    <height>20</height>\n")
        outfile.write("  </widget>\n")

        row += 1

    outfile.write("  <height>{0}</height>\n".format(row*24+6))
    outfile.write("</display>\n")

def main():
    usage = ("%prog -i <input .cpp file> -o <outdir>\n")

    parser = argparse.ArgumentParser(description=usage)
    parser.add_argument("infile",  default=None, help="Input .cpp file")
    parser.add_argument("outdir", default=None, help="Output directory")
    parser.add_argument("-v", "--verbose", default=False, action="store_true", help="Verbose mode")
    args = parser.parse_args()

    if not args.infile or not args.outdir:
        parse.print_usage()
        sys.exit(1)

    if not os.path.isdir(args.outdir):
        os.makedirs(args.outdir)

    params = parse_src_file(args.infile, args.verbose)
    for filetype,fileparams in params.iteritems():
        filename = os.path.basename(args.infile).rsplit(".", 1)[0]
        filename += "_" + filetype + ".bob"
        filepath = os.path.join(args.outdir, filename)
        with open(filepath, "w") as outfile:
            if filetype is "config":
                generate_config_file(outfile, "$(D) config", fileparams)
            else:
                generate_table_file(outfile, "$(D) " + filetype, fileparams)

if __name__ == "__main__":
    main()
