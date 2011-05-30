#  Project Name
PROJECT=v3_3d_printer
#  List of the objects files to be compiled/assembled
OBJECTS=startup_LPC17xx.o core_cm3.o system_LPC17xx.o main_LPC17xx.o 
LSCRIPT=ldscript_rom_gnu.ld
VPATH = $(CURDIR)/lib_sd;$(CURDIR)/system

OPTIMIZATION = 0
DEBUG = -g
#LISTING = -ahls

#  Compiler Options
GCFLAGS = -Wall -fno-common -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) $(DEBUG)
GCFLAGS += -D__RAM_MODE__=0
#GCFLAGS += -Wcast-align -Wcast-qual -Wimplicit -Wpointer-arith -Wswitch
#GCFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -nostartfiles -Wl,-Map=$(PROJECT).map -T$(LSCRIPT)
ASFLAGS = $(LISTING) -mcpu=cortex-m3 --defsym RAM_MODE=0

#  Compiler/Assembler/Linker Paths
GCC = arm-none-eabi-gcc
GPP = arm-none-eabi-g++
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
REMOVE = rm -f
SIZE = arm-none-eabi-size

#########################################################################

all:: $(PROJECT).hex $(PROJECT).bin

$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary -j .text -j .data $(PROJECT).elf $(PROJECT).bin

$(PROJECT).hex: $(PROJECT).elf
	$(OBJCOPY) -R .stack -O ihex $(PROJECT).elf $(PROJECT).hex

$(PROJECT).elf: $(OBJECTS)
	$(GCC) $(LDFLAGS) $(OBJECTS) -o $(PROJECT).elf

stats: $(PROJECT).elf
	$(SIZE) $(PROJECT).elf

clean:
	$(REMOVE) $(OBJECTS)
	$(REMOVE) $(PROJECT).hex
	$(REMOVE) $(PROJECT).elf
	$(REMOVE) $(PROJECT).map
	$(REMOVE) $(PROJECT).bin
	$(REMOVE) $(OBJECTS)

#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

.c.o : 
	$(GCC) $(GCFLAGS) -c $<

.cc.o :
	$(GPP) $(GCFLAGS) -c $<

.s.o :
	$(AS) $(ASFLAGS) -o startup_LPC17xx.o startup_LPC17xx.S
	$(AS) $(ASFLAGS) -o $(PROJECT).o $< > $(PROJECT).lst

#########################################################################
