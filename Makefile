all:
	make -C Configurator
	make -C ProtectLayer

clean:
	make -C Configurator clean
	make -C ProtectLayer clean
