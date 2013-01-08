/**
 * @file
 *
 */

#include "rdoorbell0.h"
#include "bsp.h"


/** The only active RDoorbell0. */
struct RDoorbell0 rdoorbell0;


Q_DEFINE_THIS_FILE;

static QState rdoorbell0Initial        (struct RDoorbell0 *me);
static QState rdoorbell0State          (struct RDoorbell0 *me);


static QEvent rdoorbell0Queue[4];

QActiveCB const Q_ROM Q_ROM_VAR QF_active[] = {
	{ (QActive *)0              , (QEvent *)0      , 0                        },
	{ (QActive *)(&rdoorbell0)  , rdoorbell0Queue  , Q_DIM(rdoorbell0Queue)       },
};
/* If QF_MAX_ACTIVE is incorrectly defined, the compiler says something like:
   rdoorbell0.c:68: error: size of array ‘Q_assert_compile’ is negative
 */
Q_ASSERT_COMPILE(QF_MAX_ACTIVE == Q_DIM(QF_active) - 1);


int main(int argc, char **argv)
{
 startmain:
	BSP_startmain();
	rdoorbell0_ctor();
	BSP_init(); /* initialize the Board Support Package */

	QF_run();

	goto startmain;
}

void rdoorbell0_ctor(void)
{
	QActive_ctor((QActive *)(&rdoorbell0), (QStateHandler)&rdoorbell0Initial);
}


static QState rdoorbell0Initial(struct RDoorbell0 *me)
{
	return Q_TRAN(&rdoorbell0State);
}


static QState rdoorbell0State(struct RDoorbell0 *me)
{
	switch (Q_SIG(me)) {
	case WATCHDOG_SIGNAL:
		BSP_watchdog(me);
		return Q_HANDLED();
	}
	return Q_SUPER(&QHsm_top);
}
