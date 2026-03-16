CC = i686-w64-mingw32-gcc
WINDRES = i686-w64-mingw32-windres

CRTFLAGS = -mcrtdll=msvcrt-os
CFLAGS = -Wall -O2 -DWIN32 -D_WINDOWS -D_MBCS $(CRTFLAGS)
LDFLAGS = -mwindows -static-libgcc -static-libstdc++ $(CRTFLAGS)

SRCDIR = src
RESDIR = res
OBJDIR = obj

SOURCES = $(SRCDIR)/bit.c $(SRCDIR)/console.c $(SRCDIR)/cuimode.c $(SRCDIR)/detail.c \
          $(SRCDIR)/emusmw5.c $(SRCDIR)/exlayer.c $(SRCDIR)/main.c $(SRCDIR)/proll.c \
          $(SRCDIR)/runtime.c $(SRCDIR)/smaf.c $(SRCDIR)/version.c $(SRCDIR)/voice.c

OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
RESOURCE_OBJ = $(OBJDIR)/res.o

TARGET = mmftool.exe

LIBS = -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 \
       -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -lwinmm -lversion -lcomctl32

all: $(TARGET)

$(TARGET): $(OBJECTS) $(RESOURCE_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(RESOURCE_OBJ): $(RESDIR)/res.rc | $(OBJDIR)
	$(WINDRES) $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
