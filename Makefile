OUT=encode
SRC=main.cpp
OBJ=$(SRC:.cpp=.o)
CXXFLAGS=-O2

all: $(OBJ)
	$(CXX) $(OBJ) -o $(OUT)
clean:
	$(RM) $(OUT)
	$(RM) $(OBJ)
