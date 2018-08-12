//-----------------------------------------------------------------------------
/*

Display Control

*/
//-----------------------------------------------------------------------------

#include "display.h"
#include "io.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct display_drv ggm_display;

//-----------------------------------------------------------------------------
// SPI Setup

#if defined(SPI_DRIVER_HW)
static struct spi_cfg lcd_spi_cfg = {
	.base = SPI2_BASE,
	.mode = SPI_MODE_MASTER,
	.cpol = SPI_CPOL_LO,	// clock is normally low
	.cpha = SPI_CPHA_CLK1,	// data valid on 1st clock edge
	.lsb = SPI_MSB_FIRST,	// ms bit first
	.div = SPI_BAUD_DIV2,	// spi_clock = 168 MHz / 4 * divider
};
#elif defined(SPI_DRIVER_BITBANG)
static struct spi_cfg lcd_spi_cfg = {
	.clk = IO_LCD_SCK,
	.mosi = IO_LCD_SDI,
	.miso = IO_LCD_SDO,
	.cpol = 0,		// clock is normally low
	.cpha = 1,		// latch MISO on falling clock edge
	.lsb = 0,		// ms bit first
	.delay = 0,
};
#else
#error "what kind of SPI driver are we building?"
#endif

//-----------------------------------------------------------------------------

static struct lcd_cfg lcd_cfg = {
	.rst = IO_LCD_RESET,	// gpio for reset pin
	.dc = IO_LCD_DATA_CMD,	// gpio for d/c line
	.cs = IO_LCD_CS,	// gpio for chip select
	.led = IO_LCD_LED,	// gpio for led backlight control
	.bg = LCD_COLOR_NAVY,
	.rotation = 3,
};

//-----------------------------------------------------------------------------

static struct term_cfg term_cfg = {
	.font = 0,
	.lines = 11,
	.yofs = 0,
	.bg = LCD_COLOR_BLACK,
	.fg = LCD_COLOR_WHITE,
};

//-----------------------------------------------------------------------------

static void term_test(struct term_drv *drv) {
	term_print(drv, "_______________________________\n",1);
	term_print(drv, "       1D Waveguide\n",2);
	term_print(drv, "_______________________________\n",3);
	term_print(drv, "       Struck String\n",4);

}


//-----------------------------------------------------------------------------

int display_init(struct display_drv *display) {
	int rc = 0;
	// setup the lcd spi bus
	rc = spi_init(&display->spi, &lcd_spi_cfg);
	if (rc != 0) {
		DBG("spi_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the lcd
	lcd_cfg.spi = &display->spi;
	rc = lcd_init(&display->lcd, &lcd_cfg);
	if (rc != 0) {
		DBG("lcd_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the stdio terminal
	term_cfg.lcd = &display->lcd;
	rc = term_init(&display->term, &term_cfg);
	if (rc != 0) {
		DBG("term_init failed %d\r\n", rc);
		goto exit;
	}

	term_test(&display->term);
	// todo: make this a function
	static const uint32_t data_print[] = {
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFDFF, 
		0xFFFF7FFF, 0xFFFFFFFF, 0xFFFFFFAF, 0xFFFFEBFF, 
		0xFFFFFFFF, 0xFFFFFFF9, 0x7FFFFD3F, 0xFFFFFFFF, 
		0xFFFFFFFF, 0x8BFFFFA3, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xF85FFFF4, 0x3FFFFFFF, 0xFFFFFFFF, 0xFF82FFFE, 
		0x83FFFFFF, 0xFFFFFFFF, 0xFFFA1644, 0xD0BFFFFF, 
		0xFFFFFFFF, 0xFFFFA083, 0x800BFFFF, 0xFFFFFFFF, 
		0xFFFFFA02, 0x80BFFF, 0xFFFFFFFF, 0xFFFFFFD0, 
		0x282817FF, 0xFFFFFFFF, 0xFFFFFFFD, 0x701C17F, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xC0882207, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFC044440, 0x7FFFFFFF, 0xFFFFFFFF, 
		0xFFE03838, 0xFFFFFFF, 0xFFFFFFFF, 0xFFFE0044, 
		0x2FFFFFF, 0xFFFFFFFF, 0xFFFFD304, 0x4197FFFF, 
		0xFFFFFFFF, 0xFFFFFA78, 0xEE3CBFFF, 0xFFFFFFFF, 
		0xFFFFFFA7, 0x8EE3CBFF, 0xFFFFFFFF, 0xFFFFFFF4, 
		0x4C44645F, 0xFFFFFFFF, 0xFFFFFFFF, 0x40F93E05, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xE81F39F0, 0x2FFFFFFF, 
		0xFFFFFFFF, 0xFE87E7CF, 0xC2FFFFFF, 0xFFFFFFFF, 
		0xFFEC037D, 0xC02FFFFF, 0xFFFFFFFF, 0xFFFECE1F, 
		0xF872FFFF, 0xFFFFFFFF, 0xFFFFF7F8, 0x7E1FD3FF, 
		0xFFFFFFFF, 0xFFFFFF7F, 0xFFFFFDCF, 0xFFFFFFFF, 
		0xF9FFFFE8, 0xFFFFFE3F, 0x7FFFFFFF, 0xFE67FFFC, 
		0x101FF00F, 0xFBFFFFFF, 0xFF9F9FFF, 0xB0007C0B, 
		0xFFDFFFFF, 0xFFE7FE7F, 0xF7855541, 0xFFFEFFFF, 
		0xFFF9FF99, 0xFEFC2AA8, 0xFFFFEFFF, 0xFFFE7FE7, 
		0xE7DFF955, 0x1FFFFF7F, 0xFFFF9FF9, 0xFF807F8A, 
		0xA7FFFFFB, 0xFFFFF7FE, 0x7FE6F9FE, 0x55FFFFFF, 
		0xDFFFFFFC, 0x9FF9FFEE, 0xA29C0FFF, 0xFDFFFFFF, 
		0x7FE7C3E, 0xF5C7BF3F, 0xFFEFFFFF, 0xC07F9FC0, 
		0xA62FF03D, 0xFFFEFFFF, 0xF01F27F1, 0x5517F00, 
		0xCFFFEFFF, 0xFF07C1FE, 0x8A60AA0, 0xC2FFFEFF, 
		0xFFF1F01F, 0xD8884054, 0xE60FFFEF, 0xFFFF3C07, 
		0xF8488382, 0x3127FF5, 0x7FFFF301, 0xFC0298FE, 
		0xC179F, 0xA7FFFF80, 0x7F001F3F, 0x99C02470, 
		0xFD7FFFFE, 0x1FE0078F, 0xE78E00A6, 0xFA7FFFF, 
		0xF9F801C7, 0xF9F0703A, 0xC1D47FFF, 0xFFE65033, 
		0xFE7E01EF, 0x40FA87FF, 0xFFFF9C87, 0x7C9FC001, 
		0xC03FC07F, 0xFFFFFE6B, 0xCF07F000, 0xF03FE807, 
		0xFFFFFFF8, 0x33C07C00, 0x7C955504, 0x7FFFFFFF, 
		0xE0F01F00, 0xF3E2A80, 0x87FFFFFF, 0xFF9C07C4, 
		0x10FF940, 0x47FFFFF, 0xFFFE01F0, 0x6027FE60, 
		0x8FFFFF, 0xFFFFF87C, 0x5C6FF9F, 0x8000FFFF, 
		0xFFFFFFE7, 0x1BBDFE7, 0xFE000FFF, 0xFFFFFFFF, 
		0x807C33F9, 0xFF9800FF, 0xFFFFFFFF, 0xFE1FE4F2, 
		0x7FE7E01F, 0xFFFFFFFF, 0xFFF9F9FC, 0x1FF9FF81, 
		0xFFFFFFFF, 0xFFFFE67F, 0x1FE7FE6, 0x3FFFFFFF, 
		0xFFFFFF9F, 0xC07C9FF9, 0xF9FFFFFF, 0xFFFFFFFE, 
		0x701F07FE, 0x7FE7FFFF, 0xFFFFFFFF, 0xF807C07F, 
		0x9FF99FFF, 0xFFFFFFFF, 0xFFE1F01F, 0xE7FE7E7F, 
		0xFFFFFFFF, 0xFFFF9C07, 0xF9FF9FF9, 0xFFFFFFFF, 
		0xFFFFFE01, 0xFE7FE7FE, 0x67FFFFFF, 0xFFFFFFF8, 
		0x7F9FC9FF, 0x9F9FFFFF, 0xFFFFFFFF, 0xE7E7F07F, 
		0xE7FE7FFF, 0xFFFFFFFF, 0xFF99FC07, 0xF9FF99FF, 
		0xFFFFFFFF, 0xFFFE7F01, 0xF27FE7E7, 0xFFFFFFFF, 
		0xFFFFF9C0, 0x7C1FF9FF, 0xBFFFFFFF, 0xFFFFFFE0, 
		0x1F01FE7F, 0xE7FFFFFF, 0xFFFFFFFF, 0x87C07C9F, 
		0xF9FFFFFF, 0xFFFFFFFF, 0xFE701F07, 0xFE7FFFFF, 
		0xFFFFFFFF, 0xFFF807C0, 0x7F9FFFFF, 0xFFFFFFFF, 
		0xFFFFE1F0, 0x1FE7FFFF, 0xFFFFFFFF, 0xFFFFFF9C, 
		0x7F9FFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0x1FE7FFF, 
		0xFFFFFFFF, 0xFFFFFFFF, 0xF87F9FFF, 0xFFFFFFFF, 
		0xFFFFFFFF, 0xFFE7E7FF, 0xFFFFFFFF, 0xFFFFFFFF, 
		0xFFFF99FF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFE7F, 
	};
	lcd_draw_bitmap(&display->lcd,100,120,100,100,LCD_COLOR_WHITE,LCD_COLOR_BLACK,data_print);


 exit:
	return rc;
}

//-----------------------------------------------------------------------------
