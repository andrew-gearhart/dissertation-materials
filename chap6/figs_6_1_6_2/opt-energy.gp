load 'const.p'

set xlabel "p"
set ylabel "M"

set xrange [1:1000]
set yrange [0:M_max]
set isosamples 5,5
set cntrparam levels discrete 0
set view map
set contour
unset surface
splot y-M0+oor(x,y) ti "Minimum energy runs", y-M0*1.5-oor(x,y) ti "constant energy" ls 2
#y-M0*0.5-oor(x,y) ti "" ls 2,\
#y-M0*2+oor(x,y) ti "" ls 2,\
#y-M0*2.5+oor(x,y) ti "" ls 2,\
#x*y-n, x*y*y-n*n