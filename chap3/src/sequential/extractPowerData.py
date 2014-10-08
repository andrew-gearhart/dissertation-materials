#!/usr/bin/python

# the blocks and non-blocked outputs are different...ugh

import sys
import re
import numpy
from os import listdir
from os.path import isfile,join

dataDir="POWER_DATA"
chopPowerValues=3

def isfloat(x):
    try:
        a = float(x)
    except ValueError:
        return False
    else:
        return True

def isint(x):
    try:
        a = float(x)
        b = int(a)
    except ValueError:
        return False
    else:
        return a == b

p = re.compile('counterData|#')
p2 = re.compile('error')

totalArgs = len(sys.argv)

directory = sys.argv[1]
runType = sys.argv[2]

runDataDir = dataDir+"/"+directory
counterFileName = runDataDir+"/"+directory+"_counterData.txt"
print counterFileName
counterOutName = runDataDir+"/"+directory+"_counterDataPower.txt"
counterOutFile = open(counterOutName,"w")

with open(counterFileName,"r") as counterFile:
    filesInDir = [f for f in listdir(runDataDir) if isfile(join(runDataDir,f)) and not p.search(f)]
    files = {}
    for f in filesInDir:
        tmp = f.split('_')
        key = tmp[-3]+"_"+tmp[-2]+"_"+tmp[-1].split(".")[0]
        files[ key ] = f
    # for each line in the counter file...
    for counterDataLine in counterFile:
        counterDataLine = counterDataLine.rstrip()
        if not counterDataLine: continue
        counterDataFields = counterDataLine.split(',')
        ts1 = counterDataFields[0]
        ts2 = counterDataFields[1]
#        print "%s,%s" % (ts1,ts2)
        # in all types of runs, the last 3 numbers of the power output file determine the run correlation
        # however, the counterFile output does change...so we need to know if it's a blocked run
        M = counterDataFields[3]
        N = counterDataFields[4]
        K = counterDataFields[5]
        if (runType == "blocked"):
            BM = counterDataFields[6]
            BN = counterDataFields[7]
            BK = counterDataFields[8]
            runIDString = BM+"_"+BN+"_"+BK
        else:
            runIDString = M+"_"+N+"_"+K
        powerFileName =  runDataDir+"/"+files[runIDString]
        with open(powerFileName,"r") as powerFile:
            print "RunID: " + runIDString
            inTest = 0
            powerValues = []
            for powerLine in powerFile:
                fields = powerLine.split(' ')
                if (fields[0] == ts1):
                    inTest = 1;
                if (inTest == 1):
#                    powerValues.append(fields[1][:-1])
                    if (isfloat(fields[1][:-1])):
                        powerValues.append(float(fields[1][:-1]))
                if (fields[0] == ts2):
                    inTest = 0;
            powerValues = powerValues[(chopPowerValues):]
            if (len(powerValues) > 0):
                powerValues = numpy.array(powerValues)
                mean = numpy.mean(powerValues)
                stddev = numpy.std(powerValues)
            else:
                mean = 0
                stddev = 0
            counterOutFile.write(counterDataLine+str(mean)+","+str(stddev)+","+str(len(powerValues))+"\n")

counterOutFile.close()
