fcc: fcc.c

test: fcc
	./test.sh

clean:
	rm -f fcc *.o *~ tmp*
