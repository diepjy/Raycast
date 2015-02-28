raycast : raycast.cpp cutil_math.h  standalone.h  vector_types.h
	# No optimisation version
	#g++ $< -I. -o $@ 
	# O3 option version
	#g++ -O3 $< -I. -o $@ 
	# O3 and openmp version
	g++ -O3 -fopenmp $< -I. -o $@ 

clean :
	rm raycast

.PHONY :    clean test
