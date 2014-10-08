#!/usr/bin/perl -s

# Parse lines like this and transform them into EPICS records:
# createConfigParam("LvdsRxNoEr5", 'E', 0x0,  1, 16, 0); // LVDS ignore errors (0=discard erronous packet,1=keep all packets)
# createStatusParam("UartByteErr", 0x0,  1, 29); // UART: Byte error              (0=no error,1=error)
# The name and description are truncd to match EPICS string specifications

if (!defined $input_file) {
    print { STDERR } "Usage: $0 -input_file=<input file>\n";
    exit 1;
}
my $class=$input_file;
my $version="";
if ($input_file =~ /^([^_]*)_(v[0-9]*)/) {
    $class=$1;
    $version=$2;
} else if ($input_file =~ /^([^\.]*)/) {
    $class=$1;
}

my $reg;

print <<EOF;
/**
 * \\class ${class}
 *
 * Following parameters describe module ${class} ${version} status:
 * Parameter name | Status register   | Description |
 * -------------- | ----------------- | ----------- |
EOF
open (INFILE, $input_file);
foreach $line ( <INFILE> ) {
    chomp($line);
    if ($line =~ m/createStatusParam *\( *"([a-zA-Z0-9_]+)" *, *(0x[0-9A-Fa-f]+) *, *([0-9]+) *, *([0-9]+).*\/\/ *(.*)$/) {
        my ($name,$offset,$width,$shift,$comment) = ($1,$2,$3,$4,$5);
        if ($width == 1) {
            $reg = sprintf("Reg %s Bit %d", $offset, $shift);
        } else {
            $reg = sprintf("Reg %s Bits %d:%d", $offset, $shift, $shift+$width-1);
        }
        printf (" * %-14s | %-17s | %s\n", $name, $reg, $comment);
    }
}
close (INFILE);

print <<EOF;
 *
 * Following parameters describe module ${class} ${version} counters:
 * Parameter name | init val | Counter register | Description |
 * -------------- | -------- | ---------------- | ----------- |
EOF
open (INFILE, $input_file);
foreach $line ( <INFILE> ) {
    chomp($line);
    if ($line =~ m/createCounterParam *\( *"([a-zA-Z0-9_]+)" *, *(0x[0-9A-Fa-f]+) *, *([0-9]+) *, *([0-9]+).*\/\/ *(.*)$/) {
        my ($name,$offset,$width,$shift,$comment) = ($1,$2,$3,$4,$5);
        if ($width == 1) {
            $reg = sprintf("Reg %s Bit %d", $offset, $shift);
        } else {
            $reg = sprintf("Reg %s Bits %d:%d", $offset, $shift, $shift+$width-1);
        }
        printf (" * %-14s | %-17s | %s\n", $name, $reg, $comment);
    }
}
close (INFILE);

print <<EOF;
 *
 * Following parameters describe module ${class} ${version} configuration:
 * Parameter name | init val | Config register  | Description |
 * -------------- | -------- | ---------------- | ----------- |
EOF
open (INFILE, $input_file);
foreach $line ( <INFILE> ) {
    chomp($line);
    if ($line =~ m/createConfigParam *\( *"([a-zA-Z0-9_]+)" *, *'([0-9A-Fa-f])' *, *(0x[0-9A-Fa-f]+) *, *([0-9]+) *, *([0-9]+) *, *([0-9]+).*\/\/ *(.*)$/) {
        my ($name,$section,$offset,$width,$shift,$val,$comment) = ($1,$2,$3,$4,$5,$6,$7);
        if ($width == 1) {
            $reg = sprintf("Reg %s Bit %d", $offset, $shift);
        } else {
            $reg = sprintf("Reg %s Bits %d:%d", $offset, $shift, $shift+$width-1);
        }
        printf (" * %-14s | %8d | %-17s | %s\n", $name, $val, $reg, $comment);
    }
}
close (INFILE);

print <<EOF;
 */
EOF
