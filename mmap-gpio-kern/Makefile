obj-m += mod.o  
CURRENT_DIR:=$(shell pwd)  
KERNEL_PATH:=/root/arm/linux-3.4.43
  
all:  
	    make -C $(KERNEL_PATH) M=$(CURRENT_DIR) modules  
clean:  
	    make -C $(KERNEL_PATH) M=$(CURRENT_DIR) clean  
