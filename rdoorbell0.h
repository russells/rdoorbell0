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
	BUTTON_PRESS_SIGNAL,
	BUTTON_RELEASE_SIGNAL,
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
 * Call this instead of calling QActive_post().
 *
 * It checks that there is room in the event queue of the receiving state
 * machine.  QP-nano does this check itself anyway, but the assertion from
 * QP-nano will always appear at the same line in the same file, so we won't
 * know which state machine's queue is full.  If this check is done in user
 * code instead of library code we can tell them apart.
 */
#define post(o, sig)							\
	do {								\
		QActive *_me = (QActive *)(o);				\
		QActiveCB const Q_ROM *ao = &QF_active[_me->prio];	\
		Q_ASSERT(_me->nUsed < Q_ROM_BYTE(ao->end));		\
		QActive_post(_me, sig);					\
	} while (0)

/**
 * Call this instead of calling QActive_postISR().
 *
 * @see post()
 */
#define postISR(o, sig)							\
	do {								\
		QActive *_me = (QActive *)(o);				\
		QActiveCB const Q_ROM *ao = &QF_active[_me->prio];	\
		Q_ASSERT(_me->nUsed < Q_ROM_BYTE(ao->end));		\
		QActive_postISR(_me, sig);				\
	} while (0)


#endif
