#!/usr/bin/perl -s

# Parse lines like this and transform them into EPICS records:
# createConfigParam("LvdsRxNoEr5", 'E', 0x0,  1, 16, 0); // LVDS ignore errors (0=discard erronous packet,1=keep all packets)
# createStatusParam("UartByteErr", 0x0,  1, 29); // UART: Byte error              (0=no error,1=error)
# The name and description are truncd to match EPICS string specifications
#
# asyn device support doesn't support bi-directional EPICS records. We use a trick
# from Mark Rivers (http://www.aps.anl.gov/epics/tech-talk/2014/msg00057.php)
# and create 3 additional records to support reading and writing to a single
# record.
#
# Record names are limited to 10 characters. Based on given fixed prefix of 17
# characters, there's 1 left which we use fo dynamicly created records:
# - C for CALC record
# - W for write out record
# - R for read in record
# - S for sync record
#
# Usage: export2epics.pl < InputFile.cpp > OutputFile.include"

my $MAX_NAME_LEN      = 30; # Worst case prefix BL99A:Det:roc245:, total is 60
my $MAX_DESC_LEN      = 28;

sub  trim { my $s = shift; $s =~ s/^\s+|\s+$//g; return $s };
sub trunc {
    my ($str, $max_len, $record_name, $field) = @_;
    $str = trim($str);
    if (length($str) > $max_len) {
        print { STDERR } ("WARN: Truncating $record_name record $field to $max_len chars\n");
    }
    return substr($str, 0, $max_len);
}

sub _inout_common {
    my ($type, $name, $desc, $valstr, $val) = @_;
    $name = trunc($name, $MAX_NAME_LEN, $name, "name");
    $desc = trunc($desc, $MAX_DESC_LEN, $name, "DESC");

    my $intype = "";
    if    ($type eq "bo")      { $intype = "bi"; }
    elsif ($type eq "mbbo")    { $intype = "mbbi"; }
    elsif ($type eq "longout") { $intype = "longin"; }
    else { return; }

    print ("record($type, \"\$(P)$name\")\n");
    print ("\{\n");
    print ("    info(autosaveFields, \"VAL\")\n");
    print ("    field(ASG,  \"BEAMLINE\")\n");
    print ("    field(DESC, \"$desc\")\n");
    print ("    field(PINI, \"YES\")\n");
    print ("    field(VAL,  \"$val\")\n");
    print ("    field(OUT,  \"\$(P)${name}W PP\")\n");
    if    ($type eq "bo")      { _bibo_val($name, $valstr); }
    elsif ($type eq "mbbo")    { _mbbimbbo_val($name, $valstr); }
    elsif ($type eq "longout") { _longinlongout_val($name, $valstr); }
    print ("\}\n");

    print ("record($type, \"\$(P)${name}W\")\n");
    print ("\{\n");
    print ("    field(ASG,  \"BEAMLINE\")\n");
    print ("    field(DESC, \"$desc\")\n");
    print ("    field(DTYP, \"asynInt32\")\n");
    print ("    field(OUT,  \"\@asyn(\$(PORT))$name\")\n");
    print ("    field(SDIS, \"\$(P)${name}S.PACT\")\n");
    print ("    field(DISV, \"1\")\n");
    if    ($type eq "bo")      { _bibo_val("${name}W", $valstr); }
    elsif ($type eq "mbbo")    { _mbbimbbo_val("${name}W", $valstr); }
    elsif ($type eq "longout") { _longinlongout_val("${name}W", $valstr); }
    print ("\}\n");

    print ("record($intype, \"\$(P)${name}R\")\n");
    print ("\{\n");
    print ("    field(DTYP, \"asynInt32\")\n");
    print ("    field(DESC, \"$desc\")\n");
    print ("    field(INP,  \"\@asyn(\$(PORT))$name\")\n");
    print ("    field(SCAN, \"I/O Intr\")\n");
    print ("    field(FLNK, \"\$(P)${name}S\")\n");
    if    ($type eq "bo")      { _bibo_val("${name}R", $valstr); }
    elsif ($type eq "mbbo")    { _mbbimbbo_val("${name}R", $valstr); }
    elsif ($type eq "longout") { _longinlongout_val("${name}R", $valstr); }
    print ("\}\n");

    print ("record($type, \"\$(P)${name}S\")\n");
    print ("\{\n");
    print ("    field(DOL,  \"\$(P)${name}R NPP\")\n");
    print ("    field(OMSL, \"closed_loop\")\n");
    print ("    field(OUT,  \"\$(P)$name PP\")\n");
    if    ($type eq "bo")      { _bibo_val("${name}S", $valstr); }
    elsif ($type eq "mbbo")    { _mbbimbbo_val("${name}S", $valstr); }
    elsif ($type eq "longout") { _longinlongout_val("${name}S", $valstr); }
    print ("\}\n");
}

sub _in_common {
    my ($type, $name, $desc, $valstr) = @_;
    $name = trunc($name, $MAX_NAME_LEN, $name, "name");
    $desc = trunc($desc, $MAX_DESC_LEN, $name, "DESC");

    print ("record($type, \"\$(P)$name\")\n");
    print ("\{\n");
    print ("    field(DESC, \"$desc\")\n");
    print ("    field(DTYP, \"asynInt32\")\n");
    print ("    field(INP,  \"\@asyn(\$(PORT))$name\")\n");
    print ("    field(SCAN, \"I/O Intr\")\n");
    if    ($type eq "bi")     { _bibo_val($name, $valstr); }
    elsif ($type eq "mbbi")   { _mbbimbbo_val($name, $valstr); }
    elsif ($type eq "longin") { _longinlongout_val($name, $valstr); }
    print ("\}\n");
}

sub _bibo_val {
    my $MAX_BO_xNAM_LEN = 20;

    my ($name, $valstr) = @_;
    if ($valstr =~ m/([0-9]+) *= *([^,]+), *([0-9]+) *= *(.+)$/) {
        my ($zval,$znam,$oval,$onam) = ($1,$2,$3,$4);
        if ($zval != 0) { my $temp=$znam; $znam=$onam; $onam=$temp; }
        $znam = trunc($znam, $MAX_BO_xNAM_LEN, $name, "ZNAM");
        $onam = trunc($onam, $MAX_BO_xNAM_LEN, $name, "ONAM");
        print ("    field(ZNAM, \"$znam\")\n");
        print ("    field(ONAM, \"$onam\")\n");
    }
}

# Print bi record
# 
# Params:
# * name - Record name, might get truncated
# * desc - Record description, might get truncated
# * valstr - Part of the comment inside the (), used to give fields a name
sub bi {
    _in_common("bi", @_);
}

# Print bo record
# 
# Params:
# * name - Record name, might get truncated
# * desc - Record description, might get truncated
# * valstr - Part of the comment inside the (), used to give fields a name
# * val - Record default value, just for first time, should be maintained by autosave afterwards
sub bo {
    _inout_common("bo", @_);
}

sub _mbbimbbo_val {
    my $MAX_MBBO_xNAM_LEN = 16;
    my @vals = ("ZRVL","ONVL","TWVL","THVL","FRVL","FVVL","SXVL","SVVL","EIVL","NIVL","TEVL","ELVL","TVVL","TTVL","FTVL","FFVL");
    my @nams = ("ZRST","ONST","TWST","THST","FRST","FVST","SXST","SVST","EIST","NIST","TEST","ELST","TVST","TTST","FTST","FFST");

    my ($name, $valstr) = @_;

    my $i=0;
    foreach (split(',', $valstr)) {
        my ($xval,$xnam) = split(/=/, $_);
        $xnam = trunc($xnam, $MAX_MBBO_xNAM_LEN, $name, $nams[$i]);
        print ("    field($vals[$i], \"$xval\")\n");
        print ("    field($nams[$i], \"$xnam\")\n");
        $i++;
    }
}

# Print mbbi record
# 
# Params:
# * name - Record name, might get truncated
# * desc - Record description, might get truncated
# * valstr - Part of the comment inside the (), used to give fields a name
sub mbbi {
    _in_common("mbbi", @_);
}

# Print mbbo record
# 
# Params:
# * name - Record name, might get truncated
# * desc - Record description, might get truncated
# * valstr - Part of the comment inside the (), used to give fields a name
# * val - Record default value, just for first time, should be maintained by autosave afterwards
sub mbbo {
    _inout_common("mbbo", @_);
}

sub _longinlongout_val {
    my ($name, $valstr) = @_;

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
}

# Print longin record
# 
# Params:
# * name - Record name, might get truncated
# * desc - Record description, might get truncated
# * valstr - Part of the comment inside the (), additional record instructions, like calc,prec,etc.
sub longin {
    _in_common("longin", @_);
}

# Print longout record
# 
# Params:
# * name - Record name, might get truncated
# * desc - Record description, might get truncated
# * valstr - Part of the comment inside the (), additional record instructions, like calc,prec,etc.
# * val - Record default value, just for first time, should be maintained by autosave afterwards
sub longout {
    _inout_common("longout", @_);
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
            longin($name, $desc, $valstr);
        } elsif ($width == 1 && $valstr ne "") {
            bi($name, $desc, $valstr);
        } elsif ($width > 1 && $width < 15 && $valstr ne "") {
            mbbi($name, $desc, $valstr);
        } else {
            longin($name, $desc, $valstr);
        }
    }
    if ($line =~ m/createConfigParam *\( *"([a-zA-Z0-9_:]+)" *, *'([0-9A-F])' *, *([0-9a-fA-FxX]+) *, *([0-9]+) *, *([0-9]+) *, *(-?[0-9]+).*\/\/ *(.*)$/) {
        my ($name,$section,$offset,$width,$shift,$val,$comment) = ($1,$2,$3,$4,$5,$6,$7);
        $comment =~ /^\s*([^\(]*)\(?(.*)\)?$/;
        my ($desc, $valstr) = ($1, $2);
        $valstr =~ s/\)$//;

        if ($valstr =~ /^range/) {
            longout($name, $desc, $valstr, $val);
        } elsif ($width == 1 && $valstr ne "") {
            bo($name, $desc, $valstr, $val);
        } elsif ($width > 1 && $width < 15 && $valstr ne "") {
            mbbo($name, $desc, $valstr, $val);
        } else {
            longout($name, $desc, $valstr, $val);
        }
    }
    if ($line =~ m/createConfigParamEx *\( *"([a-zA-Z0-9_:]+)" *, *'([0-9A-F])' *, *([0-9a-fA-FxX]+) *, *([0-9]+) *, *([0-9]+) *, *(-?[0-9]+).*\/\/ *(.*)$/) {
        my ($name,$section,$offset,$width,$shift,$val,$comment) = ($1,$2,$3,$4,$5,$6,$7);
        $comment =~ /^\s*([^\(]*)\(?(.*)\)?$/;
        my ($desc, $valstr) = ($1, $2);
        $valstr =~ s/\)$//;

        if ($valstr =~ /^range/) {
            longout($name, $desc, $valstr, $val);
        } elsif ($width == 1 && $valstr ne "") {
            bo($name, $desc, $valstr, $val);
        } elsif ($width > 1 && $width < 15 && $valstr ne "") {
            mbbo($name, $desc, $valstr, $val);
        } else {
            longout($name, $desc, $valstr, $val);
        }
   }
}
