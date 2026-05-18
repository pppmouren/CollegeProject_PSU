TARGET=hw0c1blink1s

SOURCES=../include/import_registers.c \
        ../include/enable_pwm_clock.c \
        ../include/wait_period.c \
        hw0c1blink1s.c

OBJECTS=$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(notdir $(SOURCES))))

all: $(OBJECTS)
	g++ $(OBJECTS) -lpthread -o $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

%.o:%.c
	gcc -c $< -o $@

%.o:../include/%.c
	gcc -c $< -o $@

%.o:%.cpp
	g++ -c $< -o $@

%.o:../include/%.cpp
	g++ -c $< -o $@
