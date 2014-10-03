#!/usr/bin/perl -s

# Extract PV names and create PVtable templates

use Switch;

sub usage {
    my ($prog) = @_;
    print { STDERR } "Usage: $prog -mode=<status|counter|config>\n";
}

# Check input parameters
if (!defined $mode) {
    usage($0);
    exit 1;
}
switch($mode) {
    case "status"    { $line_regex="createStatusParam *\\( *\"([a-zA-Z0-9_]+)\" *,.*\$"; }
    case "counter"   { $line_regex="createCounterParam *\\( *\"([a-zA-Z0-9_]+)\" *,.*\$"; }
    case "config"    { $line_regex="createConfigParam *\\( *\"([a-zA-Z0-9_]+)i\" *,.*\$"; }
    else             { usage($0); exit 1; }
}

# Print pvtable template, all fields prefixed with $(P)
print("<?xml version=\"1.0\"?>\n");
print("<pvtable version=\"2.0\">\n");
print("    <pvlist>\n");
foreach $line ( <STDIN> ) {
    chomp($line);
    if ($line =~ m/$line_regex/) {
        print("        <pv>\n");
        print("            <selected>false</selected>\n");
        print("            <name>\$(P)$1</name>\n");
        print("            <tolerance>0.01</tolerance>\n");
        print("            <saved_value>0</saved_value>\n");
        print("        </pv>\n");
    }
}
print("    </pvlist>\n");
print("</pvtable>\n");
