cc = gcc
target = cpuinfo
deps = PCD8544.h
obj = PCD8544_rpi.o PCD8544.o

$(target): $(obj)
     $(cc) -o $(target) $(obj) -L/usr/local/lib -lwiringPi

%.o: %.c $(deps)
   $(cc) -c $< -o $@

.PHONY: clean
clean:
    rm -rf $(obj) $(target)