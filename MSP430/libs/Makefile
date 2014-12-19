SOURCES  = libutils.c libuart.c libfifo.c libsoftuart.c liblcd.c liblcdi2c.c libi2c.c libclock.c libsim900.c
INCLUDES = -I ../include
MCU      = msp430g2553

#######################################################################################
CFLAGS   = -mmcu=$(MCU) -g -Os -Wall -Wunused -std=c99 $(INCLUDES)
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
