# source files
# no need to explain a wildcard in name and sign
files = $(wildcard *.cpp)
# object files instead of listing them.
# this way we have our make file always working,
# so long as files are in the same folder
# then substitute the .cpp part of $(files) with .o
objects = $(patsubst %.cpp,%.o,$(files))

#g++ commands
# g++
CPPFLAG = g++
# C++ version to compile to
CVFLAG = -std=c++11
# error handling flags
CEFLAG = -Wall -Werror
BIN = matrixmult_smpi
# rules
all: $(BIN)

# create executable from object files
$(BIN): $(objects)
	$(CPPFLAG) $(CEFLAG) -g -o $(BIN) $(objects) -pthread -lrt

# create object files from source files
%.o: %.cpp
	$(CPPFLAG) $(CVFLAG) $(CEFLAG) -g -c -O3 $< -o $@

# clean 
.PHONY:clean

run:$(BIN)
	./${BIN}

clean:
	rm -f $(BIN) *.o