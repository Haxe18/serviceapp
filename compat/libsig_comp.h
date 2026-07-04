#ifndef __LIBSIG_COMP_H
#define __LIBSIG_COMP_H

#include <lib/base/sigc.h>

#ifndef CONNECT
#define CONNECT(_signal, _slot) _signal.connect(sigc::mem_fun(*this, &_slot))
#endif

#ifndef CONNECT_1_0
#define CONNECT_1_0(SENDER, EMPFAENGER, PARAM) SENDER.connect( sigc::bind( sigc::mem_fun(*this, &EMPFAENGER) ,PARAM ) )
#endif

#endif // __LIBSIG_COMP_H
