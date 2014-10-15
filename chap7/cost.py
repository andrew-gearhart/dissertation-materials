# calculate financial cost per task (and other metrics) given a fixed set of parameters
#!/usr/bin/python2.7

from math import floor,ceil

baselineNodeCost = 1943.96
nodeBasePower = 100.0
s = 3.0
n = 100000.0
F = 1*n**3
nicIdleFrac = 0.85
cUtil = 1.0
cEnergy = 1.89e-8
cLife = 1.57785e8

# 1.8,2,2.1,2.2,2.4,2.6,2.7,2.9Ghz...assumes peak throughput for two sockets
gTList = [4.34E-12,3.91E-12,3.72E-12,3.55E-12,3.26E-12,3.00E-12,2.89E-12,2.69E-12]
gEList = [5.16493E-10,6.30859E-10,6.00818E-10,5.73509E-10,6.36393E-10,5.8744E-10,6.39468E-10,6.18265E-10]
# assumed to be 15% of combined TDP of two processors
procIdlePowerList = [21,28.5,28.5,28.5,34.5,34.5,39,40.5]
# cost of two processors
procCostList = [2214.0,2214.0,2372.0,2658.0,2880.0,3104.0,3446.0,4114.0]
 
# 1066,1333,1600,1866 DDR3...assumes peak BW
dEList = [1.9448E-09,2.0264E-09,2.1072E-09]
memCostList = [1.20E-07,1.25E-07,1.08E-07]
memActivePowerList = [4.9368E-09,5.3832E-09,5.8424E-09]
memPeakBWList = [82812500,100000000,116406250] # per GB
# NICs assumed to have 2 ports, all cables are of length 5m, and NIC runs at TDP (a big assumption)
# 1Gb/s,10Gb/s,40Gb/s
bTList = [6.40E-08,6.40E-09,1.60E-09]
nicTDPList = [4.4,13.4,17.0]
nicCostList = [128.0,508.0,1023.49]
linkCostList = [3.4,7.32,154.0]

def costFunc(p,d,M,baseline,procCost,memCost,nicCost,linkCost):
    return p*( baseline + procCost + memCost*M + d*(nicCost+linkCost) )

def wordBoundTime(F,p,M,s,d):
    return max(F/(p*M**(s-1)),(F/p)**(1/s),F**(1/s)/p**(1-1/d))
def wordBoundEnergy(F,p,M,s,d):
    return max(F/(p*M**(s-1)),(F/p)**(1/s))

def timeFunc(F,s,p,d,M,gT,bT):
    W = wordBoundTime(F,p,M,s,d)
  #  print F,gT,p,bT,W,(gT*F)/p,bT*W
    return gT*F/p + bT*W

def energyFunc(F,s,p,d,M,gE,bE,dE,eE,T):
    We = wordBoundEnergy(F,p,M,s,d)
    return p*(gE*F/p + bE*We + (dE*M+eE)*T)

for p in range(1000,10000,1000):
# NOTE: THIS is specific to matmul!!!
#    Mlower = (int)(ceil((3.0*(n**2.0))/p))
#    Mupper = (int)(floor((3.0*(n**2.0))/(p**(2.0/3.0))))
#    Minc = (int)(floor((Mupper-Mlower)/10.0))
#    for M in range (Mlower,Mupper,Minc):

# installed memory, or utilized memory...installed modifies cost...
    for MGb in range(16,16*9,16):
        M = (MGb*1000000000)/8
        for d in range(1,5):
            for gT,gE,procIdlePower,procCost in zip(gTList,gEList,procIdlePowerList,procCostList):
                for dE,memCost,memActivePower,memPeakBW in zip(dEList,memCostList,memActivePowerList,memPeakBWList):
                    for bT,nicPower,nicCost,linkCost in zip(bTList,nicTDPList,nicCostList,linkCostList):
                        cost = costFunc(p,d,M,baselineNodeCost,procCost,memCost,nicCost,linkCost)
                        bT = bT/d # peak network bandwidth is actually single NIC BW*numNICs
                        time = timeFunc(F,s,p,d,M,gT,bT)
                        eE = procIdlePower + nodeBasePower + nicPower*nicIdleFrac*d
                        memActivePower = memActivePower*((1/bT)/(memPeakBW*MGb))
                        bE = bT*(memActivePower*M + nicPower*d*(1-nicIdleFrac))
                        energy = energyFunc(F,s,p,d,M,gE,bE,dE,eE,time)
                        numJobs = cUtil*cLife/time
                        capExPerJob = (cost*time)/(cUtil*cLife)
                        opExPerJob = energy*cEnergy
                        costPerJob = capExPerJob + opExPerJob
                       # print gT,gE,bT,bE,dE,eE
                        print p,M,d,gT,gE,bT*d,bE,dE,eE,time,energy,cost,numJobs,p*baselineNodeCost,p*procCost,p*memCost*M,p*d*(nicCost+linkCost),capExPerJob,opExPerJob,costPerJob
