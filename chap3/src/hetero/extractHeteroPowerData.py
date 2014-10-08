#!/usr/bin/python

import sys
import re
import numpy
from os import listdir
from os.path import isfile,join

dataDir="POWER_DATA"
chopPowerValues=4

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

runDataDir = dataDir+"/"+directory
counterFileName = runDataDir+"/"+directory+"_counterData.txt"
print counterFileName
counterOutName = runDataDir+"/"+directory+"_counterDataPower.txt"
counterOutFile = open(counterOutName,"w")

def calculateTwoPhase(powerFileName,counterDataLine,cpuStart,cpuEnd,gpuStart,gpuEnd,ts,finets):
    tmp = {ts[0]:cpuStart,ts[1]:cpuEnd,ts[2]:gpuStart,ts[3]:gpuEnd}    

    phases2 = sorted(finets)
    phaseString = ','.join(map(str, phases2))

    with open(powerFileName,"r") as powerFile:
        inCPUTest = 0
        inGPUTest = 0
        powerValuesCPU = []
        powerValuesGPU = []

        for powerLine in powerFile:
            fields = powerLine.split(' ')
            if (fields[0] == ts[0]):
                inCPUTest = 1
            if (fields[0] == ts[2]):
                inGPUTest = 1
            if (inCPUTest == 1):
                if (isfloat(fields[1][:-1])):
                    powerValuesCPU.append(float(fields[1][:-1]))
            if (inGPUTest == 1):
                if (isfloat(fields[1][:-1])):
                    powerValuesGPU.append(float(fields[1][:-1]))
            if (fields[0] == ts[1]):
                inCPUTest = 0;
            if (fields[0] == ts[3]):
                inGPUTest = 0;

        powerValuesCPU = powerValuesCPU[(chopPowerValues):]
        if (len(powerValuesCPU) > 0):
            powerValuesCPU = numpy.array(powerValuesCPU)
            meanCPU = numpy.mean(powerValuesCPU)
            stddevCPU = numpy.std(powerValuesCPU)
        else:
            meanCPU = 0
            stddevCPU = 0

        powerValuesGPU = powerValuesGPU[(chopPowerValues):]
        if (len(powerValuesGPU) > 0):
            powerValuesGPU = numpy.array(powerValuesGPU)
            meanGPU = numpy.mean(powerValuesGPU)
            stddevGPU = numpy.std(powerValuesGPU)
        else:
            meanGPU = 0
            stddevGPU = 0
        # add dummy zeros for an empty third phase so that output formats are identical
        counterOutFile.write(counterDataLine+","+str(meanCPU)+","+str(stddevCPU)+","+str(len(powerValuesCPU))+","+str(meanGPU)+","+str(stddevGPU)+","+str(len(powerValuesGPU))+",0,0,0,"+phaseString+"\n")

def calculateThreePhase(powerFileName,counterDataLine,cpuStart,cpuEnd,gpuStart,gpuEnd,ts,finets):
    # WARNING: Duplicate timestamps are eliminated here..
    tmp = {ts[0]:cpuStart,ts[1]:cpuEnd,ts[2]:gpuStart,ts[3]:gpuEnd}
    phases = sorted(tmp,key=lambda key:tmp[key])
    phases2 = sorted(finets)
    phaseString = ','.join(map(str, phases2))
    with open(powerFileName,"r") as powerFile:
        inPhase1 = 0
        inPhase2 = 0
        inPhase3 = 0
        powerValuesP1 = []
        powerValuesP2 = []
        powerValuesP3 = []

        for powerLine in powerFile:
            fields = powerLine.split(' ')

            if (fields[0] == phases[0]):
                inPhase1 = 1
            # first clause in check is to make sure the GPU and CPU runs aren't perfectly aligned into a single phase
            if (len(phases) > 2 and fields[0] == phases[1]):
                inPhase2 = 1
            # first clause in check is to make sure the GPU and CPU runs aren't aligned at the start or end into two phases, not three
            if (len(phases) > 3 and fields[0] == phases[2]):
                inPhase3 = 1

            if (inPhase1 == 1):
                if (isfloat(fields[1][:-1])):
                    powerValuesP1.append(float(fields[1][:-1]))
            if (inPhase2 == 1):
                if (isfloat(fields[1][:-1])):
                    powerValuesP2.append(float(fields[1][:-1]))
            if (inPhase3 == 1):
                if (isfloat(fields[1][:-1])):
                    powerValuesP3.append(float(fields[1][:-1]))

            if (fields[0] == phases[1]):
                inPhase1 = 0;
            if (len(phases) > 2 and fields[0] == phases[2]):
                inPhase2 = 0;
            if (len(phases) > 3 and fields[0] == phases[3]):
                inPhase3 = 0;

        powerValuesP1 = powerValuesP1[(chopPowerValues):]
        powerValuesP2 = powerValuesP2[(chopPowerValues):]
        powerValuesP3 = powerValuesP3[(chopPowerValues):]

        if (len(powerValuesP1) > 0):
            powerValuesP1 = numpy.array(powerValuesP1)
            meanP1 = numpy.mean(powerValuesP1)
            stddevP1 = numpy.std(powerValuesP1)
        else:
            meanP1 = 0
            stddevP1 = 0
        if (len(powerValuesP2) > 0):
            powerValuesP2 = numpy.array(powerValuesP2)
            meanP2 = numpy.mean(powerValuesP2)
            stddevP2 = numpy.std(powerValuesP2)
        else:
            meanP2 = 0
            stddevP2 = 0
        if (len(powerValuesP3) > 0):
            powerValuesP3 = numpy.array(powerValuesP3)
            meanP3 = numpy.mean(powerValuesP3)
            stddevP3 = numpy.std(powerValuesP3)
        else:
            meanP3 = 0
            stddevP3 = 0

        # add dummy zeros for an empty third phase so that output formats are identical
        counterOutFile.write(counterDataLine+","+str(meanP1)+","+str(stddevP1)+","+str(len(powerValuesP1))+","+str(meanP2)+","+str(stddevP2)+","+str(len(powerValuesP2))+","+str(meanP3)+","+str(stddevP3)+","+str(len(powerValuesP3))+","+phaseString+"\n")
    

with open(counterFileName,"r") as counterFile:
    # list of all wall power traces in directory, and not the counterData file
    filesInDir = [f for f in listdir(runDataDir) if isfile(join(runDataDir,f)) and not p.search(f)]
    files = {}

    # get unique run identifier from within the wall power traces 
    for f in filesInDir:
        tmp = f.split('_')
        # key is now "Mh_Nh_Kh_Md_Nd_Kd"
        key = tmp[-6]+"_"+tmp[-5]+"_"+tmp[-4]+"_"+tmp[-3]+"_"+tmp[-2]+"_"+tmp[-1].split(".")[0]
        files[ key ] = f

    # for each line in the counter file...
    for counterDataLine in counterFile:
        counterDataLine = counterDataLine.rstrip()
        if not counterDataLine: continue
        counterDataFields = counterDataLine.split(',')

        # get the intervals for the various time stamps, avoiding the initialization intervals (which could be added later)
        ts = []
        ts.append(counterDataFields[2]) # CPU start
        ts.append(counterDataFields[3]) # CPU end
        ts.append(counterDataFields[6]) # GPU start
        ts.append(counterDataFields[7]) # GPU end

        finets = []
        finets.append(float(counterDataFields[22]))
        finets.append(float(counterDataFields[23]))
        finets.append(float(counterDataFields[26]))
        finets.append(float(counterDataFields[27]))


        # need to sort the timestamps...so, convert them to integers
        cpuStart = int(ts[0][1]+ts[0][2]+ts[0][4]+ts[0][5]+ts[0][7]+ts[0][8])
        cpuEnd = int(ts[1][1]+ts[1][2]+ts[1][4]+ts[1][5]+ts[1][7]+ts[1][8])
        gpuStart = int(ts[2][1]+ts[2][2]+ts[2][4]+ts[2][5]+ts[2][7]+ts[2][8])
        gpuEnd = int(ts[3][1]+ts[3][2]+ts[3][4]+ts[3][5]+ts[3][7]+ts[3][8])

        # identify the correct wall power trace file
        Mh = counterDataFields[8]
        Nh = counterDataFields[9]
        Kh = counterDataFields[10]
        Md = counterDataFields[11]
        Nd = counterDataFields[12]
        Kd = counterDataFields[13]
        runIDString = Mh+"_"+Nh+"_"+Kh+"_"+Md+"_"+Nd+"_"+Kd
        powerFileName =  runDataDir+"/"+files[runIDString]

        print runIDString

        # if the two devices have non-overlapping runtimes, we want to measure them seperately
        if (cpuEnd < gpuStart or gpuEnd < cpuStart):
            calculateTwoPhase(powerFileName,counterDataLine,cpuStart,cpuEnd,gpuStart,gpuEnd,ts,finets)
        # otherwise, need to sort the timestamps and calculate the average powers of each phase
        else:
            calculateThreePhase(powerFileName,counterDataLine,cpuStart,cpuEnd,gpuStart,gpuEnd,ts,finets)

counterOutFile.close()
