ifdef DEBUG
MAKE=make DEBUG=$(DEBUG)
else
MAKE=make
endif


all:
	$(MAKE) -C ProtectLayer/common/
	$(MAKE) -C Configurator
	$(MAKE) -C ProtectLayer

clean:
	$(MAKE) -C Configurator clean
	$(MAKE) -C ProtectLayer clean
