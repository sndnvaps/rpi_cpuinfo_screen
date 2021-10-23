cc=gcc
CFLAGS=-I/usr/local/include

target = cpuinfo
deps = PCD8544.h
obj = PCD8544_rpi.o PCD8544.o

LIBS=-lwiringPi

$(target): $(obj)
	$(cc) -o $(target) $(obj) $(CFLAGS) $(LIBS)

%.o: %.c $(deps)
	$(cc) -c $< -o $@ $(CFLAGS)

.PHONY: clean
clean:
	rm -rf $(obj) $(target)
