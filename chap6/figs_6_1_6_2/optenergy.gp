set term postscript enhanced eps color font "Helvetica,24"
set output "optenergy.eps"
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

M0 = 16117447.7804
set arrow 1 from 65,1.05*M0 to 52,.9*M0 front
set label 2 at 54,1.3*M0 "minimum energy\ngiven runtime limit" front
   set arrow 3 from 35,1.5*M0 to 31,M0 front
   set label 4 at 24,1.75*M0 "minimum energy and runtime\ngiven total power limit" front

#unset xtics
#unset ytics
set xtics (6,20,40,60,80,100)
set ytics ("M_{opt}" 16117447.7804)
set xlabel "P"
set ylabel "M"
plot [6:] [0:4.e7]\
'data2.dat' using (rifmore($1,50)):(max(min($26,$3),$2)):3 ls 7 with filledcu fill pattern 9 border ti "runs within a maximum time", \
   'data.dat' using 1:2:(max(min($21,$3),$2)) ls 4 with filledcu fill transparent pattern 3 border ti "runs within a total power budget", '' using 1:(ifmore(ifless($21,$3),$2)) ls 4 with lines ti "",\
      'data2.dat' using (rifmore($1,50)):(ifmore(ifless($26,$3),$2)) with lines ls 7 ti "",\
	 'data.dat' using 1:(ifmore(ifless($4,$3),$2)) with lines ls 2 ti "minimum energy runs",\
	       "data.dat" using 1:2 with lines ls 1 ti "", '' using 1:3 with lines ls 1 ti ""
#      '' using 1:(ifmore(ifless($9,$3),$2)) with lines ls 3 ti "constant energy lines",\
#	 '' using 1:(ifmore(ifless($10,$3),$2)) with lines ls 3 ti "",\
#	 '' using 1:(ifmore(ifless($11,$3),$2)) with lines ls 3 ti "",\
#	 '' using 1:(ifmore(ifless($12,$3),$2)) with lines ls 3 ti "",\
#	 '' using 1:(ifmore(ifless($13,$3),$2)) with lines ls 3 ti "",\
#	 '' using 1:(ifmore(ifless($14,$3),$2)) with lines ls 3 ti "",\
#	 '' using 1:(ifmore(ifless($15,$3),$2)) with lines ls 3 ti "",\
#	 '' using 1:(ifmore(ifless($16,$3),$2)) with lines ls 3 ti "",\
