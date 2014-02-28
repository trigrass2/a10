
static inline int name_to_gpio(char *name) {
	return ((name[1]-'A')<<8) + name[2]-'0';
}

static inline u32 gpio_readl(int off) {
	return *(u32 *)(base[0] + off);
}

static inline void gpio_writel(int off, u32 val) {
	*(u32 *)(base[0] + off) = val;
}

static inline void gpio_set_value(int gpio, int val)
{
	int off = (gpio>>8)*0x24 + 0x10;
	u32 dat = gpio_readl(off);
	int pin = gpio&0xff;
	if (val)
		dat |= 0x1 << pin;
	else
		dat &= ~(0x1 << pin);
	gpio_writel(off, dat);
}

static inline int gpio_get_value(int gpio)
{
	int pin = gpio&0xff;
	u32 dat = gpio_readl((gpio>>8)*0x24 + 0x10);
	dat >>= pin;
	return dat & 0x1;
}

static inline void gpio_direction_output(int gpio) {
}


