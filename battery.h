#ifndef battery_h_INCLUDED
#define battery_h_INCLUDED

#include "qpn_port.h"


struct Battery {
	QActive super;
};


extern struct Battery battery;


void battery_ctor(struct Battery *battery);

#endif
