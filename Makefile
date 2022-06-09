TARGET := c523

BUILD_DIR := build

CC := gcc
CFLAGS := -lm -lasound -lpthread

SRCS := notes.c lpcm.c noncanon.c stdin_monitor.c 
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

# .DEFAULT_GOAL := run Hvis jeg vil sette defaulten til noe annet enn øverste oppskrift

$(TARGET) : $(OBJS)
	$(CC) main.c -o $@ $(OBJS) $(CFLAGS)

$(BUILD_DIR) :
	mkdir $@

$(OBJS): $(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

# Phony gjør at ting ikke assosieres med filer. Så .PHONY: clean gjør at make clean fungerer fint selv hvis jeg har en fil som heter "clean"
.PHONY: run clean

run : $(TARGET)
	./$(TARGET)

clean :
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f mywave.pcm
