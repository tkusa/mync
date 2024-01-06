CC = g++
CFLAGS = -Wall -g

SERVER_PROG = mync
SERVER_OBJECTS = netcat.o socket.o log.o
OBJ = ./build/obj
BIN = ./build/bin
SRC = ./src
BINPROGS = $(addprefix $(BIN)/, $(SERVER_PROG))

all: prepare $(BINPROGS)

clean:
	rm -f $(OBJ)/* $(BIN)/*

prepare:
	mkdir -p $(OBJ);
	mkdir -p $(BIN);
	
# compile obj
$(OBJ)/%.o : $(SRC)/%.cpp
	$(COMPILE.cpp) $< -o $@

# link server program
$(BIN)/$(SERVER_PROG): $(addprefix $(OBJ)/, $(SERVER_OBJECTS))
	$(LINK.o) $^ $(LDLIBS) -o $@

