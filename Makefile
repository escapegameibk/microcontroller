MKDIR_P := mkdir -p
CP := cp
MV := mv
CC := avr-gcc
CCC := avr-g++
RM_RF = rm -rf
OCPY := avr-objcopy
AVRDUDE := avrdude
PORT := /dev/ttyACM0
BOARD := atmega2560
PROGRAMMER := wiring
PROGBOARD := m2560
BAUD_RATE_PROG := 115200
FLASH_CMD := $(AVRDUDE) -b $(BAUD_RATE_PROG) -p $(PROGBOARD) -D  -P $(PORT) -c $(PROGRAMMER) 

# directories
CWD := $(realpath .)
BINDIR := $(CWD)/bin
BUILDDIR := $(CWD)/build
SRCDIR := $(CWD)/src
INCLUDEDIR := $(CWD)/include

# flags
CFLAGS := -mmcu=$(BOARD) -Os -I$(INCLUDEDIR) -Wall -Wextra
LDFLAGS := -mmcu=$(BOARD)

# target files
DIRS_TARGET := $(BINDIR) $(BUILDDIR)
TARGET := $(BINDIR)/mc.hex
TARGET_ELF := $(BINDIR)/mc.elf
SRCFILES := $(wildcard $(SRCDIR)/*.c)
OBJFILES := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCFILES))

# fancy targets
all: directories $(TARGET)

directories: $(DIRS_TARGET)

# less fancy targets

$(DIRS_TARGET):
	$(MKDIR_P) $@

$(TARGET) : $(TARGET_ELF)
	$(OCPY) -O ihex -j .text -j .data $^ $@

$(TARGET_ELF): $(OBJFILES)
	$(CC) $(LDFLAGS) -o $@  $^
 
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<                              

flash: $(TARGET)
	$(FLASH_CMD) -U flash:w:$^:i
	$(FLASH_CMD) -U flash:v:$^:i
clean:                                                                          
	$(RM_RF) $(DIRS_TARGET)     
