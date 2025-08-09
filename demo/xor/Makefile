# build by iverilog
#    apt install iverilog
#    apt install gtkwave

xor_test.bin: xor_encryptor.v xor_encryptor_tb.v
	iverilog -o $@ $^

clean:
	rm -f xor_test.bin *.vcd