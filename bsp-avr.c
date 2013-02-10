#include "bsp.h"
#include "rdoorbell0.h"
#include "morse.h"

#include <avr/wdt.h>


Q_DEFINE_THIS_FILE;


#define SB(port,bit) port |= (1 << bit)
#define CB(port,bit) port &= ~ (1 << bit)


void BSP_startMain(void)
{
	wdt_reset();
	wdt_disable();
}


void BSP_onStartup(void)
{
	/* Must match BSP_TICKS_PER_SECOND */
	wdt_enable(WDTO_60MS);
	WDTCR |= (1 << WDIE);
}

void QF_onIdle(void)
{
	/* TODO: sleep */
	QF_INT_UNLOCK();
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


/**
 * Turn on the buzzer, with the given frequency.
 *
 * We rely on gcc's -Os option to collapse this function into BSP_buzzer(), and
 * not actually use another function call.
 *
 * OCR1C determines the frequency (along with the prescaler value, CS13-CS10.
 *
 * OCR1A determines the volume.
 */
static void buzzer_freq(uint16_t freq, uint8_t volume)
{
	uint8_t ocr1c;
	uint8_t cs1;

	CB(DDRB, 0);		/* Input while we set up. */
	switch (freq) {
	case external_bell_high:
		ocr1c = 0;
		cs1 = 0;
		break;
	case external_bell_low:
		ocr1c = 0;
		cs1 = 0;
		break;
	case external_bell_buzz:
		ocr1c = 0;
		cs1 = 0;
		break;
	default:
		Q_ASSERT(0);
		break;
	}
}


/**
 * Turn on the buzzer, with the given frequency and volume.
 *
 * @param freq the buzzer frequency.  If 0, the buzzer is turned off.
 * @param volume can be 1, 2, 4, or 8.  Lower numbers are higher volume.
 */
void BSP_buzzer(uint16_t freq, uint8_t volume)
{
	if (freq) {
		Q_ASSERT(volume==1 || volume==2 || volume==4 || volume==8);
		buzzer_freq(freq, volume);
		SB(DDRB, 1);	/* Ensure the signal gets out. */
	} else {
		CB(DDRB, 1);	/* Stop the signal getting out. */
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
