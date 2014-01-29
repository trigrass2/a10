
package gpio

import (
	"syscall"
	"log"
	"unsafe"
)

type param struct {
	slot int
	off int
	data uintptr
	size int
}

func io(slot int, w uintptr, off int, b unsafe.Pointer, size int) {
	p := &param{
		slot: slot,
		off: off,
		data: uintptr(b),
		size: size,
	}
	syscall.Syscall(syscall.SYS_IOCTL, uintptr(fd),
		w, uintptr(unsafe.Pointer(p)))
}

func Writeb(slot, off int, b byte) {
	io(slot, 1, off, unsafe.Pointer(&b), 1)
}

func Readb(slot, off int) (b byte) {
	io(slot, 0, off, unsafe.Pointer(&b), 1)
	return
}

func Writel(slot, off int, b uint32) {
	io(slot, 1, off, unsafe.Pointer(&b), 4)
}

func Readl(slot, off int) (b uint32) {
	io(slot, 0, off, unsafe.Pointer(&b), 4)
	return
}

func GetPinMode(port, pin int) byte {
	pin_idx := pin>>3
	off := port*0x24 + pin_idx<<2;

	v := Readl(0, off)
	bit_off := (pin - (pin_idx<<3))<<2
	v = (v>>uint32(bit_off))&7
	return byte(v)
}

func SetPinMode(port, pin int, mode byte) {
	pin_idx := pin>>3
	off := port*0x24 + pin_idx<<2;

	v := Readl(0, off)
	bit_off := (pin - (pin_idx<<3))<<2

	v &= ^(7<<uint32(bit_off))
	v |= uint32(mode)<<uint32(bit_off)
	Writel(0, off, v)
}

// 0=disable
// 1=pull up
// 2=pull down
func SetPinPull(port, pin int, mode byte) {
	off := port*0x24 + 0x1c + pin/16*4
	v := Readl(0, off)
	v &= ^(3<<uint32((pin%16)*2))
	v |= uint32(mode)<<uint32((pin%16)*2)
	Writel(0, off, v)
}

func SetPinData(port, pin int, v bool) {
	off := port*0x24 + 0x10
	val := Readl(0, off)
	val &= ^(1<<uint32(pin))
	if v {
		val |= 1<<uint32(pin)
	}
	Writel(0, off, val)
}

func GetPinData(port, pin int) bool {
	off := port*0x24 + 0x10
	val := Readl(0, off)
	return byte(val>>uint32(pin))&1 != 0
}

// mode = 00, 01, 10, 11 level
func SetPinDrv(port, pin int, mode byte) {
	off := port*0x24 + 0x14 + pin/16*4
	v := Readl(0, off)
	v &= ^(3<<uint32((pin%16)*2))
	v |= uint32(mode)<<uint32((pin%16)*2)
	Writel(0, off, v)
}

/* Pins that can be used as interrupt source  */
/* PH0 - PH21, PI10 - PI19 (all in mux6 mode) */
/* PH18 = EINT18 */
/* PH13 = EINT13 */
func SetIntMode(eint int, mode byte) {
	off := 0x200 + (eint/8)*4
	v := Readl(0, off)
	eint %= 8
	v &= ^(7<<uint32(eint*4))
	v |= uint32(mode)<<uint32(eint*4)
	Writel(0, off, v)
}

/*
mode:
0x0: Positive Edge
0x1: Negative Edge
0x2: High Level
0x3: Low Level
0x4: Double Edge (Positive/ Negative)
*/
func SetIntEnable(eint int, mode byte) {
	off := 0x210
	v := Readl(0, off)
	v &= ^(1<<uint32(eint))
	v |= uint32(mode)<<uint32(eint)
	Writel(0, off, v)
}

var fd int

type Pin struct {
	port, pin int
}

const (
	In = 0
	Out = 1
)

func Open(port, pin, dir int) Pin {
	if dir == In {
		SetPinMode(port, pin, 0)
	} else {
		SetPinMode(port, pin, 1)
	}
	return Pin{port, pin}
}

func (p Pin) Read() bool {
	return GetPinData(p.port, p.pin)
}

func (p Pin) Write(v bool) {
	SetPinData(p.port, p.pin, v)
}

func (p Pin) H() {
	p.Write(true)
}

func (p Pin) L() {
	p.Write(false)
}

func PollEInt() (r uint32) {
	syscall.Syscall(syscall.SYS_READ,
		uintptr(fd), uintptr(unsafe.Pointer(&r)), 4)
	return
}

func Init() {
	var err error
	fd, err = syscall.Open("/dev/gpio", syscall.O_RDWR, 0744)
	if err != nil {
		log.Fatal(err)
	}
	log.Println("gpio init done")
}

