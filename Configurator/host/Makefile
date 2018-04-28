APP_NAME=config_host
LIB_NAME=libconfigurator.a

ifdef DEBUG
CXX=g++ -g -std=c++11 -pedantic -Wall -Wextra
DEFINES=-DDEBUG -DLINUX_ONLY
else
CXX=g++ -std=c++11 -pedantic -Wall -Wextra -o2
DEFINES=-DLINUX_ONLY
endif

SRC_DIR=.
INC_DIRS=-I../ -I../../ProtectLayer/common/AES/ -I../../ProtectLayer/common/
LIB_DIRS=-L../ProtectLayer/common/AES -L../../ProtectLayer/common/AES
LIBS=-laes

OBJ=configurator.o

all: $(OBJ) $(APP_NAME) $(LIB_NAME)

$(OBJ):
	$(CXX) -c $(DEFINES) $(INC_DIRS) configurator.cpp -o configurator.o

$(APP_NAME):
	$(CXX) $(DEFINES) $(INC_DIRS) $(LIB_DIRS) main.cpp $(OBJ) -o $(APP_NAME) $(LIBS)

$(LIB_NAME):
	ar rcs $(LIB_NAME) $(OBJ)

clean:
	rm -vf *.o
	rm -vf $(APP_NAME)
