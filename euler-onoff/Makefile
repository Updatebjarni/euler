firmware.hex: firmware
	avr-objcopy -j .text -j .data -O ihex firmware firmware.hex
	@echo -e '\ntext\tdata\tbss\tROM\tRAM'
	@avr-size firmware | \
	  awk 'NR==2{print $$1"\t"$$2"\t"$$3"\t"$$1+$$2"\t"$$2+$$3}'

firmware: firmware.o
	avr-gcc -o firmware -mmcu=attiny13 firmware.o

firmware.o: firmware.c
	avr-gcc -c -mmcu=attiny13 -std=gnu99 -Os -Wall firmware.c

upload: firmware.hex
	avrdude -B 5 -c usbasp -p attiny13 -U flash:w:firmware.hex

clean:
	rm -f firmware.hex firmware *.o
