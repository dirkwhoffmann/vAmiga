SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)

MYCC = g++ -std=c++17 -Wfatal-errors
MYFLAGS = \
-Wall \
-I $(CURDIR)/..

.PHONY: all prebuild clean

all: prebuild $(OBJ)
	@echo > /dev/null

prebuild:
	@echo "Entering ${CURDIR}"
	
clean:
	@echo "Cleaning up $(CURDIR)"
	@rm -f *.o

%.o: %.cpp $(DEPS)
	@echo "Compiling $<"
	@$(MYCC) $(MYFLAGS) -c -o $@ $<
