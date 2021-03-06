#include "external-bell.h"
#include "bsp.h"


Q_DEFINE_THIS_FILE;


struct ExternalBell externalbell;


static QState initialState(struct ExternalBell *me);
static QState topState(struct ExternalBell *me);
static QState offState(struct ExternalBell *me);
static QState highState(struct ExternalBell *me);
static QState lowState(struct ExternalBell *me);
static QState buzzerState(struct ExternalBell *me);


void externalbell_ctor(void)
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
		BSP_buzzer(0, 0);
		return Q_HANDLED();
	case EXTERNAL_BELL_SIGNAL:
		return Q_TRAN(highState);
	case EXTERNAL_BUZZER_SIGNAL:
		return Q_TRAN(buzzerState);
	}
	return Q_SUPER(topState);
}


static QState highState(struct ExternalBell *me)
{
	switch (Q_SIG(me)) {
	case Q_ENTRY_SIG:
		BSP_buzzer(external_bell_high, 4);
		QActive_arm((QActive*)me, 7);
		return Q_HANDLED();
	case Q_TIMEOUT_SIG:
		return Q_TRAN(lowState);
	case Q_EXIT_SIG:
		BSP_buzzer(0, 0);
		return Q_HANDLED();
	}
	return Q_SUPER(topState);
}


static QState lowState(struct ExternalBell *me)
{
	switch (Q_SIG(me)) {
	case Q_ENTRY_SIG:
		BSP_buzzer(external_bell_low, 4);
		QActive_arm((QActive*)me, 7);
		return Q_HANDLED();
	case Q_TIMEOUT_SIG:
		return Q_TRAN(offState);
	case Q_EXIT_SIG:
		BSP_buzzer(0, 0);
		return Q_HANDLED();
	}
	return Q_SUPER(topState);
}


static QState buzzerState(struct ExternalBell *me)
{
	switch (Q_SIG(me)) {
	case Q_ENTRY_SIG:
		BSP_buzzer(external_bell_buzz, 8);
		QActive_arm((QActive*)me, 7);
		return Q_HANDLED();
	case Q_TIMEOUT_SIG:
		return Q_TRAN(offState);
	case Q_EXIT_SIG:
		BSP_buzzer(0, 0);
		return Q_HANDLED();
	}
	return Q_SUPER(topState);
}
