OBJ_RUN:= run.o
SRC_RUN:= run.cpp
OBJ_DIS:= disk.o
SRC_DIS:= disk.cpp
CXX:= g++
CXXFLAGS:= -Wall -g
VGFLAGS:= --leak-check=yes
GDBFLAGS:= --args

all: run mk_disk

run: $(OBJ_RUN)
	$(CXX) $(CXXFLAGS) $(OBJ_RUN) -o run
mk_disk: $(OBJ_DIS)
	$(CXX) $(CXXFLAGS) $(OBJ_DIS) -o mk_disk
run.o: $(SRC_RUN)
	$(CXX) $(CXXFLAGS) -c $(SRC_RUN)
disk.o: $(SRC_DIS)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIS)
clean:
	rm $(OBJ_RUN) $(OBJ_DIS) run mk_disk file.*;
valgrind_run:
	valgrind $(VGFLAGS) ./run;
valgrind_disk:
	valgrind $(VGFLAGS) ./mk_disk file.valgrind;
gdb_run:
	gdb $(GDBFLAGS) run
gdb_disk:
	gdb $(GDBFLAGS) mk_disk file.gdb

test_disk:
	make;
	./mk_disk file.disk;

push:
	git push https://github.com/goplemms/Forky-Fork.git master;
pull:
	git pull https://github.com/goplemms/Forky-Fork.git master;
