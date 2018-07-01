ifdef DEBUG
MAKE=make DEBUG=$(DEBUG)
else
MAKE=make
endif

CONFIGURATOR_PATH=../Configurator
CONFIGURATOR_LIB=$(CONFIGURATOR_PATH)/libconfigurator.a

all: $(CONFIGURATOR_LIB)
	$(MAKE) -C common
	make DEBUG= -C BS_slave
	make DEBUG= -C demo
	$(MAKE) -C demo_CTP

clean:
	make -C common clean
	make -C BS_slave clean
	make -C demo clean
	make -C demo_CTP clean

$(CONFIGURATOR_LIB):
	$(MAKE) -C $(CONFIGURATOR_PATH)
