#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"
#include "value.h"
#include <list>
#include <unordered_map>
#include "environment.h"

class BinaryExp;
class NumberExp;
class FloatExp;
class Program;
class PrintStm;
class AssignStm;
class FunDec;
class ReturnStm;
class Body;
class VarDec;
class FcallExp;
class BoolExp;

// ──────────────────────────────────────────────
//   INTERFAZ VISITOR (para PrintVisitor)
//   Todas las visitas devuelven int
// ──────────────────────────────────────────────

class Visitor {
public:
    virtual int visit(BinaryExp* exp) = 0;
    virtual int visit(NumberExp* exp) = 0;
    virtual int visit(FloatExp* exp) = 0;
    virtual int visit(BoolExp* exp) = 0;
    virtual int visit(IdExp* exp) = 0;
    virtual int visit(Program* p) = 0;
    virtual int visit(PrintStm* stm) = 0;
    virtual int visit(AssignStm* stm) = 0;
    virtual int visit(ReturnStm* stm) = 0;
    virtual int visit(VarDec* vd) = 0;
    virtual int visit(Body* b) = 0;
    virtual int visit(FunDec* fd) = 0;
    virtual int visit(FcallExp* stm) = 0;
};

// ──────────────────────────────────────────────
//   INTERFAZ EVALVISITOR (para EVALVisitor)
//   Expresiones devuelven Value, sentencias void
// ──────────────────────────────────────────────

class EvalVisitor {
public:
    virtual Value visit(BinaryExp* exp) = 0;
    virtual Value visit(NumberExp* exp) = 0;
    virtual Value visit(FloatExp* exp) = 0;
    virtual Value visit(BoolExp* exp) = 0;
    virtual Value visit(IdExp* exp) = 0;
    virtual Value visit(FcallExp* exp) = 0;

    virtual void visit(PrintStm* stm) = 0;
    virtual void visit(AssignStm* stm) = 0;
    virtual void visit(ReturnStm* stm) = 0;
    virtual void visit(VarDec* vd) = 0;
    virtual void visit(Body* b) = 0;
    virtual void visit(FunDec* fd) = 0;
    virtual void visit(Program* p) = 0;

    virtual ~EvalVisitor() = default;
};

// ──────────────────────────────────────────────
//   PrintVisitor
// ──────────────────────────────────────────────

class PrintVisitor : public Visitor {
public:
    int visit(BinaryExp* exp) override;
    int visit(NumberExp* exp) override;
    int visit(FloatExp* exp) override;
    int visit(BoolExp* exp) override;
    int visit(IdExp* exp) override;
    int visit(Program* p) override;
    int visit(PrintStm* stm) override;
    int visit(AssignStm* stm) override;
    int visit(ReturnStm* stm) override;
    int visit(VarDec* vd) override;
    int visit(Body* b) override;
    int visit(FunDec* fd) override;
    int visit(FcallExp* stm) override;

    void imprimir(Program* program);
};

// ──────────────────────────────────────────────
//   EVALVisitor
//   Intérprete con soporte de int, float y bool
// ──────────────────────────────────────────────

class EVALVisitor : public EvalVisitor {
public:
    Environment<Value> env;
    unordered_map<string, FunDec*> envfun;
    Value retval;
    bool  retcall;

    Value visit(BinaryExp* exp) override;
    Value visit(NumberExp* exp) override;
    Value visit(FloatExp* exp) override;
    Value visit(BoolExp* exp) override;
    Value visit(IdExp* exp) override;
    Value visit(FcallExp* exp) override;

    void visit(PrintStm* stm) override;
    void visit(AssignStm* stm) override;
    void visit(ReturnStm* stm) override;
    void visit(VarDec* vd) override;
    void visit(Body* b) override;
    void visit(FunDec* fd) override;
    void visit(Program* p) override;

    void interprete(Program* program);
};

#endif // VISITOR_H
