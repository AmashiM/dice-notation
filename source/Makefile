
CXX=g++

OBJ_DIR=../obj/

TARGETS = $(foreach file,$(wildcard ./*.h),$(patsubst ./%.h, %, $(file)))

all: link build

print-%: ; @echo $* = $($*)

link: $(foreach targ,$(TARGETS),link-$(targ))

link-%:
	$(CXX) -c ./$*.cpp -o $(OBJ_DIR)$*.o

build:
	$(CXX) $(foreach targ,$(TARGETS),$(OBJ_DIR)$(targ).o) main.cpp -o main