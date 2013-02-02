#include "bsp.h"
#include "rdoorbell0.h"
#include "morse.h"

#include <avr/wdt.h>


Q_DEFINE_THIS_FILE;


#define SB(port,bit) port |= (1 << bit)
#define CB(port,bit) port &= ~ (1 << bit)


void BSP_onStartup(void)
{
	/* Must match BSP_TICKS_PER_SECOND */
	wdt_enable(WDTO_60MS);
	WDTCR |= (1 << WDIE);
}

void QF_onIdle(void)
{
	PRR = 0b00001111;       /* Everything off. */
	SB(MCUCR, SM1);         /* Power down sleep mode. */
	CB(MCUCR, SM0);
	SB(MCUCR, SE);          /* Enable sleep mode. */

	/* Don't separate the following two assembly instructions.  See Atmel's
	   NOTE03. */
	__asm__ __volatile__ ("sei" "\n\t" :: );
	__asm__ __volatile__ ("sleep" "\n\t" :: );

	/* Now we're awake again. */
	CB(MCUCR, SE);          /* Disable sleep mode. */
}

void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line)
{
	morse_assert(file, line);
}


void BSP_watchdog(struct RDoorbell0 *me)
{
	wdt_reset();
	WDTCR |= (1 << WDIE);
}


/**
 * Check to see if the button is down.  If it appears to be down twice in a row
 * send a button signal.
 */
void BSP_button(struct RDoorbell0 *me)
{
	static uint8_t bstate = 0;

	if (0 == (PINB & (1<<2))) {
		switch (bstate) {
		case 0:
			bstate ++;
			break;
		case 1:
			post((QActive*)me, BUTTON_PRESS_SIGNAL);
			bstate ++;
			break;
		default:
			post((QActive*)me, BUTTON_PRESS_SIGNAL);
			break;
		}
	} else {
		if (bstate > 1) {
			post((QActive*)me, BUTTON_RELEASE_SIGNAL);
		}
		bstate = 0;
	}
}


void BSP_init(void)
{
	wdt_disable();
	wdt_reset();
	cli();
	DDRB =  (1 << 4) |	/* Power relay. */
		(1 << 3) |	/* Internal bell. */
		(0 << 2) |	/* Button. */
		(1 << 1) |	/* Piezo. */
		(1 << 0);	/* LED. */
	PORTB = (1 << 4) |	/* Power relay on. */
		(0 << 3) |	/* Internal bell off. */
		(1 << 2) |	/* Button, pull up on. */
		(0 << 1) |	/* Piezo off. */
		(0 << 0);	/* LED off. */
	sei();
}


void BSP_LED(uint8_t onoff)
{
	if (onoff) {
		CB(PORTB, 0);
	} else {
		SB(PORTB, 0);
	}
}


void BSP_bell(uint8_t onoff)
{
	if (onoff) {
		SB(PORTB, 3);
	} else {
		CB(PORTB, 3);
	}
}


void BSP_power(uint8_t onoff)
{
	if (onoff) {
		SB(PORTB, 4);
	} else {
		CB(PORTB, 4);
	}
}


void BSP_buzzer(uint8_t onoff)
{
	if (onoff) {
		SB(PORTB, 1);
	} else {
		CB(PORTB, 1);
	}
}


SIGNAL(WDT_vect)
{
	postISR((QActive*)(&rdoorbell0), WATCHDOG_SIGNAL);
	QF_tick();
}


void BSP_stop_everything(void)
{
	cli();
	wdt_reset();
	wdt_disable();
	PORTB = (1 << 4) |
		(0 << 3) |
		(0 << 2) |
		(0 << 1) |
		(0 << 0);
}


void BSP_enable_morse_line(void)
{
	SB(DDRB, 0);
	CB(PORTB, 0);
}


void BSP_morse_signal(uint8_t onoff)
{
	BSP_LED(onoff);
}
