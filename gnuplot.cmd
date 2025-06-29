set datafile commentschars ";"
set term svg
set output "bump.svg"
# plot "bump.dat" with lines
set multiplot layout 2,1
plot "bump.dat" using 1:2 with lines  title 'Left' linecolor rgb 'blue'
plot ""         using 1:3 with lines  title 'Right' linecolor rgb 'red'


