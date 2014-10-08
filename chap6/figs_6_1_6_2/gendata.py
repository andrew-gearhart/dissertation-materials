M_max = 17179869184.
m = 17179869184.
ge = 3.78024e-10
be = 3.78024e-10
ae = 0
de = 5.7742e-9
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

TValues = [90672,90672/2.,90672*1.5,90672*2]
EValues = [378530105.,378558115.57230556,378586126.14461112,378614136.71691668]
P1Values = [147.5,141]
PTValues = [4425]

Mopt = ((be+bt*ee+(ae+at*ee)/m)/(de*gt*f))**.5

def T(p,M):
    return gt*f*n*n/p+bt*n*n/(p*M)+at*n*n/(min(m,M)*M*p)
def E(p,M):
    return (f*(ge+gt*ee)+de*(bt+at/min(m,M)))*n*n+((be+bt*ee)+(ae+at*ee)/min(m,M))*n*n/M+de*gt*f*M*n*n
def MT(p,T):
    return (bt+at/m)/(T*p/n/n-gt*f)
def ME(p,E):
    A = f*(ge+gt*ee)+de*bt
    B = be+bt*ee
    C = de*gt*f
    L = E/n/n-A
    try:
        S = ((A-E/n/n)**2-4*B*C)**.5
    except:
        return "-","-"
    return (L+S)/(2*C),(L-S)/(2*C)
def MP1(p,P1):
    A = de*gt*f
    B = de*bt+gt*ee*f+ge*f-P1*gt*f
    C = be+bt*ee-bt*P1
    try:
        S = (B*B-4*A*C)**.5
    except:
        return "-","-"
    return (-B+S)/(2*A),(-B-S)/(2*A)
def MPT(p,PT):
    return MP1(p,PT/p)

def restrict(p,M):
    #if type(M) == type('a'):
    #    return M
    #if p*M < n or p*M*M > n*n:
    #    return "-"
    return M

p = pmin
output = open("data.dat", "w")
output.write("#p LB UB M0 T(x"+str(len(TValues))+") E_1 E_2(x"+str(len(EValues))+") P1_1 P1_2(x"+str(len(P1Values))+") PT_1 PT_2(x"+str(len(PTValues))+")\n")
l = []
while p <= pmax:
    output.write(str(p) + " " + str(n/p) + " " + str((n*n/p)**.5)+ " " + str(restrict(p,Mopt)) + " ")
    for t in TValues:
        M = restrict(p,MT(p,t))
        output.write(str(M)+" ")
    for e in EValues:
        M = restrict(p,ME(p,e)[0])
        output.write(str(M)+" ")
        M = restrict(p,ME(p,e)[1])
        output.write(str(M)+" ")
    for P in P1Values:
        M = restrict(p,MP1(p,P)[0])
        output.write(str(M)+" ")
        M = restrict(p,MP1(p,P)[1])
        output.write(str(M)+" ")
    for P in PTValues:
        M = restrict(p,MPT(p,P)[0])
        output.write(str(M)+" ")
        if type(M)==type(1.) and p*M >= n and p*M*M <= n*n:
            l.append((T(p,M),p,M,))
        M = restrict(p,MPT(p,P)[1])
        output.write(str(M)+" ")
        if type(M)==type(1.) and p*M >= n and p*M*M <= n*n:
            l.append((T(p,M),p,M,))
    output.write("\n")
    p += pstep
print min(l),max(l)
