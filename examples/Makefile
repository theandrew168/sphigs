# This sample Makefile allows you to make any SPHIGS application
#   whose source is exactly one .c file.
#
# It also can be used to make an application made up of a collection
#   of .c files all having a common prefix.
#
# To use this Makefile, you must type:
#
#        make PROG=xxxx
#                  ----
# where
#       xxxx.c is the name of the file you wish to compile, or
#       ----
#
# where
#       xxxx*.c are the names of the files you wish to compile
#       ----


## System Settings

SRGPDIR 	= /cs/src/srgp
CC 		= gcc
GCCFLAGS 	= -fpcc-struct-return
CFLAGS 		= 

## Default Application

SPHDEMO_OBJS = SPHDEMO_main.o SPHDEMO_struct.o SPHDEMO_view.o SPHDEMO_input.o


## Compile/Link Flags

INCLUDE = -I../src/sphigs -I$(SRGPDIR)/src/srgp
LDFLAGS = -L$(SRGPDIR)/src/srgp/objects -L../src/sphigs
LDLIBS  = -lsphigs -lsrgp -lX11 -lm 


## Targets

.c.o: ../src/sphigs/*.h
	$(CC) -c $(GCCFLAGS) $(CFLAGS) $(INCLUDE) $*.c

$(PROG):
	$(CC) $(GCCFLAGS) $(CFLAGS) $(INCLUDE) $(PROG)*.c \
		$(LDFLAGS) $(LDLIBS) -o $(PROG)

SPHDEMO: $(SPHDEMO_OBJS)
	$(CC) $(SPHDEMO_OBJS) $(LDFLAGS) $(CFLAGS) $(LDLIBS) -o SPHDEMO

exec_fast: $(PROG)*.o
	$(CC) $(GCCFLAGS) $(CFLAGS) $(INCLUDE) $(PROG)*.o \
	    $(LDFLAGS) $(LDLIBS) -o $(PROG)
