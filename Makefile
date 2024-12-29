TOOLCHAIN_PATH=E:\Programm\EMBEDED\AVR\avr-gcc-12.1.0-x64-windows\bin

CC=$(TOOLCHAIN_PATH)\avr-g++.exe
SIZE=$(TOOLCHAIN_PATH)\avr-size.exe
DUDE=$(TOOLCHAIN_PATH)\avrdude.exe
OBJ_COPY=$(TOOLCHAIN_PATH)\avr-objcopy.exe
OBJ_DUMP=$(TOOLCHAIN_PATH)\avr-objdump.exe
AR=$(TOOLCHAIN_PATH)\avr-ar.exe

SOURCE=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,%.o,$(SOURCE))
LIB=$(SOURCE:.cpp=.h)

OUT=main.out
OUT_HEX=main.hex

MMCU=attiny13a

CFLAGS=-Os -std=c++23 -Wall --param=min-pagesize=0 -DF_CPU=1200000 -I include/
LDFLAGS=-I include/

DUDE_CONFIG=$(TOOLCHAIN_PATH)\avrdude.conf
DUDE_PART_ID=t13
DUDE_PORT=COM3
DUDE_PROGRAMMER=stk500v1



all: build size

put: build to_raw flush

build: $(OBJ)
	$(CC) -mmcu=$(MMCU) $(CFLAGS) -o $(OUT) $(OBJ)

%.o: %.cpp
	$(CC) -mmcu=$(MMCU) $(CFLAGS) -o $@ -c $<


size:
	$(SIZE) -B --total --common $(OBJ)
	$(SIZE) -B --total --common $(OUT)

to_raw:
	$(OBJ_COPY) -O ihex $(OUT) $(OUT_HEX)

to_assembly:
	$(CC) -mmcu=$(MMCU) $(CFLAGS) -S -fverbose-asm $(SOURCE)

dump:
	$(OBJ_DUMP) -d -S $(OUT)
	
flush:
	$(DUDE) -C$(DUDE_CONFIG) -p$(DUDE_PART_ID) -P$(DUDE_PORT) -c$(DUDE_PROGRAMMER) -Uflash:w:$(OUT_HEX) -b19200 -v

clean:
	@del *.o *.s *.out *.hex *.a

archive:
	$(AR) -r "lib.a" $(OBJ) 
