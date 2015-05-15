OUT=encode
OUT2=decode
SRC=main.cpp
SRC2=main2.cpp
OBJ=$(SRC:.cpp=.o)
OBJ2=$(SRC2:.cpp=.o)
CXXFLAGS=-O2

all: $(OBJ) $(OBJ2)
	$(CXX) $(OBJ) -std=c++11 -o $(OUT)
	$(CXX) $(OBJ2) -std=c++11 -o $(OUT2)
clean:
	$(RM) $(OUT)
	$(RM) $(OBJ)
	$(RM) $(OUT2)
	$(RM) $(OBJ2)
