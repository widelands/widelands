#  Figure to illustrate how to test whether a point P = (Xp, Yp) is above a
#  line L from A = (Xa, Ya) to B = (Xb, Yb) when Xa <= Xp < Xb.

set terminal png transparent crop size 800,400

set xrange [-12 : 75]
set yrange [-40 :  8]

set arrow from 0,0 to  4, 0
set arrow from 0,0 to  0,-4
set label "X" at 5, 0
set label "Y" at 0,-5 center

set arrow from  0, -8 to 64,-40 nohead lt  2 lw 4
set arrow from  0, -8 to 64, -8 nohead lt -1 lw 1
set arrow from 64, -8 to 64,-40 nohead lt -1 lw 1
set arrow from 40,-16 to 64,-40 nohead lt  1 lw 2
set arrow from 40,-16 to 40,-40 nohead lt -1 lw 1
set arrow from 40,-40 to 64,-40 nohead lt -1 lw 1

set label "P = (Xp, Yp)"  at 39,-16 right
set label "pdy = Yb - Yp" at 39,-32 right
set label "pdx = Xb - Xp" at 52,-41 center
set label "A = (Xa, Yb)"  at -1, -8 right
set label "B = (Xb, Yb)"  at 65,-40
set label "L"             at 30,-24 right
set label "ldy = Yb - Ya" at 65,-24
set label "ldx = Xb - Xa" at 32, -7

set samples 1000
set size ratio -1
unset xtics
unset ytics
set border 0

#  Must plot something to produce output.
set parametric
plot [0 to 0] t, t notitle
