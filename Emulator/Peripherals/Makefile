SUBDIRS = Drive Joystick Mouse Keyboard

SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)

.PHONY: all clean subdirs

all: subdirs
	@echo > /dev/null

subdirs:
	@for dir in $(SUBDIRS); do \
		echo "Entering ${CURDIR}/$$dir"; \
		$(MAKE) -C $$dir; \
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
