bump.dat: bump.wav
	sox bump.wav bump.dat
	
bump.wav: bumpwrite
	./bumpwrite
	
bumpwrite: bumpwrite.c
	gcc -Wall -O2 -o bumpwrite bumpwrite.c -lm

wavwrite: wavwrite.c
	gcc -Wall -O2 -o wavwrite wavwrite.c

.PHONY: clean

clean:
	rm -f wavwrite bumpwrite bump.wav bump.dat sawtooth_test.wav 
	
gnuplot: bump.dat
	@echo 'gnuplot> set datafile commentschars ";"'
	@echo 'gnuplot> plot "bump.dat" with lines'
	@echo 'gnuplot> plot "bump.dat" with lines'
	@echo 'gnuplot> set output "bump.svg"'
	@echo 'gnuplot> plot "bump.dat" with lines'
	@echo 'gnuplot> '

firefox: bump.svg
	firefox file:///home/richard/Projects/wavwrite/bump.svg
	
