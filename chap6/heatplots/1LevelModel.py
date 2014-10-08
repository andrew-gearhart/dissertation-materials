# Andrew Gearhart 10/21/13
# UC Berkeley
#
# This code implements the basic one-level model of machine runtime and energy assuming
# a distributed heterogeneous environment

from scipy import interpolate
from scipy.optimize import *
from modelCommonFuncs import *
from pylab import *
# Calculate parameters from machine characteristics for 1-Level model
def MEnergyHBL(F,Na,s,p,E,pa):
    M = Na/p
    A = pa['gamma_e']*F
    B = pa['beta_e']*F/M**(s-1.0)
    C = pa['delta_e']*M*pa['gamma_t']*F
    D = pa['delta_e']*pa['beta_t']*F/M**(s-2.0)
    EE = pa['epsilon_e']*pa['gamma_t']*F
    FF = pa['epsilon_e']*pa['beta_t']*F/M**(s-1.0)
    Ap = A + EE - E
    Bp = B + FF

    zeros = fsolve(lambda x: C*x*x*x + Ap*x*x + D*x + Bp,[0.001, 0.001, 0.001])
    return zeros[0]

def MTimeHBL(F,Na,s,p,T,pa):
    num = -pa['beta_t']*F*p**(s-1.)*Na**(1.-s)
    denom = -T*p + pa['gamma_t']*F
  #  print num,denom,(num/denom)**(1.0/(s-1.0))
    return (num/denom)**(1.0/(s-1.0))

def calculateParameters(machine):
# xeon nodes
#    params['gamma_t'] = 3.91e-12
#    params['gamma_e'] = 6.31e-10 
#    params['beta_t'] = 6.4e-9
#    params['beta_e'] = 5.88e-8
#    params['delta_e'] = 2.03e-9
#    params['epsilon_e'] = 162.67
# future nodes
    params['gamma_t'] = 3.13e-13
    params['gamma_e'] = 2.39e-11 
    params['beta_t'] = 1.6e-9
    params['beta_e'] = 1.97e-8
    params['delta_e'] = 1.02e-9
    params['epsilon_e'] = 97.67

    print params
    return params
    
### BEGIN ANALYSIS FUNCTIONS ###
# HBL example
# scaling M on each node...with replication
def n_vs_c_withFixed_p_HBL(params):
    ret = {}
    Ecrazy = 0
    Tcrazy = 0
    # anything over maxReplication falls under the memory-independent lower bound...
# matmul
#    p = 1000
#    maxReplication = int(floor(pow(p,1.0/3.0)))
    # max memory usage must be less than installed memory used to calculate DRAM parameters!!! (128GB)
#    maxN = 800000
#    Narray = np.arange(125000,maxN,5000,dtype=np.uint64)
##    maxReplication = 10
##    maxN = 10000000
##    Narray = np.arange(50000,maxN,50000,dtype=np.uint64)


#nbody
#    p = 1000
#    maxReplication = 10
#    maxN = 4000000
#    Narray = np.arange(5000,maxN,5000,dtype=np.uint64)
# HBL example
    p = 1000
    maxReplication = 10
    maxN = 50000
    Narray = np.arange(250,maxN,250,dtype=np.uint64)

    carray = np.arange(1,maxReplication,1,dtype=np.uint64)
    maxN = len(Narray)
    maxc = len(carray)
    efficiency = np.zeros((maxc,maxN), dtype=np.float64)
    T = np.zeros((maxc,maxN),dtype=np.float64)
    E = np.zeros((maxc,maxN),dtype=np.float64)
    constE = np.zeros((maxN),dtype=np.float64)
    constT = np.zeros((maxN),dtype=np.float64)
    repLimit = np.zeros((maxN),dtype=np.float64)

    Tcomp = np.zeros((maxc,maxN),dtype=np.float64)
    Tcomm = np.zeros((maxc,maxN),dtype=np.float64)
    Estatic = np.zeros((maxc,maxN),dtype=np.float64)
    Edyn = np.zeros((maxc,maxN),dtype=np.float64)

    F = np.dtype(np.float64)
    M = np.dtype(np.float64)
    W = np.dtype(np.float64)

    i = 0
    flagT = 0
    flagE = 0
    for N in Narray:
        # size of the problem in memory
# matmul
#        Na = 3*N*N
#        f = 1
#        F = (f+1)*N*N*N
#        s = 3.0/2.0
#        energyBound = 10000.
#        timeBound = 50

# nbody
#        Na = 2*N
#        f = 10
#        F = (f+1)*N*N
#        s = 2.0
##        energyBound = 1000
##        timeBound = 0.05
#        energyBound = 5000
#        timeBound = 0.02

        # HBL example
        Na = 2*N
        f = 0
        F = (f+1)*N*N*N
        s = 3.0
        energyBound = 5000
        timeBound = 0.01

        memDepMax = int(floor( (F**(1./s)*p**(1.0-1.0/s))/Na))
        repLimit[i] = memDepMax
        j = 0
        # calculate any lines of constant energy or runtime for the plot
        constE[i] = MEnergyHBL(F,Na,s,p,energyBound,params)
        constT[i] = MTimeHBL(F,Na,s,p,timeBound,params)
        # eliminate asymptotes in the plot...
        if (constE[i] < 0 or np.isnan(constE[i]) or flagE == 1):
            constE[i] = maxReplication+1
            if (flagE == 0):
                Ecrazy = i
            flagE = 1
        if (constT[i] < 0 or np.isnan(constT[i]) or flagT == 1):
            constT[i] = maxReplication+1
            if (flagT == 0):
                Tcrazy = i
            flagT = 1
       # print constT[i]
        for c in carray:
            M = (c*Na)/p
            if (c > memDepMax):
# NBody!!!
#                W = (F/(p*(f+1)))**(1./s)
# matmul and HBL
                W = (F/p)**(1./s)
            else:
# NBody!!!
#                W = F/(p*(f+1)*M**(s-1))
# matmul and HBL
                W = F/(p*M**(s-1))
            if (N == 795000):
                print M*8/1e9
            T[j][i] = params['gamma_t']*F/p + params['beta_t']*W 
         #   T[j][i] = max(params['gamma_t']*F,params['beta_t']*W) 
            E[j][i] = p*(params['gamma_e']*F/p + params['beta_e']*W + params['delta_e']*M*T[j][i] + params['epsilon_e']*T[j][i])
            efficiency[j][i] = (F)/(E[j][i]*1e9)

            Tcomp[j][i] = params['gamma_t']*F/p
            Tcomm[j][i] = params['beta_t']*W 
            Estatic[j][i] = p*(params['epsilon_e']*T[j][i]+params['delta_e']*M*T[j][i])
            Edyn[j][i] = p*(params['gamma_e']*F/p + params['beta_e']*W)
            j = j + 1
        i = i + 1 


    ret['labels'] = {'title':"Problem size vs. replication (P = %d)"%(p),
                     'Xlabel':"n",
                     'Ylabel':"Data Replications (c)"};
    ret['Xaxis'] = Narray
    ret['Yaxis'] = carray

    ret['data'] = efficiency
    ret['repLimit'] = repLimit

#    # ok, now need to interpolate and hack the constant expressions

    if (Ecrazy != 0):
        energyConstInv = interpolate.splrep(constE[:Ecrazy],Narray[:Ecrazy]) # interpolate inverse function
    else:
        energyConstInv = interpolate.splrep(constE,Narray) # interpolate inverse function
    if (Tcrazy != 0):
        timeConstInv = interpolate.splrep(constT[:Tcrazy],Narray[:Tcrazy])
    else:
        timeConstInv = interpolate.splrep(constT,Narray)
        
    cHat = np.arange(1,maxReplication+1)
    constEUpdated = interpolate.splev(cHat,energyConstInv,der=0) # which values of N result in integer c?
    constTUpdated = interpolate.splev(cHat,timeConstInv,der=0)
    ret['newConstEx'] = constEUpdated
    ret['newConstTx'] = constTUpdated
    ret['newY'] = cHat

#    ret['constE'] = constE
#    ret['constT'] = constT

    return ret

# initialize model
params = calculateParameters()
data = n_vs_c_withFixed_p_HBL(params)
quickHeatPlot(data)
plt.show()

