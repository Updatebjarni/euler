%.hex: %.o
	avr-gcc -o $* -mmcu=atmega328 $<
	avr-objcopy -j .text -j .data -O ihex $* $@
	@echo -e '\ntext\tdata\tbss\tROM\tRAM'
	@avr-size $* | \
	  awk 'NR==2{print $$1"\t"$$2"\t"$$3"\t"$$1+$$2"\t"$$2+$$3}'

%.o: %.c
	avr-gcc -c -mmcu=atmega328p -std=gnu99 -Os -Wall $<

upload: firmware.hex
	avrdude -B 5 -c usbasp -p atmega328p -U flash:w:$< -U lfuse:w:0xE2:m

clean:
	rm -f firmware.hex firmware *.o
