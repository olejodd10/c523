TARGET := a440

BUILD_DIR := build

CC := gcc
CFLAGS := -lm -lasound 

SRCS := notes.c lpcm.c
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

# .DEFAULT_GOAL := run Hvis jeg vil sette defaulten til noe annet enn øverste oppskrift

$(TARGET) : $(OBJS)
	$(CC) main.c -o $@ $(OBJS) $(CFLAGS)

$(BUILD_DIR) :
	mkdir $@

$(OBJS): $(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

# Phony gjør at ting ikke assosieres med filer. Så .PHONY: clean gjør at make clean fungerer fint selv hvis jeg har en fil som heter "clean"
.PHONY: run clean play

run : $(TARGET)
	./$(TARGET)

clean :
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f mywave.pcm

mywave.pcm : run

play : mywave.pcm
	ffplay -showmode 1 -f u8 -ar 44100 mywave.pcm
