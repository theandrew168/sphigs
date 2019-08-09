default :
	@echo MAKING... SPHIGS optimized library 
	(cd src/sphigs;  make clean CFLAGS=-O)
	@echo MAKING... example programs
	(cd examples; make)
	(cd examples; make PROG=robot_anim)
	(cd doc; make)
