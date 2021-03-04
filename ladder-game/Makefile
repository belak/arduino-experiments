build:
	arduino-cli compile --fqbn arduino:avr:uno

upload: build
	arduino-cli upload --fqbn arduino:avr:uno -p /dev/cu.usbmodem101

.PHONY: build upload
