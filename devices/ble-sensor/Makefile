build:
	arduino-cli compile --profile m5stack_stamp_c3

upload:
	arduino-cli upload -v -p $(PORT) --profile m5stack_stamp_c3

monitor:
	arduino-cli monitor -p $(PORT) --profile m5stack_stamp_c3 --config baudrate=115200

list:
	arduino-cli board list

clean:
	arduino-cli cache clean
