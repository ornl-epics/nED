#!/usr/bin/perl -s

# Parse lines like this and transform them into EPICS records:
# createConfigParam("LvdsRxNoEr5", 'E', 0x0,  1, 16, 0); // LVDS ignore errors (0=discard erronous packet,1=keep all packets)
# createStatusParam("UartByteErr", 0x0,  1, 29); // UART: Byte error              (0=no error,1=error)
# The name and description are truncd to match EPICS string specifications
#
# Usage: export2epics.pl [-cfg_rbv=1] < InputFile.cpp > OutputFile.cpp"

my $MAX_NAME_LEN      = 10; # 28 chars total, worst case prefix BL99A:Det:roc245:
my $MAX_DESC_LEN      = 28;

if (!defined $cfg_rbv || $cfg_rbv != 1) {
    $cfg_rbv = 0;
}

sub  trim { my $s = shift; $s =~ s/^\s+|\s+$//g; return $s };
sub trunc {
    my ($str, $max_len, $record_name, $field) = @_;
    $str = trim($str);
    if (length($str) > $max_len) {
        print { STDERR } ("WARN: Truncating $record_name record $field to $max_len chars\n");
    }
    return substr($str, 0, $max_len);
}

# Print bi or bo record
# 
# Params:
# * type - <bi|bo>
# * name - Record name, might get truncated
# * desc - Record description, might get truncated
# * valstr - Part of the comment inside the (), used to give fields a name
# * val - default value, might be undefined
# * autosave - when defined, make the record autosave-able
sub bibo {
    my $MAX_BO_xNAM_LEN = 20;

    my ($type, $name, $desc, $valstr, $val, $infofields) = @_;
    $type = ( $type eq "bi" ? "bi" : "bo" );
    my $io = ( $type eq "bi" ? "INP" : "OUT" );
    $name = trunc($name, $MAX_NAME_LEN, $name, "name");
    $desc = trunc($desc, $MAX_DESC_LEN, $name, "DESC");

    print ("record($type, \"\$(P)$name\")\n");
    print ("\{\n");
    if ($infofields =~ m/autosave/) {
        print ("    info(autosaveFields, \"VAL\")\n");
    }
    if ($infofields =~ m/access/) {
        print ("    field(ASG, \"BEAMLINE\")\n");
    }
    print ("    field(DESC, \"$desc\")\n");
    print ("    field(DTYP, \"asynInt32\")\n");
    print ("    field($io,  \"\@asyn(\$(PORT))$name\")\n");
    if ($type eq "bi") {
        print ("    field(SCAN, \"I/O Intr\")\n");
    } else {
        print ("    field(PINI, \"YES\")\n");
    }
    if (defined $val) {
        print ("    field(VAL,  \"$val\")\n");
    }
    if ($valstr =~ m/([0-9]+) *= *([^,]+), *([0-9]+) *= *(.+)$/) {
        my ($zval,$znam,$oval,$onam) = ($1,$2,$3,$4);
        if ($zval != 0) { my $temp=$znam; $znam=$onam; $onam=$temp; }
        $znam = trunc($znam, $MAX_BO_xNAM_LEN, $name, "ZNAM");
        $onam = trunc($onam, $MAX_BO_xNAM_LEN, $name, "ONAM");
        print ("    field(ZNAM, \"$znam\")\n");
        print ("    field(ONAM, \"$onam\")\n");
    }
    print ("\}\n");
}

# Print mbbi or mbbo record
# 
# Params:
# * type - <mbbi|mbbo>
# * name - Record name, might get truncated
# * desc - Record description, might get truncated
# * valstr - Part of the comment inside the (), used to give fields a name
# * val - default value, might be undefined
# * autosave - when defined, make the record autosave-able
sub mbbimbbo {
    my $MAX_MBBO_xNAM_LEN = 16;
    my @vals = ("ZRVL","ONVL","TWVL","THVL","FRVL","FVVL","SXVL","SVVL","EIVL","NIVL","TEVL","ELVL","TVVL","TTVL","FTVL","FFVL");
    my @nams = ("ZRST","ONST","TWST","THST","FRST","FVST","SXST","SVST","EIST","NIST","TEST","ELST","TVST","TTST","FTST","FFST");

    my ($type, $name, $desc, $valstr, $val, $infofields) = @_;
    $type = ( $type eq "mbbi" ? "mbbi" : "mbbo" );
    my $io = ( $type eq "mbbi" ? "INP" : "OUT" );
    $name = trunc($name, $MAX_NAME_LEN, $name, "name");
    $desc = trunc($desc, $MAX_DESC_LEN, $name, "DESC");

    print ("record($type, \"\$(P)$name\")\n");
    print ("\{\n");
    if ($infofields =~ m/autosave/) {
        print ("    info(autosaveFields, \"VAL\")\n");
    }
    if ($infofields =~ m/access/) {
        print ("    field(ASG, \"BEAMLINE\")\n");
    }
    print ("    field(DESC, \"$desc\")\n");
    print ("    field(DTYP, \"asynInt32\")\n");
    print ("    field($io,  \"\@asyn(\$(PORT))$name\")\n");
    if ($type eq "mbbi") {
        print ("    field(SCAN, \"I/O Intr\")\n");
    } else {
        print ("    field(PINI, \"YES\")\n");
    }
    if (defined $val) {
        print ("    field(VAL,  \"$val\")\n");
    }
    my $i=0;
    foreach (split(',', $valstr)) {
        my ($xval,$xnam) = split(/=/, $_);
        $xnam = trunc($xnam, $MAX_MBBO_xNAM_LEN, $name, $nams[$i]);
        print ("    field($vals[$i], \"$xval\")\n");
        print ("    field($nams[$i], \"$xnam\")\n");
        $i++;
    }
    print ("\}\n");
}

# Print longin or longout record
# 
# Params:
# * type - <longin|longout>
# * name - Record name, might get truncated
# * desc - Record description, might get truncated
# * valstr - Part of the comment inside the (), additional record instructions, like calc,prec,etc.
# * val - default value, might be undefined
# * autosave - when defined, make the record autosave-able
sub longinlongout {
    my ($type, $name, $desc, $valstr, $val, $infofields) = @_;
    $type = ( $type eq "longin" ? "longin" : "longout" );
    my $io = ( $type eq "longin" ? "INP" : "OUT" );
    $name = trunc($name, $MAX_NAME_LEN, $name, "name");
    $desc = trunc($desc, $MAX_DESC_LEN, $name, "DESC");

    print ("record($type, \"\$(P)$name\")\n");
    print ("\{\n");
    if ($infofields =~ m/autosave/) {
        print ("    info(autosaveFields, \"VAL\")\n");
    }
    if ($infofields =~ m/access/) {
        print ("    field(ASG,  \"BEAMLINE\")\n");
    }
    print ("    field(DESC, \"$desc\")\n");
    print ("    field(DTYP, \"asynInt32\")\n");
    print ("    field($io,  \"\@asyn(\$(PORT))$name\")\n");
    if ($type eq "longin") {
        print ("    field(SCAN, \"I/O Intr\")\n");
    } else {
        print ("    field(PINI, \"YES\")\n");
    }
    if (defined $val) {
        print ("    field(VAL,  \"$val\")\n");
    }
    if ($valstr =~ /calc:([^,]*)/) {
        print ("    field(FLNK, \"\$(P)${name}C\")\n");
        print ("\}\n");
        print ("record(calc, \"\$(P)${name}C\")\n");
        print ("\{\n");
        print ("    field(DESC, \"$desc\")\n");
        print ("    field(INPA, \"\$(P)$name\")\n");
        print ("    field(CALC, \"$1\")\n");
    }
    if ($valstr =~ /prec:([0-9]*)/) {
        print ("    field(PREC, \"$1\")\n");
    }
    if ($valstr =~ /unit:([^,]*)/) {
        print ("    field(EGU,  \"$1\")\n");
    }
    print ("\}\n");
}

# Extract matching lines from stdin
foreach $line ( <STDIN> ) {
    chomp($line);
    if ($line =~ m/createStatusParam *\( *"([a-zA-Z0-9_:]+)" *, *([0-9a-fA-FxX]+) *, *([0-9]+) *, *([0-9]+).*\/\/ *(.*)$/ ||
        $line =~ m/createCounterParam *\( *"([a-zA-Z0-9_:]+)" *, *([0-9a-fA-FxX]+) *, *([0-9]+) *, *([0-9]+).*\/\/ *(.*)$/) {
        my ($name,$offset,$width,$shift,$comment) = ($1,$2,$3,$4,$5);
        $comment =~ /^\s*([^\(]*)\(?(.*)\)?$/;
        my ($desc, $valstr) = ($1, $2);
        $valstr =~ s/\)$//;

        if ($valstr =~ /^range/) {
            longinlongout("longin", $name, $desc, $valstr);
        } elsif ($width == 1) {
            bibo("bi", $name, $desc, $valstr);
        } elsif ($width > 1 && $width < 15 && $valstr ne "") {
            mbbimbbo("mbbi", $name, $desc, $valstr);
        } else {
            longinlongout("longin", $name, $desc, $valstr);
        }
    }
    if ($line =~ m/createConfigParam *\( *"([a-zA-Z0-9_:]+)" *, *'([0-9A-F])' *, *([0-9a-fA-FxX]+) *, *([0-9]+) *, *([0-9]+) *, *([0-9]+).*\/\/ *(.*)$/) {
        my ($name,$section,$offset,$width,$shift,$val,$comment) = ($1,$2,$3,$4,$5,$6,$7);
        $comment =~ /^\s*([^\(]*)\(?(.*)\)?$/;
        my ($desc, $valstr) = ($1, $2);
        $valstr =~ s/\)$//;

        if ($valstr =~ /^range/) {
            my $type = ( $cfg_rbv == 1 ? "longin" : "longout" );
            longinlongout($type, $name, $desc, $valstr, $val, "autosave,access");
        } elsif ($width == 1) {
            my $type = ( $cfg_rbv == 1 ? "bi" : "bo" );
            bibo($type, $name, $desc, $valstr, $val, "autosave,access");
        } elsif ($width > 1 && $width < 15 && $valstr ne "") {
            my $type = ( $cfg_rbv == 1 ? "mbbi" : "mbbo" );
            mbbimbbo($type, $name, $desc, $valstr, $val, "autosave,access");
        } else {
            my $type = ( $cfg_rbv == 1 ? "longin" : "longout" );
            longinlongout($type, $name, $desc, $valstr, $val, "autosave,access");
        }
    }
}
