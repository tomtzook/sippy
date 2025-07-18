
#include <sip/bodies.h>

#include "serialization/matchers.h"
#include "serialization/reader.h"


using namespace sippy;


DEFINE_SIP_BODY_READ(test) {
    is >> b.v;
}

DEFINE_SIP_BODY_WRITE(test) {
    os << b.v;
}
