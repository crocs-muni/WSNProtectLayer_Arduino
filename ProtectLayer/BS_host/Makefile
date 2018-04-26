APP_NAME=WSNPL_BS

ifdef DEBUG
CXX=g++ -g -std=c++11 -pedantic -Wall -Wextra
DEFINES=-DDEBUG -DLINUX_ONLY
else
CXX=g++ -std=c++11 -pedantic -Wall -Wextra -o2
DEFINES=-DLINUX_ONLY
endif

SRC_DIR=.
INC_DIRS=-I. -I../common -I../common/AES/
LIB_DIRS=-L../common -L../common/AES/
# LIBS=-luteslamaster -lblake224 -lutils
LIBS=-lcommon -laes
#OBJ_DIR=./obj

all:
	$(CXX) $(DEFINES) $(INC_DIRS) $(LIB_DIRS) $(SRC_DIR)/*.cpp -o $(APP_NAME) $(LIBS)

clean:
	rm -vf *.o
	rm -vf $(APP_NAME)
