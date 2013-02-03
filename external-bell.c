#include "external-bell.h"
#include "bsp.h"


struct ExternalBell externalbell;


static QState initialState(struct ExternalBell *me);
static QState topState(struct ExternalBell *me);
static QState offState(struct ExternalBell *me);


void externalbell_ctor(struct ExternalBell *externalbell)
{
	QActive_ctor((QActive *)(&externalbell),
		     (QStateHandler)&initialState);
}


static QState initialState(struct ExternalBell *me)
{
	return Q_TRAN(&offState);
}


static QState topState(struct ExternalBell *me)
{
	return Q_SUPER(&QHsm_top);
}


static QState offState(struct ExternalBell *me)
{
	switch (Q_SIG(me)) {
	case Q_ENTRY_SIG:
		BSP_buzzer(0);
		return Q_HANDLED();
	default:
		return Q_SUPER(topState);
	}
}
