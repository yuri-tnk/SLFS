# =============================================================================
#
# Make file for TNKernel examples
#
# Processor: LPC1766
#
# Example:   test_slfs
#
# Compiler: gcc version (GNU Tools ARM Embedded. ver. 4.7 2012q4 )
#
# Dependencies support - for C files (*.c, *.h) only.
#
# =============================================================================

# =========  General =============

FORMAT = ihex

# =========  Pathes ==============

TOOLSROOT = c:\"Program Files"\"GNU Tools ARM Embedded"\"4.7 2012q4"
TOOLS     = $(TOOLSROOT)/bin/arm-none-eabi-

PRJROOTPATH   = i:/slfs
PRJPATH       = i:/slfs/examples/LPC176x/compilers/GCC/Cortex_M3
OBJ_DIR       = $(PRJPATH)/OBJ
LSTPATH       = $(PRJPATH)/LST

CPUPATH         = $(PRJROOTPATH)/Cpu/lpc17xx
TSTPATH         = $(PRJROOTPATH)/examples/LPC176x/test_slfs 
FSCOMMONPATH    = $(PRJROOTPATH)/fs_common 
SLFSPATH        = $(PRJROOTPATH)/slfs 
TNKERNELPATH    = $(PRJROOTPATH)/TNKernel
TNKERNELPATHCM3 = $(TNKERNELPATH)/CortexM3

INCLUDES = -I ./ -I $(TNKERNELPATH)

VPATH = $(PRJPATH) $(CPUPATH) $(TSTPATH) $(FSCOMMONPATH) $(SLFSPATH) $(TNKERNELPATH) $(TNKERNELPATHCM3) 

#===========  Files  ==============

#--  Project target file name (without extension).

IMAGE = test_slfs

#--  Linker script

LDSCRIPT = $(PRJPATH)/LPC1766_gcc.ld

#-- List of objects


OBJECTS = \
  $(OBJ_DIR)/tn_LPC17XX_Startup.o\
  $(OBJ_DIR)/startup_hardware_init.o\
  $(OBJ_DIR)/tn_port_cm3_gcc.o\
  $(OBJ_DIR)/tn_port_cm3.o\
  $(OBJ_DIR)/tn.o\
  $(OBJ_DIR)/tn_dqueue.o\
  $(OBJ_DIR)/tn_event.o\
  $(OBJ_DIR)/tn_mem.o\
  $(OBJ_DIR)/tn_mutex.o\
  $(OBJ_DIR)/tn_sem.o\
  $(OBJ_DIR)/tn_tasks.o\
  $(OBJ_DIR)/tn_utils.o\
  $(OBJ_DIR)/lpc17xx_core.o\
  $(OBJ_DIR)/df_flash.o\
  $(OBJ_DIR)/df_create_init.o\
  $(OBJ_DIR)/df_disk_access.o\
  $(OBJ_DIR)/slfs_log.o\
  $(OBJ_DIR)/slfs_api.o\
  $(OBJ_DIR)/slfs_api_ex.o\
  $(OBJ_DIR)/slfs_create_init.o\
  $(OBJ_DIR)/utils.o\
  $(OBJ_DIR)/main.o\
  $(OBJ_DIR)/shell.o\
  $(OBJ_DIR)/slfs_rand.o\
  $(OBJ_DIR)/slfs_test_utilities.o\
  $(OBJ_DIR)/slfs_tst_dump.o\
  $(OBJ_DIR)/tn_sprintf.o\
  $(OBJ_DIR)/tn_user.o
#  $(OBJ_DIR)/unix_str_func.o   
  

DEP_FILE =  $(OBJ_DIR)/$(notdir $(basename $@).d)

# ---------------------------------------------------------------------------
#  Tools, tools options(flags)
# ---------------------------------------------------------------------------

CC      = $(TOOLS)gcc.exe
AS      = $(TOOLS)as.exe
CPP     = $(TOOLS)g++.exe
LD      = $(TOOLS)ld.exe
OBJCOPY = $(TOOLS)objcopy.exe
OBJDUMP = $(TOOLS)objdump.exe
SIZE    = $(TOOLS)size.exe
NM      = $(TOOLS)nm.exe
REMOVE  = rm
COPY    = copy

#-- Assembler flags

ASMOPTS = -mcpu=cortex-m3 -mthumb --gdwarf-2 -mthumb-interwork
ASMLST = -alhms=$(LSTPATH)/$(notdir $(basename $@).lst)

#-- Compiler  flags

CCOPTS  = $(INCLUDES) $(CDEF) -c -mcpu=cortex-m3 -mthumb -gdwarf-2 \
    -Wall -O1 -mapcs-frame -mthumb-interwork -mlittle-endian \
    -Wnested-externs -Wpointer-arith -Wswitch -fno-builtin\
    -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wunused\
    -D TNKERNEL_PORT_CORTEXM3

#-- Linker flags

LDFLAGS = -T$(LDSCRIPT) -mcpu=cortex-m3 -mthumb -mthumb-interwork \
          -Wl,-Map=$(OBJ_DIR)/$(IMAGE).map -nostartfiles

#--- Compiling/Assembling ---

$(OBJ_DIR)/%.o: %.s
	$(AS) $(ASMOPTS) $< $(ASMLST) -o $@
	
$(OBJ_DIR)/%.o:  %.c 
	$(CC) $(CCOPTS) $< -MD -MF $(DEP_FILE) -o $@

include  $(wildcard   $(OBJ_DIR)/$(notdir $(basename *.*).d))

#-- Build & Linking ---

all: $(PRJPATH)/$(IMAGE).elf

$(PRJPATH)/$(IMAGE).elf: $(OBJECTS)

	$(CC) $(OBJECTS) $(LDFLAGS) --output $(OBJ_DIR)/$(notdir $@)
	$(OBJCOPY) -O$(FORMAT)    $(OBJ_DIR)/$(IMAGE).elf  $(OBJ_DIR)/$(IMAGE).hex

#--- Display segment's size

	$(SIZE) -A $(OBJ_DIR)/$(IMAGE).elf

# ===========================================================================

.PHONY:   clean

clean:

	$(REMOVE) -f $(LSTPATH)/*.lst 
	$(REMOVE) -f $(OBJ_DIR)/*.*

# ===========================================================================
# ===========================================================================
# ===========================================================================






