#!/usr/local/bin/perl -w

use strict;

# size indexed by block.
my %sizes;
my $totalsize = 0;
my $maxsize = 0;
my $maxline = 0;

sub align {
    my $sz = shift;
#    return 8* ((($sz - 1) / 8) + 1);
    return $sz;
}

while(<>) {
    next if($. <= 4);
    chomp;
    my ($op, $id, $sz) = split;
    $sz = align($sz) unless !defined $sz;
    if($op eq 'f') {
        next if($id == -1);
        $totalsize -= $sizes{$id};
    }
    elsif($op eq 'a') {
        $sizes{$id} = $sz;
        $totalsize += $sizes{$id};
    }
    elsif($op eq 'r') {
        $sizes{$id} = 0 if !defined $sizes{$id};
        $totalsize -= $sizes{$id};
        $sizes{$id} = $sz;
        $totalsize += $sizes{$id};
    }
    if($totalsize > $maxsize) {
        $maxsize = $totalsize;
        $maxline = $.;
    }
}
print "high-water mark: $maxsize at line $maxline\n";
