M_max = 16117.*4
m = 16117.*4
ge = 3.78024e-10
be = 3.78024e-10
ae = 0
de = 5.7742e-9
ee = 0
gt = 2.5202e-12
bt = 1.56e-10
at = 6.00e-8
f = 100.

M0 = ((be+bt*ee+(ae+at*ee)/m)/(de*gt*f))**.5

n = 3e5

min(x,y) = (x < y) ? x : y
oor(x,y) = (((x*y) > n) ? 0 : 1.e50) + (((x*y*y) < n*n) ? 0 : 1.e50)
# x will represent p, y will represent M
T(x,y) = gt*f*n*n/x+bt*n*n/(x*y)+at*n*n/(min(m,y)*y*x)
E(x,y) = (f*(ge+gt*ee)+de*(bt+at/min(m,y)))*n*n+((be+bt*ee)+(ae+at*ee)/min(m,y))*n*n/y+de*gt*f*y*n*n