#!/usr/bin/env sh

./hex2raw < ctarget.l1.txt | ./ctarget -q

echo

./hex2raw < ctarget.l2.txt | ./ctarget -q

echo