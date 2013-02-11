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
	CB(MCUCR, SM0);		/* Idle sleep mode. */
	CB(MCUCR, SM1);
	SB(MCUCR, SE);		/* Sleep enable. */
	/* Don't separate the following two assembly instructions.  See
	   Atmel's NOTE03. */
	__asm__ __volatile__ ("sei" "\n\t" :: );
	__asm__ __volatile__ ("sleep" "\n\t" :: );
	CB(MCUCR, SE);
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
 * OCR1C determines the frequency (along with the prescaler value, CS1[3:0].
 *
 * fOCnx = fClkIO / (2 * N * (1 + OCRnx))  // doc2586.pdf, p75.
 * OCRnx = (fClkIO / (2 * N * fOCnx)) - 1  // N is prescaler divisor
 * fClkIO is 1MHz (default clock source, internal RC clock with CLKDIV8 fuse
 * programmed)
 *
 * For 1000Hz, N=4,   CS1[3:0]=0011, OCR1C=249.
 * For 500Hz,  N=8,   CS1[3:0]=0100, OCR1C=249.
 * For 100Hz,  N=32,  CS1[3:0]=0110, OCR1C=155.
 * For 50Hz,   N=64,  CS1[3:0]=0111, OCR1C=155.
 * For 25Hz,   N=128, CS1[3:0]=1000, OCR1C=155.
 *
 * OCR1A determines the volume.
 */
static void buzzer_freq(enum external_bell_frequency freq, uint8_t volume)
{
	uint8_t ocr1c;
	uint8_t ocr1a = 10;	/* Defeat the compiler! */
	uint8_t cs1;
	uint8_t tccr1;

	CB(DDRB, 1);		/* Input while we set up. */
	/* Set up TCCR1 every time.  Does no harm. */
	TCCR1 = (0 << CTC1) |
		(1 << PWM1A) |	/* PWM mode */
		(1 << COM1A1) |	/* OC1A cleared on compare match, /OC1A NC. */
		(0 << COM1A0);	/* CS[3:0] = 0000 */
	switch (freq) {
	case external_bell_high:
		ocr1c = 249;
		cs1 = 0b0011;
		break;
	case external_bell_low:
		ocr1c = 249;
		cs1 = 0b0100;
		break;
	case external_bell_buzz:
		ocr1c = 155;
		cs1 = 0b0110;
		break;
	default:
		Q_ASSERT(0);
		break;
	}

	switch (freq) {
	case external_bell_high:
	case external_bell_low:
		switch (volume) {
		case 1: ocr1a = 15; break;
		case 2: ocr1a = 31; break;
		case 4: ocr1a = 62; break;
		case 8: ocr1a = 125; break;
		default: Q_ASSERT(0);
		}
		break;
	case external_bell_buzz:
		switch (volume) {
		case 1: ocr1a = 9; break;
		case 2: ocr1a = 19; break;
		case 4: ocr1a = 38; break;
		case 8: ocr1a = 77; break;
		default: Q_ASSERT(0);
		}
		break;
	}

	OCR1C = ocr1c;
	OCR1A = ocr1a;
	tccr1 = TCCR1;
	tccr1 &= 0xf0;
	tccr1 |= cs1;
	TCCR1 = tccr1;
	GTCCR = 0;		/* OC1B off, etc. */
	SB(DDRB, 1);		/* Output again. */
}


/**
 * Turn on the buzzer, with the given frequency and volume.
 *
 * @param freq the buzzer frequency.  The frequency is really just an
 * indicator, selected from enum external_bell_frequency.  If 0, the buzzer is
 * turned off.
 *
 * @param volume can be 1, 2, 4, or 8.  Lower numbers are higher volume.
 */
void BSP_buzzer(enum external_bell_frequency freq, uint8_t volume)
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
