#makefile:

#################################
CC := gcc
CPP := g++
AR := ar
CFLAGS	= -Wall -g -DLINUX32 -D_FILE_OFFSET_BITS=64 -Werror -DNDEBUG




LIBS    = -lpthread
INCS    = -I../libutil -I../protocol -I../libcomm -I../peer -I../libhttpsvr -I../libuac

##test: sources := $(shell echo *.cpp)  or sources := $(shell ls *.cpp)
outpath := ../../bin/
output := $(outpath)vodpeer.e
sources :=  $(wildcard *.cpp *.c)
objects := $(subst .c,.o,$(subst .cpp,.o,$(sources)))
depends := $(subst .c,.d,$(subst .cpp,.d,$(sources)))
archives := ../peer/libpeer.a ../libutil/libutil.a ../protocol/libprotocol.a ../libcomm/libcomm.a ../libhttpsvr/libhttpsvr.a ../libuac/libuac.a

################################
all: $(output)

%.o:%.c
	$(CC) $(CFLAGS) $(INCS) -o $@ -c $<
	
%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCS) -o $@ -c $<

$(output): $(objects) $(archives)
	-mkdir $(outpath)
	$(CPP) -o $@ $^ $(LIBS)

../libutil/libutil.a:
	(cd ../libutil;make)
	pwd	
	
../protocol/libprotocol.a:
	(cd ../protocol;make)
	pwd
			
../libcomm/libcomm.a:
	(cd ../libcomm/;make)
	pwd	
		
../peer/libpeer.a:
	(cd ../peer/;make)
	pwd	
	
../libhttpsvr/libhttpsvr.a:
	(cd ../libhttpsvr;make)
	pwd	
	
 ../libuac/libuac.a:
	(cd ../libuac;make)
	pwd	
	
#################################
#depend:
-include $(depends)
%.d: %.c
	$(CC) -MM $(CFLAGS) $< > $@.$$$$;                   \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;   \
	rm -f $@.$$$$
	
%.d: %.cpp
	$(CPP) -MM $(CFLAGS) $(CPPFLAGS) $< > $@.$$$$;                   \
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
	(cd ../peer; make print)
	(cd ../libhttpsvr; make print)
	(cd ../libuac; make print)
	
clean:
	-rm *.d
	-rm *.o
	-rm $(output)
	(cd ../libutil; make clean)
	(cd ../protocol; make clean)
	(cd ../libcomm; make clean)
	(cd ../peer; make clean)
	(cd ../libhttpsvr; make clean)
	(cd ../libuac; make clean)
##################################

