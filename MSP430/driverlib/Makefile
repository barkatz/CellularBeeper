SOURCES  = $(wildcard *.c)
INCLUDES = -I/opt/ti/gcc/include
MCU      = msp430f5529

#######################################################################################
CWARNFLAGS = -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused
CFLAGS     = -mmcu=$(MCU) -g -Os $(CWARNFLAGS) -std=c99 $(INCLUDES) -fdata-sections -ffunction-sections -DGCC_MSP430
ASFLAGS    = -mmcu=$(MCU) -x assembler-with-cpp -Wa,-gstabs
LDFLAGS    = -mmcu=$(MCU) -Wl,--gc-sections -Wl,--print-gc-sections
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
LIBS    = $(SOURCES:.c=.a)

all: $(LIBS)

%.a: %.o
	$(AR) ruv $@ $<
	$(RANLIB) $@

%.o: %.c
	$(CC) $(INCLUDES) $(DEFINES) -c $(CFLAGS) -o $@ $<

.SILENT:
.PHONY:  clean
clean:
	-$(RM) $(OBJECTS)
	-$(RM) $(LIBS)
