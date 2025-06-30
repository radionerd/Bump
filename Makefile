bump.wav: bumpwrite
	./bumpwrite
	
bump.svg: bump.dat gnuplot.cmd
	gnuplot "gnuplot.cmd"
	
bump.dat: bump.wav
	sox bump.wav bump.dat
	
bumpwrite: bumpwrite.c
	gcc -Wall -O2 -o bumpwrite bumpwrite.c -lm

.PHONY: clean

clean:
	rm -f bumpwrite bump.wav bump.dat bump.svg
	
firefox: bump.svg
	firefox file:///home/richard/Projects/Bump/bump.svg
	
