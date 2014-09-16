DEFINES  = 

#######################################################################################
TARGET   = main.bin
SOURCES  = main.c
INCLUDES = -I../include
LIBS     = -L../libs -lutils -llcd -luart -lfifo
#LIBS     = -L../libs -lutils -llcd -luart -lfifo
MCU      = msp430g2553
#######################################################################################
CFLAGS   = -mmcu=$(MCU) -g -Os -Wall -Wunused $(INCLUDES) $(DEFINES)
ASFLAGS  = -mmcu=$(MCU) -x assembler-with-cpp -Wa,-gstabs
LDFLAGS  = -mmcu=$(MCU) -Wl
########################################################################################
CC       = msp430-gcc
LD       = msp430-ld
AR       = msp430-ar
AS       = msp430-gcc
GASP     = msp430-gasp
NM       = msp430-nm
OBJCOPY  = msp430-objcopy
RANLIB   = msp430-ranlib
STRIP    = msp430-strip
SIZE     = msp430-size
READELF  = msp430-readelf
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
	echo "prog $(TARGET)" | mspdebug rf2500

.PHONY: clean cleanlibs prog remakelibs
