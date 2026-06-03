#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <cmath>
using namespace std;

// ===========================================================
//  Valor en tiempo de ejecución del intérprete
//  Soporta: int, float, bool, void
// ===========================================================

class Value {
public:
    enum VType { INT_VAL, FLOAT_VAL, BOOL_VAL, VOID_VAL };
    VType vtype;
    int   ival;
    float fval;
    bool  bval;

    Value()              : vtype(INT_VAL),   ival(0),  fval(0.0f), bval(false) {}
    explicit Value(int v)   : vtype(INT_VAL),   ival(v),  fval(0.0f), bval(false) {}
    explicit Value(float v) : vtype(FLOAT_VAL), ival(0),  fval(v),    bval(false) {}
    explicit Value(bool v)  : vtype(BOOL_VAL),  ival(0),  fval(0.0f), bval(v)     {}

    static Value void_val() {
        Value v;
        v.vtype = VOID_VAL;
        return v;
    }

    void print() const {
        switch (vtype) {
            case INT_VAL:   cout << ival; break;
            case FLOAT_VAL: cout << fval; break;
            case BOOL_VAL:  cout << (bval ? "true" : "false"); break;
            case VOID_VAL:  break;
        }
    }
};

#endif // VALUE_H
