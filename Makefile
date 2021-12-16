MAKE = make
SRC_PATH = src/
OBJ_PATH = obj/
BIN_PATH = build/
GPP = g++ -std=c++11 -g -rdynamic

MAIN_O_FILES=Graph.o FilterVertices.o FilterQueryHelp.o main.o
DATASET_PROCESS_O_FILES=Graph.o Database_process.o

default: main

dataset_pro: $(DATASET_PROCESS_O_FILES) Makefile
	$(GPP) $(foreach n,$(DATASET_PROCESS_O_FILES),$(OBJ_PATH)$(n)) -o $(BIN_PATH)dataset_pro

main: $(MAIN_O_FILES) Makefile 
	$(GPP) $(foreach n,$(MAIN_O_FILES),$(OBJ_PATH)$(n)) -o $(BIN_PATH)main 

#common rules
%.o: $(SRC_PATH)%.cpp Makefile 
	$(GPP) -c $(SRC_PATH)$*.cpp -o $(OBJ_PATH)$*.o 

