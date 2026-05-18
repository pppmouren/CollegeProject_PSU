TARGET=hw10Lin

SOURCES=../include/import_registers.c \
        ../include/enable_pwm_clock.c \
        ../include/wait_period.c \
        ../include/wait_key.c \
        keypress.c \
        pwmsetup.c \
	draw_bitmap.c\
	scale_image_data.c\
	video_interface.c\
        hw10Lin.c

OBJECTS = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SOURCES)))

CFLAGS = `pkg-config --cflags --libs gtk+-2.0` \
         -Wall \
         -Werror \
         -Wno-deprecated-declarations \
         -g

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

$(TARGET): $(OBJECTS)
	gcc $^ $(CFLAGS) -o $@ -lm -lpthread -lswscale
