#!/usr/bin/perl
#
# mkconfigh.pl - Create "config.h" file from "config.mk" file.
#
# Copyright (C) 2005 Hiroshi DOYU <Hiroshi.DOYU@montavista.co.jp>
#
#

use strict;

my @lines;
my @keys;
my %mkhash;
my $i;

# follow include and set each CONFIG_* in @lines
sub set_lines {
    my ($mk) = @_;
    open(my $fh, "< $mk"), or warn "$!: $mk\n";
    while (<$fh>) {
	next if (/^s*$/ || /^\#\#\#/);
	chomp;
	if (/^include\s+(config\/(\S+))/) {
	    set_lines($1);
	    next;
	}
	push @lines, $_;
    }
    close($fh);

# DEBUG
#    for (@lines) {
#	print "$_\n";
#    }
}

set_lines(shift @ARGV);

# uniq it in hash
for (@lines) {
    /(\w+)=\s*(.*)/;
    push @keys, $1 if (!exists $mkhash{$1});
    $mkhash{$1} = $2;
}

# print hash
foreach $i (@keys) {
    my $v = $mkhash{$i};
    if ($v =~ "y") {
 	print "#define $i\t1\n";
    } elsif ($v =~ "n") {
	next;
    } else {
 	print "#define $i\t$v\n";
    }
}
