SUBDIRS = \
Agnus CIA CPU Denise Drive Files FileSystems Foundation LogicBoard Memory \
Paula Peripherals xdms

MYCC = g++ -std=c++17 -Wfatal-errors

MYFLAGS = \
-Wall \
-I $(CURDIR) \
-I $(CURDIR)/Agnus \
-I $(CURDIR)/Agnus/Blitter \
-I $(CURDIR)/Agnus/Copper \
-I $(CURDIR)/CIA \
-I $(CURDIR)/CPU \
-I $(CURDIR)/CPU/Moira \
-I $(CURDIR)/Denise \
-I $(CURDIR)/Drive \
-I $(CURDIR)/Files \
-I $(CURDIR)/Files/DiskFiles \
-I $(CURDIR)/Files/RomFiles \
-I $(CURDIR)/FileSystems \
-I $(CURDIR)/Foundation \
-I $(CURDIR)/LogicBoard \
-I $(CURDIR)/Memory \
-I $(CURDIR)/Paula \
-I $(CURDIR)/Paula/Audio \
-I $(CURDIR)/Paula/DiskController \
-I $(CURDIR)/Paula/UART \
-I $(CURDIR)/Peripherals \
-I $(CURDIR)/xdms

export MYCC MYFLAGS

DEPS =
OBJ = Amiga.o

.PHONY: all prebuild subdirs clean bin

all: prebuild $(OBJ) subdirs
	@echo > /dev/null
	
prebuild:

subdirs:
	@for dir in $(SUBDIRS); do \
		echo "Entering ${CURDIR}/$$dir"; \
		$(MAKE) -C $$dir; \
	done

clean:
	@echo "Cleaning up $(CURDIR)"
	@rm -f vAmiga *.o
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

bin:
	@echo "Linking vAmiga"
	@g++ -pthread -o vAmiga *.o */*.o */*/*.o

%.o: %.cpp $(DEPS)
	@echo "Compiling $<"
	@$(MYCC) $(MYFLAGS) -c -o $@ $<
