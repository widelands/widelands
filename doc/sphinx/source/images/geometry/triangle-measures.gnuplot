#  Figure to illustrate the measures of a triangle where every side has the
#  lenght 1.

set terminal png transparent crop size 400,200*sqrt(3)

set xrange [0 :         1]
set yrange [0 : sqrt(3)/2]

set arrow from  0,0 to  1,        0 nohead lt  2 lw 4
set arrow from  0,0 to .5,sqrt(3)/2 nohead lt  2 lw 4
set arrow from  1,0 to .5,sqrt(3)/2 nohead lt  2 lw 4
set arrow from .5,0 to .5,sqrt(3)/2 nohead lt -1 lw 1

set label "1"           at  .5,          -.02 center
set label "sqrt(3) / 2" at .52,1/(2*sqrt(3)) left

set samples 1000
set size ratio -1
unset xtics
unset ytics
set border 0

#  Must plot something to produce output.
set parametric
plot [0 to 0] t, t notitle
