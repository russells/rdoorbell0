/**
 * @file
 *
 */

#include "rdoorbell0.h"
#include "external-bell.h"
#include "bsp.h"


/** The only active RDoorbell0. */
struct RDoorbell0 rdoorbell0;


Q_DEFINE_THIS_FILE;

static QState rdoorbell0Initial        (struct RDoorbell0 *me);
static QState rdoorbell0State          (struct RDoorbell0 *me);
static QState waitingState             (struct RDoorbell0 *me);
static QState ringState                (struct RDoorbell0 *me);
static QState politePauseState         (struct RDoorbell0 *me);


static QEvent rdoorbell0Queue[4];
static QEvent externalbellQueue[4];

QActiveCB const Q_ROM Q_ROM_VAR QF_active[] = {
	{ (QActive *)0              , (QEvent *)0      , 0                        },
	{ (QActive *)(&externalbell), externalbellQueue, Q_DIM(externalbellQueue) },
	{ (QActive *)(&rdoorbell0)  , rdoorbell0Queue  , Q_DIM(rdoorbell0Queue)   },
};
/* If QF_MAX_ACTIVE is incorrectly defined, the compiler says something like:
   rdoorbell0.c:68: error: size of array ‘Q_assert_compile’ is negative
 */
Q_ASSERT_COMPILE(QF_MAX_ACTIVE == Q_DIM(QF_active) - 1);


int main(int argc, char **argv)
{
 startmain:
	BSP_init(); /* initialize the Board Support Package */
	rdoorbell0_ctor();
	externalbell_ctor(&externalbell);

	QF_run();

	goto startmain;
}


void QF_onStartup(void)
{
	BSP_onStartup();
}


void rdoorbell0_ctor(void)
{
	QActive_ctor((QActive *)(&rdoorbell0), (QStateHandler)&rdoorbell0Initial);
}


static QState rdoorbell0Initial(struct RDoorbell0 *me)
{
	return Q_TRAN(&waitingState);
}


static QState rdoorbell0State(struct RDoorbell0 *me)
{
	switch (Q_SIG(me)) {
	case WATCHDOG_SIGNAL:
		BSP_watchdog(me);
		BSP_button(me);
		return Q_HANDLED();
	}
	return Q_SUPER(&QHsm_top);
}


static QState waitingState(struct RDoorbell0 *me)
{
	switch (Q_SIG(me)) {
	case Q_ENTRY_SIG:
		BSP_LED(1);
		return Q_HANDLED();
	case BUTTON_PRESS_SIGNAL:
		return Q_TRAN(ringState);
	case Q_EXIT_SIG:
		BSP_LED(0);
		return Q_HANDLED();
	}
	return Q_SUPER(rdoorbell0State);
}


static QState ringState(struct RDoorbell0 *me)
{
	switch (Q_SIG(me)) {
	case Q_ENTRY_SIG:
		QActive_arm((QActive*)me, 2);
		post(&externalbell, EXTERNAL_BELL_SIGNAL);
		BSP_bell(1);
		return Q_HANDLED();
	case BUTTON_PRESS_SIGNAL:
		/* Button signals could be generated while we are in here.
		   That would result in a transition here if handled by the top
		   state, which will mean we exit and re-enter, and so call
		   BSP_bell() over and over.  So ignore button signals here to
		   prevent that. */
		return Q_HANDLED();
	case Q_TIMEOUT_SIG:
		return Q_TRAN(politePauseState);
	case Q_EXIT_SIG:
		BSP_bell(0);
		return Q_HANDLED();
	}
	return Q_SUPER(rdoorbell0State);
}


static QState politePauseState(struct RDoorbell0 *me)
{
	switch (Q_SIG(me)) {
	case Q_ENTRY_SIG:
		QActive_arm((QActive*)me, POLITE_PAUSE);
		return Q_HANDLED();
	case BUTTON_PRESS_SIGNAL:
		QActive_arm((QActive*)me, POLITE_PAUSE);
		post(&externalbell, EXTERNAL_BUZZER_SIGNAL);
		return Q_HANDLED();
	case Q_TIMEOUT_SIG:
		QActive_disarm((QActive*)me);
		return Q_TRAN(waitingState);
	}
	return Q_SUPER(rdoorbell0State);
}
