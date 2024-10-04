#ifndef _STRUCTURE_LIGHT
#define _STRUCTURE_LIGHT

struct LightAttribute {
    uint  type;
    uint  elementIndex;
    float pdf;
    float emissionFrac;
};

struct AliasTableElement {
    float probability;
    uint  failIndex;
};

#endif // _STRUCTURE_LIGHT