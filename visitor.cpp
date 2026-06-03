#include <iostream>
#include <fstream>
#include <cmath>
#include "ast.h"
#include "visitor.h"

using namespace std;


///////////////////////////////////////////////////////////////////////////////////
//                    SECCIÓN 1: MÉTODOS accept() — Visitor (int)
///////////////////////////////////////////////////////////////////////////////////

// Expresiones
int BinaryExp::accept(Visitor* visitor) { return visitor->visit(this); }
int NumberExp::accept(Visitor* visitor)  { return visitor->visit(this); }
int FloatExp::accept(Visitor* visitor)   { return visitor->visit(this); }
int BoolExp::accept(Visitor* visitor)    { return visitor->visit(this); }
int IdExp::accept(Visitor* visitor)      { return visitor->visit(this); }
int FcallExp::accept(Visitor* visitor)   { return visitor->visit(this); }

// Sentencias
int PrintStm::accept(Visitor* visitor)  { return visitor->visit(this); }
int AssignStm::accept(Visitor* visitor) { return visitor->visit(this); }
int ReturnStm::accept(Visitor* visitor) { return visitor->visit(this); }

// Declaraciones
int VarDec::accept(Visitor* visitor)    { return visitor->visit(this); }
int FunDec::accept(Visitor* visitor)    { return visitor->visit(this); }

// Estructuras compuestas
int Body::accept(Visitor* visitor)      { return visitor->visit(this); }
int Program::accept(Visitor* visitor)   { return visitor->visit(this); }

///////////////////////////////////////////////////////////////////////////////////
//                    SECCIÓN 2: MÉTODOS accept() — EvalVisitor (Value)
///////////////////////////////////////////////////////////////////////////////////

// Expresiones
Value BinaryExp::accept(EvalVisitor* v) { return v->visit(this); }
Value NumberExp::accept(EvalVisitor* v)  { return v->visit(this); }
Value FloatExp::accept(EvalVisitor* v)   { return v->visit(this); }
Value BoolExp::accept(EvalVisitor* v)    { return v->visit(this); }
Value IdExp::accept(EvalVisitor* v)      { return v->visit(this); }
Value FcallExp::accept(EvalVisitor* v)   { return v->visit(this); }

// Sentencias
void PrintStm::accept(EvalVisitor* v)  { v->visit(this); }
void AssignStm::accept(EvalVisitor* v) { v->visit(this); }
void ReturnStm::accept(EvalVisitor* v) { v->visit(this); }

// Declaraciones
void VarDec::accept(EvalVisitor* v)    { v->visit(this); }
void FunDec::accept(EvalVisitor* v)    { v->visit(this); }
void Body::accept(EvalVisitor* v)      { v->visit(this); }
void Program::accept(EvalVisitor* v)   { v->visit(this); }

///////////////////////////////////////////////////////////////////////////////////
//                    SECCIÓN 3: IMPLEMENTACIÓN DE PrintVisitor
///////////////////////////////////////////////////////////////////////////////////

int PrintVisitor::visit(NumberExp* exp) {
    cout << exp->value;
    return 0;
}

int PrintVisitor::visit(FloatExp* exp) {
    cout << exp->value;
    return 0;
}

int PrintVisitor::visit(IdExp* exp) {
    cout << exp->value;
    return 0;
}

int PrintVisitor::visit(BoolExp* exp) {
    cout << (exp->value ? "true" : "false");
    return 0;
}

int PrintVisitor::visit(BinaryExp* exp) {
    exp->left->accept(this);
    cout << ' ' << Exp::binopToChar(exp->op) << ' ';
    exp->right->accept(this);
    return 0;
}

int PrintVisitor::visit(PrintStm* stm) {
    cout << "print(";
    stm->e->accept(this);
    cout << ")" << endl;
    return 0;
}

int PrintVisitor::visit(AssignStm* stm) {
    cout << stm->id << "=";
    stm->e->accept(this);
    cout << endl;
    return 0;
}

int PrintVisitor::visit(ReturnStm* stm) {
    cout << "return (";
    if (stm->e) stm->e->accept(this);
    cout << ")" << endl;
    return 0;
}

int PrintVisitor::visit(VarDec* vd) {
    cout << "var " << vd->tipo << " ";
    for (auto i : vd->variables)
        cout << i << ",";
    cout << endl;
    return 0;
}

int PrintVisitor::visit(Body* b) {
    cout << "{\n";
    for (auto i : b->vdlist)  i->accept(this);
    for (auto i : b->stmlist) i->accept(this);
    cout << "}\n";
    return 0;
}

int PrintVisitor::visit(FcallExp* fcall) {
    cout << fcall->nombre << "(";
    for (auto i : fcall->argumentos) {
        i->accept(this);
        cout << ",";
    }
    cout << ")";
    return 0;
}

int PrintVisitor::visit(FunDec* fd) {
    cout << "fun " << fd->tipo << " " << fd->nombre << "(";
    for (size_t i = 0; i < fd->Nparametros.size(); ++i) {
        cout << fd->Tparametros[i] << " " << fd->Nparametros[i];
        if (i < fd->Nparametros.size() - 1)
            cout << ", ";
    }
    cout << ") ";
    fd->cuerpo->accept(this);
    cout << endl;
    return 0;
}

int PrintVisitor::visit(Program* p) {
    for (auto d : p->vdlist) d->accept(this);
    for (auto d : p->fdlist) d->accept(this);
    return 0;
}

void PrintVisitor::imprimir(Program* programa) {
    if (programa) programa->accept(this);
}

///////////////////////////////////////////////////////////////////////////////////
//                    SECCIÓN 4: IMPLEMENTACIÓN DE EVALVisitor
///////////////////////////////////////////////////////////////////////////////////

Value EVALVisitor::visit(NumberExp* exp) {
    return Value(exp->value);
}

Value EVALVisitor::visit(FloatExp* exp) {
    return Value(exp->value);
}

Value EVALVisitor::visit(BoolExp* exp) {
    return Value((bool)(exp->value != 0));
}

Value EVALVisitor::visit(IdExp* exp) {
    return env.lookup(exp->value);
}

Value EVALVisitor::visit(BinaryExp* exp) {
    Value v1 = exp->left->accept(this);
    Value v2 = exp->right->accept(this);

    auto toFloat = [](const Value& v) -> float {
        return v.vtype == Value::FLOAT_VAL ? v.fval : static_cast<float>(v.ival);
    };

    switch (exp->op) {
        case PLUS_OP:
            if (v1.vtype == Value::FLOAT_VAL || v2.vtype == Value::FLOAT_VAL)
                return Value(toFloat(v1) + toFloat(v2));
            return Value(v1.ival + v2.ival);

        case MINUS_OP:
            if (v1.vtype == Value::FLOAT_VAL || v2.vtype == Value::FLOAT_VAL)
                return Value(toFloat(v1) - toFloat(v2));
            return Value(v1.ival - v2.ival);

        case MUL_OP:
            if (v1.vtype == Value::FLOAT_VAL || v2.vtype == Value::FLOAT_VAL)
                return Value(toFloat(v1) * toFloat(v2));
            return Value(v1.ival * v2.ival);

        case DIV_OP:
            if (v1.vtype == Value::FLOAT_VAL || v2.vtype == Value::FLOAT_VAL) {
                float d = toFloat(v2);
                if (d == 0.0f) { cout << "Error: división por cero" << endl; return Value(0.0f); }
                return Value(toFloat(v1) / d);
            }
            if (v2.ival == 0) { cout << "Error: división por cero" << endl; return Value(0); }
            return Value(v1.ival / v2.ival);

        case POW_OP:
            if (v1.vtype == Value::FLOAT_VAL || v2.vtype == Value::FLOAT_VAL)
                return Value(powf(toFloat(v1), toFloat(v2)));
            return Value(static_cast<int>(pow(v1.ival, v2.ival)));

        case LE_OP:
            if (v1.vtype == Value::FLOAT_VAL || v2.vtype == Value::FLOAT_VAL)
                return Value((bool)(toFloat(v1) < toFloat(v2)));
            return Value((bool)(v1.ival < v2.ival));

        case AND_OP:
            return Value((bool)(v1.bval && v2.bval));

        default:
            cout << "Error: operador desconocido" << endl;
            return Value(0);
    }
}

void EVALVisitor::visit(PrintStm* p) {
    Value v = p->e->accept(this);
    v.print();
    cout << endl;
}

void EVALVisitor::visit(AssignStm* p) {
    Value v = p->e->accept(this);
    env.update(p->id, v);
}

void EVALVisitor::visit(ReturnStm* stm) {
    retcall = true;
    if (stm->e)
        retval = stm->e->accept(this);
    else
        retval = Value::void_val();
}

void EVALVisitor::visit(VarDec* vd) {
    for (auto& id : vd->variables) {
        if (vd->tipo == "bool")
            env.add_var(id, Value(false));
        else if (vd->tipo == "float")
            env.add_var(id, Value(0.0f));
        else
            env.add_var(id, Value(0));
    }
}

void EVALVisitor::visit(Body* b) {
    env.add_level();
    for (auto i : b->vdlist)  i->accept(this);
    for (auto i : b->stmlist) i->accept(this);
    env.remove_level();
}

Value EVALVisitor::visit(FcallExp* fcall) {
    retcall = false;
    vector<Value> args;
    for (auto i : fcall->argumentos)
        args.push_back(i->accept(this));

    FunDec* fd = envfun[fcall->nombre];
    env.add_level();
    for (size_t i = 0; i < args.size(); ++i)
        env.add_var(fd->Nparametros[i], args[i]);

    fd->cuerpo->accept(this);
    env.remove_level();

    if (retcall) return retval;
    return Value::void_val();
}

void EVALVisitor::visit(FunDec* fd) {
    envfun[fd->nombre] = fd;
}

void EVALVisitor::visit(Program* p) {
    env.add_level();
    for (auto i : p->vdlist) i->accept(this);
    for (auto i : p->fdlist) i->accept(this);
    if (envfun.count("main")) {
        envfun["main"]->cuerpo->accept(this);
    } else {
        cout << "Error: no existe main" << endl;
        exit(0);
    }
    env.remove_level();
}

void EVALVisitor::interprete(Program* programa) {
    if (programa) {
        cout << "Interprete:" << endl;
        programa->accept(this);
    }
}
