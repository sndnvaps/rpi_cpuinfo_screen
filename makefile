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

.PHONY: install
install:
	install -D cpuinfo /usr/local/rpi_cpuinfo_screen/cpuinfo
	chmod 0755 /usr/local/rpi_cpuinfo_screen/cpuinfo
	install rpi_cpuinfo_screen_service.sh /etc/init.d/cpuinfo
	chmod 0755 /etc/init.d/cpuinfo
	update-rc.d  cpuinfo defaults

.PHONY: uninstall
uninstall:
	rm -rf /usr/local/rpi_cpuinfo_screen
	update-rc.d -f cpuinfo remove
	rm -rf /etc/init.d/cpuinfo

.PHONY: clean
clean:
	rm -rf $(obj) $(target)
