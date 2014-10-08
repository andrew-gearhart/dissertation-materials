# 'filledcu' = filled curves
set term postscript enhanced eps color font "Helvetica,24"
set output "opttime.eps"
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

#unset xtics
#unset ytics
set xtics (6,20,40,60,80,100)
set ytics ("M_{opt}" 16117447.7804)
set xlabel "P"
set ylabel "M"

M0 = 16117447.7804
set arrow 1 from 55,1.2*M0 to 75,.75*M0 front
set label 2 at 44,1.45*M0 "minimum runtime\ngiven energy limit" front

plot [6:] [0:4.e7]\
'data.dat' using 1:(max(min($10,$3),$2)):(max((min($9,$3)),$2)) ls 3 with filledcu fill pattern 2 border ti "runs within an energy budget", '' using 1:(ifmore(ifless($10,$3),$2)) ls 3 with lines ti "", '' using 1:(ifmore(ifless($9,$3),$2)) ls 3 with lines ti "",\
      '' using 1:2:(max(min($19,$3),$2)) ls 5 with filledcu fill pattern 3 border ti "runs within a per-processor power budget", '' using 1:(ifmore(ifless($19,$3),$2)) ls 5 with lines ti "",\
	 'data.dat' using 1:(ifmore(ifless($4,$3),$2)) with lines ls 2 ti "minimum energy runs",\
	       "data.dat" using 1:2 with lines ls 1 ti "", '' using 1:3 with lines ls 1 ti ""
		  #   '' using (rifmore($1,28)):(ifmore(ifless($5,$3),$2)) with lines ls 7 ti "constant time lines",\
#      '' using (rifmore($1,58)):(ifmore(ifless($6,$3),$2)) with lines ls 7 ti "",\
#	 '' using (rifmore($1,19)):(ifmore(ifless($7,$3),$2)) with lines ls 7 ti "",\
#	    '' using (rifmore($1,14)):(ifmore(ifless($8,$3),$2)) with lines ls 7 ti "",\
