
obj-m = mod.o

CURRENT_DIR:=$(shell pwd)  
KERNEL_PATH:=/root/sun4i-src/linux-sunxi-sunxi-v3.4.79-r0
  
all:  
	    make -C $(KERNEL_PATH) M=$(CURRENT_DIR) modules  
clean:  
	    make -C $(KERNEL_PATH) M=$(CURRENT_DIR) clean  
