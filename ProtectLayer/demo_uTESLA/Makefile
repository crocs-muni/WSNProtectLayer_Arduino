ifdef DEBUG
MAKE=make DEBUG=$(DEBUG)
else
MAKE=make
endif

all:
	$(MAKE) -C BS_host
	make DEBUG= -C node

clean:
	make -C BS_host clean
	make -C node clean
