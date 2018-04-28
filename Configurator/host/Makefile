APP_NAME=config_host

ifdef DEBUG
CXX=g++ -g -std=c++11 -pedantic -Wall -Wextra
DEFINES=-DDEBUG -DLINUX_ONLY
else
CXX=g++ -std=c++11 -pedantic -Wall -Wextra -o2
DEFINES=-DLINUX_ONLY
endif

SRC_DIR=.
INC_DIRS=-I../
LIB_DIRS=
LIBS=

all: $(APP_NAME)

$(APP_NAME):
	$(CXX) $(DEFINES) $(INC_DIRS) $(LIB_DIRS) $(SRC_DIR)/*.cpp -o $(APP_NAME) $(LIBS)

clean:
	rm -vf *.o
	rm -vf $(APP_NAME)
