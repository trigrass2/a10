
package gpio

import (
	"syscall"
	"log"
)

var fdmem int
var pwmMem []byte
var gpioMem []byte

func InitMmap() {
	pwmMem = Mmap(0x01c20800, 0x400)
	gpioMem = Mmap(0x01c20e00, 0xc)
}

func Mmap(addr int64, size int) (data []byte) {
	log.Printf("devmem: do mmap %x %x", addr, size)

	var err error
	if fdmem == 0 {
		fdmem, err = syscall.Open("/dev/mem", syscall.O_RDWR|syscall.O_SYNC, 0777)
		if err != nil {
			log.Fatalln("devmem: open failed:", err)
		}
	}

	data, err = syscall.Mmap(fdmem, addr, size,
		syscall.PROT_READ|syscall.PROT_WRITE,
		syscall.MAP_SHARED,
	)
	if err != nil {
		log.Fatalln("devmem: mmap failed:", err)
	}

	log.Printf("devmem: done mmap %x %x successfully", addr, size)

	return
}

