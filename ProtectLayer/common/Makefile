LIBNAME=libcommon.a


SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

ifdef DEBUG
CXX=g++ -g -std=c++11
DEFINES=-DDEBUG=1 -DLINUX_ONLY=1
else
CXX=g++ -std=c++11
DEFINES=-DLINUX_ONLY
endif

INCDIRS=-I. -I.. -IAES/ -I../../Configurator/host
LIBDIRS=-LAES/ -L../../Configurator/host

LIBAES=AES/libaes.a

all: $(LIBAES) $(LIBNAME)

$(LIBAES):
	make -C AES

%.o: %.cpp
	$(CXX) -c $(INCDIRS) $(LIBDIRS) $(DEFINES) $< -o $@

$(LIBNAME): $(OBJECTS)
	#$(CXX) -c $(INCDIRS) $(LIBDIRS) $(DEFINES) uTESLAMaster.cpp -o uTESLAMaster.o
	ar rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -vf *.o
	rm -vf $(LIBNAME)
