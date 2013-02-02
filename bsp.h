/*****************************************************************************
* Product: DPP example
* Last Updated for Version: 4.0.00
* Date of the Last Update:  Apr 07, 2008
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information:
* Quantum Leaps Web site:  http://www.quantum-leaps.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#ifndef bsp_h_INCLUDED
#define bsp_h_INCLUDED

#include "rdoorbell0.h"

#ifdef __AVR
/* Must match the ticks per second generated by the AVR code. */
#define BSP_TICKS_PER_SECOND 15
#endif

#define BSP_logmsg(f,...)
#define BSP_print_event(me,name,e)

void BSP_watchdog(struct RDoorbell0 *me);
void BSP_onStartup();		/* Called from QF_onStartup(), just before the
				   event loop starts. */
void BSP_init(void);

void BSP_button(struct RDoorbell0 *me); /* Check to see if the button is pressed. */
void BSP_LED(uint8_t onoff);
void BSP_bell(uint8_t onoff);
void BSP_power(uint8_t onoff);
void BSP_buzzer(uint8_t onoff);

void BSP_stop_everything(void);
void BSP_enable_morse_line(void);
void BSP_morse_signal(uint8_t onoff);

#ifdef __AVR
#include "cpu-speed.h"
#include <util/delay.h>
#define BSP_delay_ms(ms) _delay_ms(ms)
#endif


#endif	/* bsp_h_INCLUDED */
