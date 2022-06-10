TARGET := c523

BUILD_DIR := build

CC := gcc
CFLAGS := -lm -lasound -lpthread

SRCS := notes.c lpcm.c noncanon.c stdin_monitor.c tcp_monitor.c alsa_player.c
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

$(TARGET) : $(OBJS)
	$(CC) main.c -o $@ $(OBJS) $(CFLAGS)

$(BUILD_DIR) :
	mkdir $@

$(OBJS): $(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: run clean

run : $(TARGET)
	./$(TARGET)

clean :
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f mywave.pcm
