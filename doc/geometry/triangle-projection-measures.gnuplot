#  Figure to illustrate the measures of a projection of a triangle where every
#  side has the lenght 1. The view angle is chosen so that the height of the
#  triangle is 1 / 2.

set terminal png transparent crop size 400,200

set xrange [0 :  1]
set yrange [0 : .5]

set arrow from  0,0 to  1, 0 nohead lt  2 lw 4
set arrow from  0,0 to .5,.5 nohead lt  2 lw 4
set arrow from  1,0 to .5,.5 nohead lt  2 lw 4
set arrow from .5,0 to .5,.5 nohead lt -1 lw 1

set label "1"     at  .5,-.02  center
set label "1 / 2" at .52,1/6.0 left

set samples 1000
set size ratio -1
unset xtics
unset ytics
set border 0

#  Must plot something to produce output.
set parametric
plot [0 to 0] t, t notitle
