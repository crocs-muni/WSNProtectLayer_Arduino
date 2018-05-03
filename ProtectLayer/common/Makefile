LIBNAME=libcommon.a


SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

ifdef DEBUG
#CXX=g++ -g -std=c++11 -fno-omit-frame-pointer -static-libstdc++ -static-libasan -fsanitize=address -U_FORTIFY_SOURCE
CXX=g++ -g -std=c++11 -fno-omit-frame-pointer -U_FORTIFY_SOURCE
DEFINES=-DDEBUG=1 -DLINUX_ONLY=1
else
CXX=g++ -std=c++11 -fPIC
DEFINES=-DLINUX_ONLY
endif

INCDIRS=-I. -I../ -I../../ -IAES/ -I../../Configurator/host -I../BS_host
LIBDIRS=-LAES/ -L../../Configurator/host

LIBAES=AES/libaes.a

all: $(LIBAES) $(LIBNAME)

$(LIBAES):
	make -C AES

%.o: %.cpp
	$(CXX) -c $(INCDIRS) $(LIBDIRS) $(DEFINES) $< -o $@

$(LIBNAME): $(OBJECTS)
	ar rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -vf *.o
	rm -vf $(LIBNAME)
	make -C AES/ clean
