
# Path where SRGP headers live - change this according to your setup
SRGPDIR = /cs/src/srgp/src/srgp		

# An ANSI C compiler: gnu's "gcc"
#CC = gcc
CC = /opt/SUNWspro/bin/cc # for solaris

# The UNIX flag must be defined for UNIX implementations
CPPFLAGS = -DUNIX -DX11 -I$(SRGPDIR)

# Very important!  The first flag (struct-return) makes it safe to link
#  gcc objects with cc objects.
#GCCFLAGS = -fpcc-struct-return 
GCCFLAGS = 

# Object files wiil go in this subdirectory
OBJ_DIR	     =  solaris

# User-defined; to pass options in to the compilation, say things
#   like 'make CFLAGS=-g'.  Put flags here to make them permanent.

#CFLAGS 	= 

#####################################################################

TARGET = libsphigs.a

HFILES = 			\
sph_draw.h			\
elementType.h			\
sph_errtypes.h			\
sph_face.h			\
falloc.h			\
fallocdefs.h			\
mat3.h				\
mat3defs.h			\
sphigs.h			\
sphigslocal.h			\
$(SRGPDIR)/srgp_sphigs.h	\
$(SRGPDIR)/macros.h		\
$(SRGPDIR)/srgppublic.h

OBJECTS =       \
MAT3factor.o    \
MAT3geom.o      \
MAT3inv.o       \
MAT3mat.o       \
MAT3vec.o       \
sph_attrib.o    \
sph_bitstr.o    \
sph_canon.o     \
sph_clip.o      \
sph_cull.o      \
sph_draw.o      \
sph_edit.o      \
sph_element.o   \
sph_elemdebug.o \
sph_error.o     \
sph_falloc.o    \
sph_filter.o    \
sph_input.o     \
sph_intense.o   \
sph_modxform.o  \
sph_objdebug.o  \
sph_object.o    \
sph_optimize.o  \
sph_post.o      \
sph_refresh.o   \
sph_state.o     \
sph_traverse.o  \
sph_utility.o   \
sph_view.o      \
sph_zsort.o     \
sph_bsp.o    

#####################################################################


PATH_OBJECTS =  $(OBJECTS:%=$(OBJ_DIR)/%)

$(OBJ_DIR)/%.o: %.c
	$(CC) $(GCCFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $(OBJ_DIR)/$*.o $*.c

$(TARGET):  $(PATH_OBJECTS)
	echo $(PATH_OBJECTS)
	ar rv $(TARGET) $(PATH_OBJECTS)
	ranlib $(TARGET)

remove:
	/bin/rm -f $(PATH_OBJECTS)
	/bin/rm -f *~

clean:  remove $(TARGET)

$(PATH_OBJECTS): $(HFILES)
