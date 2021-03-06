//-----------------------------------------------------------------------------
/*

Display Control

*/
//-----------------------------------------------------------------------------

#ifndef DISPLAY_H
#define DISPLAY_H

//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"
#include "lcd.h"

//-----------------------------------------------------------------------------

struct display_drv {
	struct spi_drv spi;
	struct lcd_drv lcd;
	struct term_drv term;
};

extern struct display_drv pmsynth_display;

//-----------------------------------------------------------------------------

int display_init(struct display_drv *drv);

//-----------------------------------------------------------------------------

#endif				// DISPLAY_H

//-----------------------------------------------------------------------------
