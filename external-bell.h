#ifndef external_bell_h_INCLUDED
#define external_bell_h_INCLUDED

#include "qpn_port.h"


struct ExternalBell {
	QActive super;
};


extern struct ExternalBell externalbell;

void externalbell_ctor(void);

#endif // external_bell_h_INCLUDED
