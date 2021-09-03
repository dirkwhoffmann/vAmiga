SUBDIRS = Blitter Copper

SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)

.PHONY: all clean subdirs

all: $(OBJ) subdirs
	@echo > /dev/null
	
subdirs:
	@for dir in $(SUBDIRS); do \
		echo "Entering ${CURDIR}/$$dir"; \
		$(MAKE) $(MAKEOPT) -C $$dir; \
	done

clean:
	@echo "Cleaning up $(CURDIR)"
	@rm -f *.o
	@for dir in $(SUBDIRS); do \
		$(MAKE) $(MAKEOPT) -C $$dir clean; \
	done

%.o: %.cpp
	@echo "Compiling $<"
	@$(CC) $(CCFLAGS) $(CPPFLAGS) -c -o $@ $< $(CFLAGS)
