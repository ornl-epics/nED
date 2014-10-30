#!/usr/bin/perl -s

# Parse lines like this and transform them into Doxygen tables for this class
#
# For non-detector plugins:
# createParam("HwId", asynParamOctet, &HwId); // 28 characters description (optional (mb)[bi|bo] selectors)
# createParam("Type", asynParamOctet, &Type, 0); // 28 characters description (optional (mb)[bi|bo] selectors)
#
# For detector plugins:
# createConfigParam("LvdsRxNoEr5", 'E', 0x0,  1, 16, 0); // LVDS ignore errors (0=discard erronous packet,1=keep all packets)
# createStatusParam("UartByteErr", 0x0,  1, 29); // UART: Byte error              (0=no error,1=error)
# createCounterParam("UartByteErr", 0x0,  1, 29); // UART: Byte error              (0=no error,1=error)

if (!defined $input_file) {
    print { STDERR } "Usage: $0 -input_file=<input file> -output_file=<output_file>\n";
    exit 1;
}
my $class=$input_file;
my $version="";
if ($input_file =~ /([a-zA-Z][^_]*)(_(v[0-9]*))?\./) {
    $class=$1;
    $version=$2;
}

my $reg;

sub  trim { my $s = shift; $s =~ s/^\s+|\s+$//g; return $s };

my $has_params = 0;
my $has_configparams = 0;
my $has_statusparams = 0;
my $has_counterparams = 0;

my $INFILE, $OUTFILE;

open $INFILE, "<", $input_file or die $!;
foreach $line ( <$INFILE> ) {
    if    ($line =~ /^\s*createParam/)        { $has_params = 1; }
    elsif ($line =~ /^\s*createConfigParam/)  { $has_configparams = 1; }
    elsif ($line =~ /^\s*createStatusParam/)  { $has_statusparams = 1; }
    elsif ($line =~ /^\s*createCounterParam/) { $has_configparams = 1; }
}
close $INFILE;

if (($has_params + $has_configparams + $has_statusparams + $has_configparams) == 0) {
    exit(0);
}

if (defined $output_file) {
    open($OUTFILE, ">$output_file") or die "Error opening output file $output_file";
} else {
    $OUTFILE = *STDOUT;
}

print $OUTFILE <<EOF;
/**
 * \\class ${class}
EOF

if ($has_params) {
print $OUTFILE <<EOF;
 *
 * Class ${class} provides following EPICS parameters.
 * | Parameter  | Type          | Init value | Description | Valid values |
 * | ---------- | ------------- | ---------- | ----------- | ------------ |
EOF
open $INFILE, "<", $input_file or die $!;
foreach $line ( <$INFILE> ) {
    chomp($line);
    if ($line =~ m/createParam *\( *"([a-zA-Z0-9_]+)" *, *asynParam([A-Za-z0-9]+) *, *\&[A-Za-z0-9_]+ *(, *(.*))?\) *; *(\/\/ *(.*))?$/) {
        my ($name,$type,$valblock,$initval,$descblock,$desc) = ($1,$2,$3,$4,$5,$6);
        my ($direction,$description,$values);
        if ($desc =~ m/(READ|WRITE) *- *([^\(]*)(\((.*)\))?/) {
            ($direction,$description,$valuesblock,$values) = ($1,$2,$3,$4);
        }
        if ($initval eq "") { $initval = "undef"; }
        $type = "$direction $type";
        printf $OUTFILE (" * | %-10s | %-13s | %-10s | %s | %s |\n", $name, $type, $initval, trim($description), $values);
    }
}
close $INFILE;

}
if ($has_statusparams) {

print $OUTFILE <<EOF;
 *
 * EPICS status read-only parameters for ${class} ${version}
 * Parameter name | Status register   | Description |
 * -------------- | ----------------- | ----------- |
EOF
open $INFILE, "<", $input_file or die $!;
foreach $line ( <$INFILE> ) {
    chomp($line);
    if ($line =~ m/createStatusParam *\( *"([a-zA-Z0-9_]+)" *, *(0x[0-9A-Fa-f]+) *, *([0-9]+) *, *([0-9]+).*\/\/ *(.*)$/) {
        my ($name,$offset,$width,$shift,$comment) = ($1,$2,$3,$4,$5);
        if ($width == 1) {
            $reg = sprintf("Reg %s Bit %d", $offset, $shift);
        } else {
            $reg = sprintf("Reg %s Bits %d:%d", $offset, $shift, $shift+$width-1);
        }
        printf $OUTFILE (" * %-14s | %-17s | %s\n", $name, $reg, $comment);
    }
}
close $INFILE;

}
if ($has_counterparams) {

print $OUTFILE <<EOF;
 *
 * EPICS counter read-only parameters for ${class} ${version}
 * Parameter name | init val | Counter register | Description |
 * -------------- | -------- | ---------------- | ----------- |
EOF
open $INFILE, "<", $input_file or die $!;
foreach $line ( <$INFILE> ) {
    chomp($line);
    if ($line =~ m/createCounterParam *\( *"([a-zA-Z0-9_]+)" *, *(0x[0-9A-Fa-f]+) *, *([0-9]+) *, *([0-9]+).*\/\/ *(.*)$/) {
        my ($name,$offset,$width,$shift,$comment) = ($1,$2,$3,$4,$5);
        if ($width == 1) {
            $reg = sprintf("Reg %s Bit %d", $offset, $shift);
        } else {
            $reg = sprintf("Reg %s Bits %d:%d", $offset, $shift, $shift+$width-1);
        }
        printf $OUTFILE (" * %-14s | %-17s | %s\n", $name, $reg, $comment);
    }
}
close $INFILE;

}
if ($has_configparams) {

print $OUTFILE <<EOF;
 *
 * EPICS config writable parameters for ${class} ${version}
 * Parameter name | init val | Config register  | Description |
 * -------------- | -------- | ---------------- | ----------- |
EOF
open $INFILE, "<", $input_file or die $!;
foreach $line ( <$INFILE> ) {
    chomp($line);
    if ($line =~ m/createConfigParam *\( *"([a-zA-Z0-9_]+)" *, *'([0-9A-Fa-f])' *, *(0x[0-9A-Fa-f]+) *, *([0-9]+) *, *([0-9]+) *, *([0-9]+).*\/\/ *(.*)$/) {
        my ($name,$section,$offset,$width,$shift,$val,$comment) = ($1,$2,$3,$4,$5,$6,$7);
        if ($width == 1) {
            $reg = sprintf("Reg %s Bit %d", $offset, $shift);
        } else {
            $reg = sprintf("Reg %s Bits %d:%d", $offset, $shift, $shift+$width-1);
        }
        printf $OUTFILE (" * %-14s | %8d | %-17s | %s\n", $name, $val, $reg, $comment);
    }
}
close $INFILE;

}

print $OUTFILE <<EOF;
 */
EOF

close $OUTFILE;
