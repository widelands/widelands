#  Figure to illustrate how to calculate the angle of view given the length of
#  a piece of ground, sqrt(3) / 2 and the length of the projection of it,
#  1 / 2.

set terminal png transparent crop size 800,600

set xrange [0          : sqrt(3)/2]
set yrange [-sqrt(3)/4 : sqrt(3)/4]

set label "ground: sqrt(3) / 2" at 0.5,0.02
set arrow from 0,0 to sqrt(3)/2,0 nohead lt 2 lw 4

set label "ground\nprojection:\n1 / 2" at 0.05,-0.26
set arrow from 0,0 to 1/sqrt(12),-1/sqrt(6) nohead lt 2 lw 4

set label "line of sight" at 0.6,0.2
set arrow from sqrt(3)/2,1/sqrt(6) to 1/sqrt(108),-1/sqrt(54) lt -1 lw 1.2

set label "ground normal" at 1/sqrt(12),0.3 center
set arrow from 1/sqrt(12),-1/sqrt(6) to 1/sqrt(12),1/sqrt(6) nohead lt -1 lw 1.2

set arrow from 1/sqrt(12),-1/sqrt(6) to sqrt(3)/2,0 nohead lt -1 lw 1.2
set parametric
set samples 1000
set size ratio -1
unset xtics
unset ytics
set border 0
set multiplot
angle1_radius_inner=0.02
angle1_radius_outer=0.03
plot [0 to acos(1/sqrt(3))]                                                                  \
             angle1_radius_inner*cos(t),           -angle1_radius_inner*sin(t) lt 5 notitle, \
             angle1_radius_outer*cos(t),           -angle1_radius_outer*sin(t) lt 5 notitle, \
  1/sqrt(12)+angle1_radius_inner*sin(t),            angle1_radius_inner*cos(t) lt 5 notitle, \
  1/sqrt(12)+angle1_radius_outer*sin(t),            angle1_radius_outer*cos(t) lt 5 notitle, \
  1/sqrt(12)-angle1_radius_inner*sin(t),           -angle1_radius_inner*cos(t) lt 5 notitle, \
  1/sqrt(12)-angle1_radius_outer*sin(t),           -angle1_radius_outer*cos(t) lt 5 notitle, \
  1/sqrt(12)+angle1_radius_inner*sin(t), -1/sqrt(6)+angle1_radius_inner*cos(t) lt 5 notitle, \
  1/sqrt(12)+angle1_radius_outer*sin(t), -1/sqrt(6)+angle1_radius_outer*cos(t) lt 5 notitle
angle2_radius=.05
plot [0 to asin(1/sqrt(3))]                                                                  \
  1/sqrt(12)+angle2_radius      *cos(t),            angle2_radius      *sin(t) lt 6 notitle, \
  1/sqrt(12)-angle2_radius      *cos(t),           -angle2_radius      *sin(t) lt 6 notitle, \
  1/sqrt(12)-angle2_radius      *sin(t), -1/sqrt(6)+angle2_radius      *cos(t) lt 6 notitle, \
  sqrt(3)/2 -angle2_radius      *cos(t),           -angle2_radius      *sin(t) lt 6 notitle
