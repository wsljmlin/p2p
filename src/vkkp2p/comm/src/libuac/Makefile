#makefile:

#################################
CC := gcc
CPP := g++
AR := ar
CFLAGS	= -Wall -g -DLINUX32 -D_FILE_OFFSET_BITS=64 -Werror -DNDEBUG




LIBS    = -lpthread
INCS    = 

##test: sources := $(shell echo *.cpp)  or sources := $(shell ls *.cpp)
output := libuac.a
sources :=  $(wildcard *.cpp *.c)
objects := $(subst .c,.o,$(subst .cpp,.o,$(sources)))
depends := $(subst .c,.d,$(subst .cpp,.d,$(sources)))
archives :=

################################
all: $(output)

%.o:%.c
	$(CC) $(CFLAGS) $(INCS) -o $@ -c $<
	
%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCS) -o $@ -c $<

$(output): $(objects) $(archives)
	$(AR) rv $@ $^
	
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
	
clean:
	-rm *.d
	-rm *.o
	-rm $(output)
##################################

