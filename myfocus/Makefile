BOARD = arduino:avr:diecimila:cpu=atmega328
#BOARD = arduino:avr:nano:cpu=atmega328old

PORT = "/dev/ttyUSB0"

SKETCH = myfocus.ino
LIBS =

compile : $(SKETCH) $(LIBS)
	arduino --verify --board $(BOARD) $(SKETCH)

upload : $(SKETCH) $(LIBS)
	arduino --upload $(SKETCH) --board $(BOARD) --port $(PORT)

