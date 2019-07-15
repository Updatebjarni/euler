FIRMWARES:=primary test-device
HEXFILES:=$(FIRMWARES:%=%.hex)

all: $(HEXFILES) bsselect
	@echo -e '\ntext\tdata\tbss\tROM\tRAM'
	@avr-size primary | \
	  awk 'NR==2{print $$1"\t"$$2"\t"$$3"\t"$$1+$$2"\t"$$2+$$3}'

$(HEXFILES): %.hex: %
	avr-objcopy -j .text -j .data -O ihex $< $@

$(FIRMWARES): %: %.o
	avr-gcc -o $@ -mmcu=atmega328p $<

%.o: %.c
	avr-gcc -c -mmcu=atmega328p -std=gnu99 -Os -Wall $<

bsselect: bsselect.c
	gcc -Wall -o bsselect bsselect.c

upload-test: test-device.hex
#	avrdude -B 5 -c usbasp -p atmega328p -U flash:w:test-device.hex
	avrdude -B 5 -c arduino -p atmega328p -P /dev/ttyACM0 -U flash:w:test-device.hex

upload: primary.hex
	./bsselect 0
	avrdude -B 5 -c usbasp -p atmega328p -U flash:w:primary.hex \
	        -U lfuse:w:0xe2:m

upload-test: test-device.hex
	avrdude -c arduino -p atmega328p -P /dev/ttyACM0 -U flash:w:test-device.hex

clean:
	rm -f $(HEXFILES) $(FIRMWARES) *.o
