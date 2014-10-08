#!/usr/bin/python
import sys
import re

a = re.compile("^%")
with open(sys.argv[1],"r") as matrixMarketFile:
    for line in matrixMarketFile:
        if not a.match(line):
            fields = line.split(' ')
            print fields[0]+"_"+fields[1]+"_"+fields[2]
            sys.exit(-1)
