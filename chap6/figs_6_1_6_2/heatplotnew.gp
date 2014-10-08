set term postscript enhanced eps color palfuncparam 2000,.1 font "Helvetica,24"
set output "heatplot2.eps"
set style line 1 lw 3 lc 1
set style line 2 lw 2 lc rgb "#00CC00" lt 1
set style line 3 lw 1 lc 3 lt 1
set style line 4 lw 1 lc 4 lt 1
set style line 5 lw 1 lc 5 lt 1
set style line 6 lw 1 lc 6 lt 1
set style line 7 lw 1 lc 7 lt 1
set style fill pattern border
max(x,y) = (x < y) ? y : x
min(x,y) = (x > y) ? y : x
rifless(x,y) = (x < y) ? x : 1/0
rifmore(x,y) = (x > y) ? x : 1/0
ifless(x,y) = min(x,y)
ifmore(x,y) = max(x,y)
onegtr(x,y) = (x > y) ? 1 : 0
oneless(x,y) = (x < y) ? 1 : 0
set palette rgbformula -6,-6,2
unset cbtics
set cblabel "Energy"
#set cbrange [378502095.697:425287171.923]
M0 = 16117447.7804

set arrow 1 from 50,1.1*M0 to 75,1.35*M0 front
set label 2 at 49,1.2*M0 "decreasing time" front rotate by 14
set label 3 at 20,1.5*M0 "maximum useful memory" front textcolor rgbcolor "red" rotate by -22 font "Helvetica,16"
   set label 4 at 10,0.3*M0 "minimum memory" front textcolor rgbcolor "red" rotate by -14 font "Helvetica,16"

n=1.e8

#unset xtics
#unset ytics
set xtics (6,20,40,60,80,100)
set ytics ("M_{opt}" 16117447.7804)
set xlabel "P"
set ylabel "M" offset 0,0
   set colorbox user origin .83,.4 front size .03,.3
plot [6:] [0:4.e7]\
   'heatplot.dat' using 1:2:($3**.6) ls 7 with image ti "Energy",\
      'data2.dat' using (rifmore($1,75)):(ifmore(ifless($5,$3),$2)) with lines ls 7 ti "constant time contours",\
   '' using (rifmore($1,38)):(ifmore(ifless($6,$3),$2)) with lines ls 7 ti "",\
   '' using (rifmore($1,25)):(ifmore(ifless($7,$3),$2)) with lines ls 7 ti "",\
      '' using (rifmore($1,18.6)):(ifmore(ifless($8,$3),$2)) with lines ls 7 ti "",\
   '' using (rifmore($1,15)):(ifmore(ifless($9,$3),$2)) with lines ls 7 ti "",\
      '' using (rifmore($1,12.4)):(ifmore(ifless($10,$3),$2)) with lines ls 7 ti "",\
	 '' using (rifmore($1,10.62)):(ifmore(ifless($11,$3),$2)) with lines ls 7 ti "",\
	    '' using (rifmore($1,9.3)):(ifmore(ifless($12,$3),$2)) with lines ls 7 ti "",\
	       '' using (rifmore($1,8.3)):(ifmore(ifless($13,$3),$2)) with lines ls 7 ti "",\
		  '' using (rifmore($1,7.4)):(ifmore(ifless($14,$3),$2)) with lines ls 7 ti "",\
		     '' using (rifmore($1,6.79)):(ifmore(ifless($15,$3),$2)) with lines ls 7 ti "",\
			'' using (rifmore($1,6.19)):(ifmore(ifless($16,$3),$2)) with lines ls 7 ti "",\
				         'data.dat' using 1:(ifmore(ifless($4,$3),$2)) with lines ls 2 ti "minimum energy runs",\
 'data2.dat' using 1:2 with lines ls 1 ti '', '' using 1:3 with lines ls 1 ti ''
