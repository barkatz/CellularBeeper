DEFINES  = -DUSE_DRIVERLIB

#######################################################################################
TARGET   = main.bin
SOURCES  = main.c
LINKTO   = -lutils -luart -lfifo -lclock ../driverlib/ucs.a ../driverlib/gpio.a
INCLUDES = -I/opt/ti/gcc/include -I ../include -I ../driverlib
LIBS     = -L/opt/ti/gcc/include -L../libs -L../driverlib $(LINKTO)
MCU      = msp430f5529
#######################################################################################
CWARNFLAGS = -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused
CFLAGS     = -mmcu=$(MCU) -g -Os $(CWARNFLAGS) -std=c99 $(INCLUDES) -fdata-sections -ffunction-sections -DGCC_MSP430 $(DEFINES)
ASFLAGS    = -mmcu=$(MCU) -x assembler-with-cpp -Wa,-gstabs
LDFLAGS    = -mmcu=$(MCU) -Wl,--gc-sections
########################################################################################
CC       = msp430-elf-gcc
LD       = msp430-elf-ld
AR       = msp430-elf-ar
AS       = msp430-elf-gcc
GASP     = msp430-elf-gasp
NM       = msp430-elf-nm
OBJCOPY  = msp430-elf-objcopy
RANLIB   = msp430-elf-ranlib
STRIP    = msp430-elf-strip
SIZE     = msp430-elf-size
READELF  = msp430-elf-readelf
MAKETXT  = srec_cat
CP       = cp -p
RM       = rm -f
MV       = mv
########################################################################################
OBJECTS = $(SOURCES:.c=.o)

all: remakelibs clean $(TARGET) cleanlibs

remakelibs:
	cd ../libs && make clean
	cd ../libs && make DEFINES="$(DEFINES)"

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(DEFINES) $(LIBS) -o $@
	echo
	echo ">>>> Size of Firmware <<<<"
	$(SIZE) $(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-$(RM) $(OBJECTS)
	-$(RM) $(TARGET)

cleanlibs:
	cd ../libs && make clean

prog:
	echo "prog $(TARGET)" | mspdebug tilib

.PHONY: clean cleanlibs prog remakelibs
