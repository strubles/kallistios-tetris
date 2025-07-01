TARGET = out/program.elf

# base directories
SRCDIR = src
INCDIR = include
OUTDIR = out

CSOURCES = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OUTDIR)/%.o, $(CSOURCES)) romdisk.o

# If you define this, the Makefile.rules will create a romdisk.o for you
# from the named dir.
KOS_ROMDISK_DIR = romdisk

KOS_CFLAGS += -DDEBUG -I$(INCDIR)

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

# Ensure the output directory exists before building the final ELF
$(TARGET): $(OBJS)
	@mkdir -p $(OUTDIR)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) \
		$(OBJS) $(OBJEXTRA) -lparallax -lmp3 -lm $(KOS_LIBS)

# Compile .c files into out/*.o
$(OUTDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OUTDIR)
	$(KOS_CC) $(KOS_CFLAGS) -c $< -o $@

clean:
	-rm -f $(TARGET) $(OBJS) romdisk.*

rm-elf:
	-rm -f $(TARGET) romdisk.*
