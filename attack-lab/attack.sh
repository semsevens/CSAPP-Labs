#!/usr/bin/env sh

./hex2raw < ctarget.l1.txt | ./ctarget -q

echo

./hex2raw < ctarget.l2.txt | ./ctarget -q

echo

./hex2raw < ctarget.l3.txt | ./ctarget -q

echo

./hex2raw < rtarget.l4.txt | ./rtarget -q

echo
