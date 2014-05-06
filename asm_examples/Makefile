%.elf: %.S
	avr-gcc -mmcu=atmega2560 -o $@ $^

%.bin: %.elf
	avr-objcopy -O binary $^ $@

clean:
	rm -f *.elf *.out *.bin
