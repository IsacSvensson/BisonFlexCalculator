BIN := Bin\\
BUILD := Build\\
SOURCE := Source\\
INCLUDES := Includes\\
EXECUTABLE := $(BIN)calculator.exe


all: $(EXECUTABLE)

$(EXECUTABLE):	$(SOURCE)calc.l $(SOURCE)calc.y $(SOURCE)calcfunc.cpp 
	bison -d $(SOURCE)calc.y -o $(BUILD)calc.tab.c -d
	flex $(SOURCE)calc.l
	g++ -g -I$(BUILD) -I$(INCLUDES) -Wno-write-strings -o $@ $(BUILD)calc.tab.c lex.yy.c $(SOURCE)calcfunc.cpp

clean:
	del $(BUILD)calc.tab.c
	del $(BUILD)calc.tab.h
	del $(EXECUTABLE)
	del lex.yy.c