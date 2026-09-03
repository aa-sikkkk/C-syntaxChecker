CC = gcc
PKGCONFIG ?= pkg-config

# Optional: point GTK_BUILD at a prebuilt GTK install tree (e.g. C:\gtk-build\gtk\x64\release)
# It must contain include/, lib/, share/, bin/ subdirectories.
GTK_BUILD ?=

# Detect which GtkSourceView is available (prefer v4)
CFLAGS = -Wall -Wextra
LDFLAGS =

ifneq ($(GTK_BUILD),)
  # Prebuilt tree: drive pkg-config from the tree's .pc files
  export PKG_CONFIG_LIBDIR := $(GTK_BUILD)/lib/pkgconfig
  HAVE_GSV4 := $(shell $(PKGCONFIG) --exists gtksourceview-4 2>/dev/null && echo 1 || echo 0)
  HAVE_GSV3 := $(shell $(PKGCONFIG) --exists gtksourceview-3.0 2>/dev/null && echo 1 || echo 0)
  HAVE_GTK := $(shell $(PKGCONFIG) --exists gtk+-3.0 2>/dev/null && echo 1 || echo 0)
  ifeq ($(HAVE_GSV4),1)
    CFLAGS += -DHAVE_GTKSOURCEVIEW4 $(shell $(PKGCONFIG) --cflags gtk+-3.0 gtksourceview-4)
    LDFLAGS = -L$(GTK_BUILD)/lib $(shell $(PKGCONFIG) --libs gtk+-3.0 gtksourceview-4)
  else ifeq ($(HAVE_GSV3),1)
    CFLAGS += -DHAVE_GTKSOURCEVIEW $(shell $(PKGCONFIG) --cflags gtk+-3.0 gtksourceview-3.0)
    LDFLAGS = -L$(GTK_BUILD)/lib $(shell $(PKGCONFIG) --libs gtk+-3.0 gtksourceview-3.0)
  else ifeq ($(HAVE_GTK),1)
    CFLAGS += $(shell $(PKGCONFIG) --cflags gtk+-3.0)
    LDFLAGS = -L$(GTK_BUILD)/lib $(shell $(PKGCONFIG) --libs gtk+-3.0)
  else
    $(error GTK_BUILD=$(GTK_BUILD) has no pkg-config files for GTK3; cannot build GUI)
  endif
else
  # Use pkg-config-detectable system installs
  HAVE_GSV4 := $(shell $(PKGCONFIG) --exists gtksourceview-4 2>/dev/null && echo 1 || echo 0)
  HAVE_GSV3 := $(shell $(PKGCONFIG) --exists gtksourceview-3.0 2>/dev/null && echo 1 || echo 0)
  ifeq ($(HAVE_GSV4),1)
    CFLAGS += -DHAVE_GTKSOURCEVIEW4 $(shell $(PKGCONFIG) --cflags gtk+-3.0 gtksourceview-4)
    LDFLAGS = $(shell $(PKGCONFIG) --libs gtk+-3.0 gtksourceview-4)
  else ifeq ($(HAVE_GSV3),1)
    CFLAGS += -DHAVE_GTKSOURCEVIEW $(shell $(PKGCONFIG) --cflags gtk+-3.0 gtksourceview-3.0)
    LDFLAGS = $(shell $(PKGCONFIG) --libs gtk+-3.0 gtksourceview-3.0)
  else
    CFLAGS += $(shell $(PKGCONFIG) --cflags gtk+-3.0 2>/dev/null)
    LDFLAGS = $(shell $(PKGCONFIG) --libs gtk+-3.0 2>/dev/null)
  endif
endif

# Availability info for user
install-info:
	@echo "GtkSourceView4=$(HAVE_GSV4) GtkSourceView3=$(HAVE_GSV3)"
	@echo "Use  make GTK_BUILD=C:/gtk-build/gtk/x64/release  to target the prebuilt tree."

TARGET = synCheckGUI
SRC = main.c ../lexer.c ../analysis.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET) gui.exe

.PHONY: all clean install-info