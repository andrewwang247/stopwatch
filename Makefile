# Compiler and flags.
CXX := g++ -std=c++17
FLAGS := -Wall -Werror -Wextra -Wconversion -pedantic -Wfloat-equal -Wduplicated-branches -Wduplicated-cond -Wshadow
OPT := -O3 -DNDEBUG
DEBUG := -g3 -DDEBUG

# Executable name and linked files without extensions.
EXE := test
LINK := framework

# Build optimized executable.
release : $(EXE).cpp $(LINK).cpp
	$(CXX) $(FLAGS) $(OPT) -c $(EXE).cpp $(LINK).cpp
	$(CXX) $(FLAGS) $(OPT) -o $(EXE) $(EXE).o $(LINK).o

# Build with debug features.
debug : $(EXE).cpp $(LINK).cpp
	$(CXX) $(FLAGS) $(DEBUG) -c $(EXE).cpp $(LINK).cpp
	$(CXX) $(FLAGS) $(DEBUG) -o $(EXE) $(EXE).o $(LINK).o

# Remove executable binary and generated objected files.
.PHONY : clean
clean : 
	rm -f $(EXE) $(EXE).o $(LINK).o
