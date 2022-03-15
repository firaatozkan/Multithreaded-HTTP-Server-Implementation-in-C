SRC = src
INC = inc
BUILD = build
OBJ = build/obj
BIN = build/bin

target = $(BIN)/demo
sources = $(wildcard $(SRC)/*.c)
objects = $(subst $(SRC), $(OBJ), $(sources:.c=.o))

CC = gcc
CFLAGS = -g -Wall -I$(INC)
LIBS = $()

# Do not optimize

all: $(target)

$(target): $(objects)
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ)/%.o: $(SRC)/%.c $(INC)/%.h
	@mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)



# Optimize for size
.PHONY: release
release: CFLAGS += -O3 -DMULTITHREADED
release: LIBS += -lpthread
release: $(target)



# Optimize for size
.PHONY: size
size: CFLAGS += -Os
size: $(target)


# Optimize for debug
.PHONY: debug
debug: CFLAGS += -DDEBUG -Og
debug: $(target)

# Clean
.PHONY: clean
clean:
	$(RM) -rf $(BIN)/* $(OBJ)/*