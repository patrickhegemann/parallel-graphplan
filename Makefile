parallelgp: *.cpp *.h
	g++ -o parallelgp *.cpp

debug: *.cpp *.h
	g++ -g -o parallelgp *.cpp

clean:
	rm parallelgp

