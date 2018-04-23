ifdef DEBUG
CXX=g++ -g -std=c++11
DEFINES=-DDEBUG -DLINUX_ONLY
else
CXX=g++ -std=c++11
DEFINES=-DLINUX_ONLY
endif

LIBNAME=libaes.a
INCDIRS=-I. -I../uTESLA_AES/
LIBDIRS=

all: $(LIBNAME)

$(LIBNAME):
	$(CXX) -c $(INCDIRS) $(LIBDIRS) $(DEFINES) TI_aes_128.cpp -o TI_aes_128.o
	$(CXX) -c $(INCDIRS) $(LIBDIRS) $(DEFINES) AES.cpp -o AES.o
	$(CXX) -c $(INCDIRS) $(LIBDIRS) $(DEFINES) AES_crypto.cpp -o AES_crypto.o
	ar rcs $(LIBNAME) *.o


clean:
	rm -f *.o
	if [ -f $(LIBNAME) ]; then \
	    rm $(LIBNAME); \
	fi
