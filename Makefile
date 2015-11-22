# Generic GNUMakefile

# Just a snippet to stop executing under other make(1) commands
# that won't understand these lines
ifneq (,)
This makefile requires GNU Make.
endif

STATIC=pwm.a
DYNAMIC=pwm.so

CC_FILES := $(wildcard *.cc)
OBJS := $(patsubst %.cc, %.o, $(CC_FILES))


ifeq ($(STATIC),pwm_x86.a)

CXX=g++ 
CXXFLAGS = -Wall -pedantic -std=c++11
CXXFLAGS +=-I /home/parallels/Documents/linux/boost_1_58_0
CFLAGS = -Wall -pedantic -std=c++11
LDFLAGS = -Bstatic
LDLIBS = /home/parallels/Documents/linux/boost_1_58_0/stage/lib/libboost_regex.a
LDLIBS +=  -Bdynamic -lm -lpthread 

else 

# If for raspberry pi 2
CXX = arm-linux-gnueabihf-g++
CXXFLAGS = -Wall -pedantic -std=c++11 -Winline -pipe -fPIC
CXXFLAGS +=-mcpu=cortex-a7
CXXFLAGS +=-I /home/parallels/Documents/rpi/boost/boost_1_58_0
CXXFLAGS +=-I /home/parallels/Documents/rpi/wiringPi/local/include
CFLAGS = -Wall -pedantic -std=c++11
LDFLAGS = -Bstatic
LDLIBS = /home/parallels/Documents/rpi/boost/boost_1_58_0/stage/lib/libboost_regex.a
LDLIBS += /home/parallels/Documents/rpi/wiringPi/local/lib/libwiringPiDev.a
LDLIBS += /home/parallels/Documents/rpi/wiringPi/local/lib/libwiringPi.a
LDLIBS += -Bdynamic -lm -lpthread

endif

all: $(STATIC)

static:   $(STATIC)

$(STATIC): .depend $(OBJS)
	@echo "[Link (Static)]"
	@ar rcs $(STATIC) $(OBJS)
	@ranlib $(STATIC)
#  @size $(STATIC)



depend: .depend_c

.depend: cmd = gcc -MM -MF depend $(var); cat depend >> .depend;
.depend:
	@echo "Generating dependencies..."
	@$(foreach var, $(C_FILES, CC_FILES), $(cmd))
	@rm -f depend

-include .depend

# These are the pattern matching rules. In addition to the automatic
# variables used here, the variable $* that matches whatever % stands for
# can be useful in special cases.
%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

%: %.c
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f .depend *.o

.PHONY: clean depend
