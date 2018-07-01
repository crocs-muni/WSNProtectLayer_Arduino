AES_PATH=../ProtectLayer/common/AES
LIBAES=$(AES_PATH)/libaes.a

all: $(LIBAES)
	make -C host
	make -C node

clean:
	make -C host clean
	make -C node clean

$(LIBAES):
	make -C $(AES_PATH)
