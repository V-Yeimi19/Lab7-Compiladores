#include "TypeChecker.h"
#include <iostream>
#include <stdexcept>
using namespace std;


Type* NumberExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* FloatExp::accept(TypeVisitor* v)  { return v->visit(this); }
Type* BoolExp::accept(TypeVisitor* v)   { return v->visit(this); }
Type* IdExp::accept(TypeVisitor* v)     { return v->visit(this); }
Type* BinaryExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* FcallExp::accept(TypeVisitor* v)  { return v->visit(this); }

void AssignStm::accept(TypeVisitor* v) { v->visit(this); }
void PrintStm::accept(TypeVisitor* v)  { v->visit(this); }
void ReturnStm::accept(TypeVisitor* v) { v->visit(this); }

void VarDec::accept(TypeVisitor* v)  { v->visit(this); }
void FunDec::accept(TypeVisitor* v)  { v->visit(this); }
void Body::accept(TypeVisitor* v)    { v->visit(this); }
void Program::accept(TypeVisitor* v) { v->visit(this); }

// ===========================================================
//   Constructor del TypeChecker
// ===========================================================

TypeChecker::TypeChecker() {
    intType   = new Type(Type::INT);
    boolType  = new Type(Type::BOOL);
    voidType  = new Type(Type::VOID);
    floatType = new Type(Type::FLOAT);
}

// ===========================================================
//   Registrar funciones globales
// ===========================================================

void TypeChecker::add_function(FunDec* fd) {
    if (functions.find(fd->nombre) != functions.end()) {
        cerr << "Error: función '" << fd->nombre << "' ya fue declarada." << endl;
        exit(0);
    }
    if (Type::string_to_type(fd->tipo) == Type::NOTYPE) {
        cerr << "Error: tipo de retorno inválido en función '" << fd->nombre << "'." << endl;
        exit(0);
    }
    functions[fd->nombre] = fd;
}

// ===========================================================
//   Método principal de verificación
// ===========================================================

void TypeChecker::typecheck(Program* program) {
    if (program) program->accept(this);
    cout << "Revisión exitosa" << endl;
}

// ===========================================================
//   Nivel superior: Programa y Bloque
// ===========================================================

void TypeChecker::visit(Program* p) {
    for (auto f : p->fdlist)
        add_function(f);

    env.add_level();
    for (auto v : p->vdlist)
        v->accept(this);
    for (auto f : p->fdlist)
        f->accept(this);
    env.remove_level();
}

void TypeChecker::visit(Body* b) {
    env.add_level();
    for (auto v : b->vdlist)
        v->accept(this);
    for (auto s : b->stmlist)
        s->accept(this);
    env.remove_level();
}

// ===========================================================
//   Declaraciones
// ===========================================================

void TypeChecker::visit(VarDec* v) {
    Type* t = new Type();
    if (!t->set_basic_type(v->tipo)) {
        cerr << "Error: tipo de variable '" << v->tipo << "' no válido." << endl;
        exit(0);
    }
    if (t->ttype == Type::VOID) {
        cerr << "Error: el tipo 'void' no es válido para variables." << endl;
        exit(0);
    }
    for (const auto& id : v->variables) {
        if (env.check(id)) {
            cerr << "Error: variable '" << id << "' ya declarada." << endl;
            exit(0);
        }
        env.add_var(id, t);
    }
}

void TypeChecker::visit(FunDec* f) {
    env.add_level();
    for (size_t i = 0; i < f->Nparametros.size(); ++i) {
        Type* pt = new Type();
        if (!pt->set_basic_type(f->Tparametros[i])) {
            cerr << "Error: tipo de parámetro '" << f->Tparametros[i]
                 << "' inválido en función '" << f->nombre << "'." << endl;
            exit(0);
        }
        env.add_var(f->Nparametros[i], pt);
    }
    Type* returnType = new Type();
    returnType->set_basic_type(f->tipo);
    retornodefuncion = returnType;
    f->cuerpo->accept(this);
    env.remove_level();
}

// ===========================================================
//   Sentencias
// ===========================================================

void TypeChecker::visit(PrintStm* stm) {
    Type* t = stm->e->accept(this);
    if (!(t->match(intType) || t->match(boolType) || t->match(floatType))) {
        cerr << "Error: tipo inválido en print (se esperaba int, float o bool)." << endl;
        exit(0);
    }
}

void TypeChecker::visit(AssignStm* stm) {
    if (!env.check(stm->id)) {
        cerr << "Error: variable '" << stm->id << "' no declarada." << endl;
        exit(0);
    }
    Type* varType = env.lookup(stm->id);
    Type* expType = stm->e->accept(this);
    if (!varType->match(expType)) {
        cerr << "Error: tipos incompatibles en asignación a '" << stm->id
             << "': se esperaba '" << Type::type_names[varType->ttype]
             << "', se recibió '" << Type::type_names[expType->ttype] << "'." << endl;
        exit(0);
    }
}

void TypeChecker::visit(ReturnStm* stm) {
    bool isVoid = retornodefuncion->match(voidType);

    if (isVoid) {
        if (stm->e != nullptr) {
            cerr << "Error: función void no puede retornar un valor." << endl;
            exit(0);
        }
        return;
    }

    if (stm->e == nullptr) {
        cerr << "Error: función no-void debe retornar un valor." << endl;
        exit(0);
    }

    Type* t = stm->e->accept(this);
    if (!t->match(retornodefuncion)) {
        cerr << "Error: tipo de retorno incorrecto: se esperaba '"
             << Type::type_names[retornodefuncion->ttype]
             << "', se retornó '" << Type::type_names[t->ttype] << "'." << endl;
        exit(0);
    }
}

// ===========================================================
//   Expresiones
// ===========================================================

Type* TypeChecker::visit(BinaryExp* e) {
    Type* left  = e->left->accept(this);
    Type* right = e->right->accept(this);

    switch (e->op) {
        case PLUS_OP:
        case MINUS_OP:
        case MUL_OP:
        case DIV_OP:
        case POW_OP:
            if (left->match(intType) && right->match(intType))
                return intType;
            if (left->match(floatType) && right->match(floatType))
                return floatType;
            cerr << "Error: operación aritmética requiere operandos del mismo tipo numérico (int o float)." << endl;
            exit(0);

        case LE_OP:
            if (!(left->match(intType)   && right->match(intType)) &&
                !(left->match(floatType) && right->match(floatType))) {
                cerr << "Error: operador '<' requiere operandos numéricos del mismo tipo." << endl;
                exit(0);
            }
            return boolType;

        case AND_OP:
            if (!(left->match(boolType) && right->match(boolType))) {
                cerr << "Error: operador 'and' requiere operandos bool." << endl;
                exit(0);
            }
            return boolType;

        default:
            cerr << "Error: operador binario no soportado." << endl;
            exit(0);
    }
}

Type* TypeChecker::visit(NumberExp* e) { return intType;   }
Type* TypeChecker::visit(FloatExp* e)  { return floatType; }
Type* TypeChecker::visit(BoolExp* e)   { return boolType;  }

Type* TypeChecker::visit(IdExp* e) {
    if (!env.check(e->value)) {
        cerr << "Error: variable '" << e->value << "' no declarada." << endl;
        exit(0);
    }
    return env.lookup(e->value);
}

Type* TypeChecker::visit(FcallExp* e) {
    auto it = functions.find(e->nombre);
    if (it == functions.end()) {
        cerr << "Error: llamada a función no declarada '" << e->nombre << "'." << endl;
        exit(0);
    }
    FunDec* fd = it->second;

    // Verificar cantidad de argumentos
    size_t expected = fd->Nparametros.size();
    size_t received = e->argumentos.size();
    if (received != expected) {
        cerr << "Error: llamada a '" << fd->nombre << "': se esperaban "
             << expected << " argumento(s), se recibieron " << received << "." << endl;
        exit(0);
    }

    // Verificar tipos de argumentos
    size_t i = 0;
    for (auto arg : e->argumentos) {
        Type* argType = arg->accept(this);
        Type::TType expectedTType = Type::string_to_type(fd->Tparametros[i]);
        if (argType->ttype != expectedTType) {
            cerr << "Error: argumento " << (i + 1) << " en llamada a '" << fd->nombre
                 << "': se esperaba '" << fd->Tparametros[i]
                 << "', se recibió '" << Type::type_names[argType->ttype] << "'." << endl;
            exit(0);
        }
        i++;
    }

    return new Type(Type::string_to_type(fd->tipo));
}
