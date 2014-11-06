#!/usr/bin/perl -s

# Extract PV names and create PVtable templates

use Switch;

sub usage {
    my ($prog) = @_;
    print { STDERR } "Usage: $prog -mode=<status|counter|config> -input_file=<file> -output_file=<file>\n";
}

# Check input parameters
if (!defined $mode or !defined $input_file or !defined $output_file) {
    usage($0);
    exit 1;
}
switch($mode) {
    case "status"    { $line_regex="createStatusParam *\\( *\"([a-zA-Z0-9_:]+)\" *,.*\$"; }
    case "counter"   { $line_regex="createCounterParam *\\( *\"([a-zA-Z0-9_:]+)\" *,.*\$"; }
    case "config"    { $line_regex="createConfigParam *\\( *\"([a-zA-Z0-9_:]+)\" *,.*\$"; }
    else             { usage($0); exit 1; }
}

my $num_lines = 0;
open($INFILE,  "<$input_file")  or die "Error opening input file $input_file";
open($OUTFILE, ">$output_file") or die "Error opening output file $output_file";

# Print pvtable template, all fields prefixed with $(P)
print $OUTFILE "<?xml version=\"1.0\"?>\n";
print $OUTFILE "<pvtable version=\"2.0\">\n";
print $OUTFILE "    <pvlist>\n";
foreach $line ( <$INFILE> ) {
    chomp($line);
    if ($line =~ m/$line_regex/) {
        print $OUTFILE "        <pv>\n";
        print $OUTFILE "            <selected>false</selected>\n";
        print $OUTFILE "            <name>\$(P)$1</name>\n";
        print $OUTFILE "            <tolerance>0.01</tolerance>\n";
        print $OUTFILE "            <saved_value>0</saved_value>\n";
        print $OUTFILE "        </pv>\n";
        $num_lines += 1;
    }
}
print $OUTFILE "    </pvlist>\n";
print $OUTFILE "</pvtable>\n";

close $INFILE;
close $OUTFILE;

if ($num_lines == 0) {
    unlink $output_file;
}
