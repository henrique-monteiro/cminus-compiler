#!/bin/bash
flex tiny.l
bison -d tiny.y
gcc -c *.c
gcc -o tiny *.o -l:libfl.a


