
typedef struct {
	int sclk, sdin, rst, dc;
} oled_t ;

static oled_t oled;

static void oled_wr(u8 val) {
	int i;
	gpio_set_value(oled.sclk, 0);
	for (i = 0; i < 8; i++) {
		gpio_set_value(oled.sdin, val&0x80);
		gpio_set_value(oled.sclk, 1);
		gpio_set_value(oled.sclk, 0);
		val <<= 1;
	}
}

static void oled_dat(u8 val) {
	gpio_set_value(oled.dc, 1);
	oled_wr(val);
}

static void oled_cmd(u8 val) {
	gpio_set_value(oled.dc, 0);
	oled_wr(val);
}

static void oled_pos(u8 x, u8 y) {
	oled_cmd(0xb0+y);
	oled_cmd((( x & 0xf0 ) >> 4) | 0x10);
	oled_cmd(( x & 0x0f ) | 0x01);
}

void oled_init(void) {
	int i;

	oled.sclk = name_to_gpio("PI4");
	gpio_direction_output(oled.sclk);
	oled.sdin = name_to_gpio("PI5");
	gpio_direction_output(oled.sdin);
	oled.rst = name_to_gpio("PI6");
	gpio_direction_output(oled.rst);
	oled.dc = name_to_gpio("PI7");
	gpio_direction_output(oled.dc);

	gpio_set_value(oled.sclk, 1);
	gpio_set_value(oled.rst, 0);
	// sleep
	for (i = 0; i < 100; i++) 
		gpio_set_value(oled.dc, 1);
	gpio_set_value(oled.rst, 1);

	oled_cmd(0xae);//--turn off oled panel
	oled_cmd(0x00);//---set low column address
	oled_cmd(0x10);//---set high column address
	oled_cmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	oled_cmd(0x81);//--set contrast control register
	oled_cmd(0xcf); // Set SEG Output Current Brightness
	oled_cmd(0xa0);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	oled_cmd(0xc0);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	oled_cmd(0xa6);//--set display: a6 normal a7 inverse
	oled_cmd(0xa8);//--set multiplex ratio(1 to 64)
	oled_cmd(0x3f);//--1/64 duty
	oled_cmd(0xd3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	oled_cmd(0x00);//-not offset
	oled_cmd(0xd5);//--set display clock divide ratio/oscillator frequency
	oled_cmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
	oled_cmd(0xd9);//--set pre-charge period
	oled_cmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	oled_cmd(0xda);//--set com pins hardware configuration
	oled_cmd(0x12);
	oled_cmd(0xdb);//--set vcomh
	oled_cmd(0x40);//Set VCOM Deselect Level
	oled_cmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
	oled_cmd(0x02);//
	oled_cmd(0x8d);//--set Charge Pump enable/disable
	oled_cmd(0x14);//--set(0x10) disable
	oled_cmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
	oled_cmd(0xaf);//--turn on oled panel
}

static void oled_write_scr(u8 buf[128*8]) {
	int i, j;
	for (i = 0; i < 8; i++) {
		oled_pos(0, i);
		for (j = 0; j < 128; j++) 
			oled_dat(buf[i*128+j]);
	}
}

