
SLOTS = /sys/devices/bone_capemgr.9/slots
TARGET = ADCLK_GPIO-00A0.dtbo

all: $(TARGET)

ADCLK_GPIO-00A0.dtbo: ADCLK_GPIO.dts
	dtc -O dtb -o $(TARGET) -b 0 -@ ADCLK_GPIO.dts

install: $(TARGET)
	@ echo "Installing"
	@ cp $(TARGET) /lib/firmware/

clean:
	rm -f $(TARGET)
