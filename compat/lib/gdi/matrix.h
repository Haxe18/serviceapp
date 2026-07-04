#ifndef __lib_gdi_matrix_h
#define __lib_gdi_matrix_h

class eMatrix4x4
{
public:
    float m[4][4];
    eMatrix4x4() {}
    static eMatrix4x4 identity() { return eMatrix4x4(); }
};

#endif
