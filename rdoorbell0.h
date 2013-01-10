#ifndef rdoorbell0_h_INCLUDED
#define rdoorbell0_h_INCLUDED

#include "qpn_port.h"

/* Testing - stop the app and busy loop. */
#define FOREVER for(;;)

enum RDoorbell0Signals {
	/**
	 * Sent for timing, and so we can confirm that the event loop is
	 * running.
	 */
	WATCHDOG_SIGNAL = Q_USER_SIG,
	/**
	 * Sent when the button is pressed.
	 */
	BUTTON_SIGNAL,
	MAX_PUB_SIG,
	MAX_SIG,
};


/**
 * The event structure does not contain any extra information.
 */
struct RDoorbell0Event {
	QEvent super;
};


/**
 * Create the doorbell.
 */
void rdoorbell0_ctor(void);


/**
 */
struct RDoorbell0 {
	QActive super;
	int presses;
};


extern struct RDoorbell0 rdoorbell0;


/** Normal alarm wait period. */
#define POLITE_PAUSE (15 * BSP_TICKS_PER_SECOND)


/**
 * Call this just before calling QActive_post() or QActive_postISR().
 *
 * It checks that there is room in the event queue of the receiving state
 * machine.  QP-nano does this check itself anyway, but the assertion from
 * QP-nano will always appear at the same line in the same file, so we won't
 * know which state machine's queue is full.  If this check is done in user
 * code instead of library code we can tell them apart.
 */
#define fff(o) Q_ASSERT(((QActive*)(o))->nUsed <= Q_ROM_BYTE(QF_active[((QActive*)(o))->prio].end))

#endif
