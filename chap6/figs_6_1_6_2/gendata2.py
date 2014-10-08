# generate data for CA nbody
M_max = 17179869184. # 16Gb (or 16 GW?)
m = 17179869184. # 16Gb
ge = 3.78024e-10 # 378 pJ/flop
be = 3.78024e-10 # 378 pJ/word?
ae = 0
de = 5.7742e-9 # 5.77 Watts/Gb???
ee = 0
gt = 2.5202e-12
bt = 1.56e-10
#at = 6.00e-8
at = 0
f = 100.

# magnify comm. costs so this is interesting:
bt *= 1.e6
be *= 1.e6

n = 1e8
pmin = 1
pmax = 100
pstep = .05
Mmax = 4.e7
Mstep = 8.e4
Mmin = 0 + Mstep

# contant time values for generating contours
TValues = [11334*3]
for i in xrange(20):
    TValues.append(TValues[0]*(i+2))
TValues.append(TValues[0]*1.5)

# n-body optimal memory size
Mopt = ((be+bt*ee+(ae+at*ee)/m)/(de*gt*f))**.5

def T(p,M):
    return gt*f*n*n/p+bt*n*n/(p*M)+at*n*n/(min(m,M)*M*p)
def E(p,M):
    return (f*(ge+gt*ee)+de*(bt+at/min(m,M)))*n*n+((be+bt*ee)+(ae+at*ee)/min(m,M))*n*n/M+de*gt*f*M*n*n
# what the value of M is for a given number of processors and runtime...
def MT(p,T):
    return (bt+at/m)/(T*p/n/n-gt*f)

def restrict(p,M):
    #if type(M) == type('a'):
    #    return M
    #if p*M < n or p*M*M > n*n:
    #    return "-"
    return M

p = pmin
output = open("data2.dat", "w")
output2 = open("heatplot.dat", "w")
output.write("#p LB UB M0 T(x"+str(len(TValues))+")\n")
while p <= pmax:
    output.write(str(p) + " " + str(n/p) + " " + str((n*n/p)**.5)+ " " + str(restrict(p,Mopt)) + " ")
    for t in TValues:
        M = restrict(p,MT(p,t))
        output.write(str(M)+" ")
    output.write("\n")
    p += pstep

M = Mmin

pstep = .2
while M <= Mmax:
    p = pmin
    while p < pmax:
        #output2.write( str(p) + " " + str(M) + " " + str(E(p,M)) + "\n")
        if p*M > n and p*M*M < n*n:
            # are these magic scaling numbers on E the smallest and largest energies we attain?
            output2.write( str(p) + " " + str(M) + " " + str((E(p,M)-378502095.697)/(381682988.13-378502095.697)) + "\n")
        else:
            output2.write( str(p) + " " + str(M) + " 0\n")
        p += pstep
    output2.write('\n')
    M += Mstep

