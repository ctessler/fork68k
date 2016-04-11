INC := . lib/
CFLAGS := -I. -Ilib -ggdb
INC_PARAM = $(foreach d, $(INC), -I$d)

all: out/ctsim

%.o: $<
	$(CXX) $(CFLAGS) -c -std=gnu++0x $< -o $@

obj/68k.o:	68k/68k.cpp
obj/68kexception.o:	68k/exception.cpp
obj/results.o:	results.cpp
obj/step68k.o:	step68k.cpp step68k.h
obj/ctsim.o:	ctsim.cpp
obj/pipeline.o: 68k/pipeline.cpp

deps := ctsim step68k 68k 68kexception results pipeline
objs := $(patsubst %,obj/%.o,$(deps))

out/ctsim: $(objs)
	echo $(objs)
	$(CXX) -o out/ctsim $(objs)
clean:
	rm -f $(objs) out/ctsim
