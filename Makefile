#CC           = avr-gcc
#CFLAGS       = -Wall -mmcu=atmega16 -Os -Wl,-Map,test.map
#OBJCOPY      = avr-objcopy
CC           = gcc
LD           = gcc
AR           = ar
ARFLAGS      = rcs
CFLAGS       = -Wall -Os -c
LDFLAGS      = -Wall -Os -Wl,-Map,test.map
OMPFLAGS     = -fopenmp
ifdef AES192
CFLAGS += -DAES192=1
endif
ifdef AES256
CFLAGS += -DAES256=1
endif

OBJCOPYFLAGS = -j .text -O ihex
OBJCOPY      = objcopy

# include path to AVR library
INCLUDE_PATH = /usr/lib/avr/include
# splint static check
SPLINT       = splint test.c aes.c -I$(INCLUDE_PATH) +charindex -unrecog

default: test.elf

.SILENT:
.PHONY:  lint clean benchmark

test.hex : test.elf
	echo copy object-code to new image and format in hex
	$(OBJCOPY) ${OBJCOPYFLAGS} $< $@

test.o : test.c aes.h aes.o
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o  $@ $<

aes.o : aes.c aes.h
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o $@ $<

test.elf : aes.o test.o
	echo [LD] $@
	$(LD) $(LDFLAGS) -o $@ $^

aes.a : aes.o
	echo [AR] $@
	$(AR) $(ARFLAGS) $@ $^

lib : aes.a

clean:
	rm -f *.OBJ *.LST *.o *.gch *.out *.hex *.map *.elf *.a benchmark.elf

test:
	make clean && make && ./test.elf
	make clean && make AES192=1 && ./test.elf
	make clean && make AES256=1 && ./test.elf

lint:
	$(call SPLINT)

# OpenMP benchmark targets
aes_openmp.o : aes_openmp.c aes_openmp.h aes.h
	echo [CC] $@ $(CFLAGS) $(OMPFLAGS)
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $<

benchmark.o : benchmark.c aes.h aes_openmp.h
	echo [CC] $@ $(CFLAGS) $(OMPFLAGS)
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $<

benchmark.elf : aes.o aes_openmp.o benchmark.o
	echo [LD] $@ with OpenMP
	$(LD) $(LDFLAGS) $(OMPFLAGS) -o $@ $^ -lrt

benchmark: benchmark.elf
	echo "Running benchmark..."
	./benchmark.elf
