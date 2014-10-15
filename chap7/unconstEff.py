# calculate raw data for cityscape model, given a set of parameters
#!/usr/bin/python2.7

import time
#    0 1  2  3   4  5 6  7
#c = [s,gt,ge,bt,be,de,ee,M]
c = [1.5, 3.91e-12, 6.31e-10, 6.40e-9, 5.88e-8, 2.03e-9, 162.67, 8589934592]
x = [0.0,0.0,0.0,0.0,0.0,0.0]
zi = [1.0,1.0,1.0,1.0,1.0,1.0]
dataX = []
dataZ = []

x[0] = c[2]
x[1] = c[4]*c[7]**(1.0-c[0])
x[2] = c[5]*c[7]*c[1]
x[3] = c[5]*c[3]*c[7]**(2.0-c[0])
x[4] = c[6]*c[1]
x[5] = c[6]*c[3]*c[7]**(1.0-c[0])
z = 100
ziInc = 1.25
Cperf = 1/sum(x)
print Cperf
xDivz = list(x)
prevMax = []
maxItem, maxIndex = max((maxItem,maxIndex) for (maxIndex, maxItem) in enumerate(xDivz))
prevMax.append(maxIndex)

dataX.append([])
dataX[-1].extend(xDivz)
dataZ.append([])
dataZ[-1].extend(zi)
dataZ[-1].append((1.0/sum(xDivz))/Cperf)

while((1.0/sum(xDivz)) < Cperf*z):
#    time.sleep(1)
    tmpMax, tmpMaxIndex = max((tmpMax,tmpMaxIndex) for (tmpMaxIndex, tmpMax) in enumerate(xDivz))
#    print tmpMax,xDivz[prevMax[-1]]
    if (tmpMax > xDivz[prevMax[-1]] and tmpMaxIndex not in prevMax):
        prevMax.append(tmpMaxIndex)
    for m in prevMax:
        zi[m] = zi[m]*ziInc
#    xDivz = [a/b for a,b in zip(xDivz,zi)]
    xDivz = [a/b for a,b in zip(x,zi)]
    dataX.append([])
    dataX[-1].extend(xDivz)
    dataZ.append([])
    dataZ[-1].extend(zi)
    dataZ[-1].append((1.0/sum(xDivz))/Cperf)
#    print prevMax
#    print 1/sum(xDivz)/1e9,Cperf*z
#    print zi

for row in dataZ:
    print ','.join(map(str,row))


