all:
	make -C ProtectLayer/common/
	make -C Configurator
	make -C ProtectLayer
#	make -C Configurator

clean:
	make -C Configurator clean
	make -C ProtectLayer clean
