set datafile commentschars ";"
set term svg
set output "bump.svg"
# plot "bump.dat" with lines
set multiplot layout 2,1
set xlabel "Time in Seconds"
set ylabel "Signal to Amplifier"
plot "bump.dat" using 1:2 with lines  title 'Left Channel' linecolor rgb 'blue'
plot ""         using 1:3 with lines  title 'Right Channel' linecolor rgb 'red'


