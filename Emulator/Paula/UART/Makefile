SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)

.PHONY: all clean

all: $(OBJ)
	@echo > /dev/null
	
clean:
	@echo "Cleaning up $(CURDIR)"
	@rm -f *.o

%.o: %.cpp
	@echo "Compiling $<"
	@$(CC) $(CCFLAGS) $(CPPFLAGS) -c -o $@ $<
