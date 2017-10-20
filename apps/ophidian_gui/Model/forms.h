#ifndef FORMS_H
#define FORMS_H

#include <ophidian/circuit/Netlist.h>

class Quad {
public:
    Quad();

    ophidian::circuit::Cell mCell;
    uint32_t mId;

    bool operator==(const Quad & o) const;
};

#endif // FORMS_H