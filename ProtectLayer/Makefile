ifdef DEBUG
MAKE=make DEBUG=$(DEBUG)
else
MAKE=make
endif

all:
	$(MAKE) -C common
	$(MAKE) -C BS_host
	make DEBUG= -C BS_slave
	make DEBUG= -C node

clean:
	make -C common clean
	make -C BS_host clean
	make -C BS_slave clean
	make -C node clean
