APP_NAME=BS_CTPdemo

ifdef DEBUG
#CXX=g++ -g -std=c++11 -pedantic -Wall -Wextra -fno-omit-frame-pointer -static-libstdc++ -static-libasan -fsanitize=address -U_FORTIFY_SOURCE
CXX=g++ -g -std=c++11 -pedantic -Wall -Wextra -U_FORTIFY_SOURCE
DEFINES=-DDEBUG -DLINUX_ONLY
else
CXX=g++ -std=c++11 -pedantic -Wall -Wextra -o2 -fPIE
DEFINES=-DLINUX_ONLY
endif

SRC_DIR=.
INC_DIRS=-I. -I.. -I../../../ -I../../common -I../../common/AES/ -I../../../Configurator/host
LIB_DIRS=-L../../common -L../../common/AES/ -L../../../Configurator/host
# LIBS=-luteslamaster -lblake224 -lutils
LIBS=-lcommon -laes -lconfigurator
#OBJ_DIR=./obj

all:
	$(CXX) $(DEFINES) $(INC_DIRS) $(LIB_DIRS) $(SRC_DIR)/*.cpp -o $(APP_NAME) $(LIBS)

clean:
	rm -vf *.o
	rm -vf $(APP_NAME)
