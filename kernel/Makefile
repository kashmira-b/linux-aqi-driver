obj-m += aqi_sensor.o

# When building inside the Lima VM, this variable is exported.
# If compiling directly against the host kernel (assuming Linux), we use uname.
KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
