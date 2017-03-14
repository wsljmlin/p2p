#makefile:

#################################
CC := gcc
CPP := g++
AR := ar
CFLAGS	= -Wall -g -DLINUX32 -D_FILE_OFFSET_BITS=64 -Werror -DNDEBUG




LIBS    = -lpthread
INCS    = -I../libutil -I../protocol -I../libcomm -I../license -I../libhttpsvr

##test: sources := $(shell echo *.cpp)  or sources := $(shell ls *.cpp)
outputdir := ../../bin/
output := $(outputdir)tracker.e
sources :=  $(wildcard *.cpp *.c)
objects := $(subst .c,.o,$(subst .cpp,.o,$(sources)))
depends := $(subst .c,.d,$(subst .cpp,.d,$(sources)))
archives := ../libcomm/libcomm.a ../protocol/libprotocol.a ../libutil/libutil.a ../license/liblicense.a ../libhttpsvr/libhttpsvr.a

################################
all: $(output)

%.o:%.c
	$(CC) $(CFLAGS) $(INCS) -o $@ -c $<
	
%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCS) -o $@ -c $<

$(output): $(objects) $(archives)
	-mkdir $(outputdir)
	$(CPP) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LIBS)

../libutil/libutil.a:
	(cd ../libutil;make)
	pwd	
	
../protocol/libprotocol.a:
	(cd ../protocol;make)
	pwd
			
../libcomm/libcomm.a:
	(cd ../libcomm/;make)
	pwd	

../license/liblicense.a:
	(cd ../license/;make)
	pwd	

../libhttpsvr/libhttpsvr.a:
	(cd ../libhttpsvr/;make)
	pwd	

#################################
#depend:
-include $(depends)
%.d: %.c
	$(CC) -MM $(CFLAGS) $< > $@.$$$$;                   \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;   \
	rm -f $@.$$$$
	
%.d: %.cpp
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$;                   \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;   \
	rm -f $@.$$$$
	
#################################
.PHONY : print clean
print:
	echo $(sources)
	echo $(objects)
	echo $(depends)
	echo $(CFLAGS)
	echo $(CPPFLAGS)
	(cd ../libutil; make print)
	(cd ../protocol; make print)
	(cd ../libcomm; make print)
	(cd ../license; make print)
	(cd ../libhttpsvr; make print)
	
clean:
	-rm *.d
	-rm *.o
	-rm $(output)
	(cd ../libutil; make clean)
	(cd ../protocol; make clean)
	(cd ../libcomm; make clean)
	(cd ../license; make clean)
	(cd ../libhttpsvr; make clean)
##################################

