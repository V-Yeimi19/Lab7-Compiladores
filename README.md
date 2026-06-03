# Laboratorio 11 — Type Checker y soporte de tipos Float/Bool

Compilador educativo en C++ con chequeo de tipos estático, soporte de `int`, `float`, `bool` y `void`, e intérprete de árbol (tree-walk interpreter).

---

## Arquitectura del compilador

```
Archivo fuente
      |
      v
   Scanner           (scanner.cpp / scanner.h)
      |  tokens
      v
   Parser            (parser.cpp / parser.h)
      |  AST (Program*)
      v
 PrintVisitor        (visitor.cpp) — imprime el AST en texto legible
      |
      v
 TypeChecker         (TypeChecker.cpp) — verifica tipos estáticos
      |
      v
 EVALVisitor         (visitor.cpp) — intérprete tree-walk
```

Si el TypeChecker detecta un error, el programa termina antes de llegar a la ejecución.

---

## Patrón Visitor — tres interfaces

El proyecto define tres interfaces independientes:

| Interfaz | Implementada por | Expresiones devuelven | Sentencias devuelven |
|----------|-----------------|----------------------|---------------------|
| `Visitor` | `PrintVisitor` | `int` (siempre 0) | `int` (siempre 0) |
| `TypeVisitor` | `TypeChecker` | `Type*` (tipo inferido) | `void` |
| `EvalVisitor` | `EVALVisitor` | `Value` (valor en tiempo de ejecución) | `void` |

**Por qué se creó `EvalVisitor` separada:** la interfaz `Visitor` original retorna `int`. Al agregar `float` y `bool`, retornar `int` trunca los valores flotantes y pierde la distinción semántica de `bool` (que debe imprimirse `"true"`/`"false"`, no `1`/`0`). La nueva interfaz `EvalVisitor` resuelve esto sin modificar `PrintVisitor`.

---

## Cambios realizados

### 1. Nuevos tipos: `FLOAT` y `BOOL` — `semantic_types.h`

```cpp
// Antes
enum TType { NOTYPE, VOID, INT, BOOL };
static const char* type_names[4];

// Después
enum TType { NOTYPE, VOID, INT, BOOL, FLOAT };
static const char* type_names[5];
// string_to_type() ahora reconoce "float"
// type_names[5] = { "notype", "void", "int", "bool", "float" }
```

`BOOL` existía en el enum pero `FLOAT` es completamente nuevo. Ambos son tipos de primera clase que el TypeChecker puede verificar, rechazar y propagar.

---

### 2. Clase `Value` — `value.h` (archivo nuevo)

```cpp
class Value {
public:
    enum VType { INT_VAL, FLOAT_VAL, BOOL_VAL, VOID_VAL };
    VType vtype;
    int   ival;
    float fval;
    bool  bval;

    explicit Value(int v);    // INT_VAL
    explicit Value(float v);  // FLOAT_VAL
    explicit Value(bool v);   // BOOL_VAL
    static Value void_val();  // VOID_VAL

    void print() const;  // "true"/"false" para bool, valor numérico para int/float
};
```

Cada expresión evaluada por `EVALVisitor` produce un `Value`. `print()` formatea `bool` como texto en lugar de número.

---

### 3. Soporte de literales float

Archivos modificados y qué hace cada uno:

- **`token.h`**: agrega `FLOAT_NUM` al enum `Token::Type`.
- **`token.cpp`**: agrega el case `FLOAT_NUM` en el operador `<<`.
- **`scanner.cpp`**: detecta patrones `<dígitos>.<dígitos>` y emite `FLOAT_NUM`. La verificación `isdigit(input[current+1])` requiere dígito después del punto (evita `3.` como float).
- **`ast.h`**: nueva clase `FloatExp : public Exp` con `float value` y las tres sobrecargas de `accept()`.
- **`parser.cpp`**: `parseF()` reconoce `FLOAT_NUM` → `FloatExp(stof(...))`.

**Ejemplo ahora válido:**
```
var float x;
x = 3.14;
print(x)   → imprime 3.14
```

---

### 4. TypeChecker — validaciones de tipo implementadas

#### a) Tipo de retorno incompatible — `visit(ReturnStm*)`
```
fun bool suma(int a, int b)
    var int y;
    y = a + b;
    return(y)        ← y es int, la función espera bool
endfun
```
```
Error: tipo de retorno incorrecto: se esperaba 'bool', se retornó 'int'.
```

#### b) Función `void` con `return` — `visit(ReturnStm*)`
```
fun void foo()
    return(5)        ← void no puede retornar valor
endfun
```
```
Error: función void no puede retornar un valor.
```

#### c) Cantidad de argumentos incorrecta — `visit(FcallExp*)`
```
fun int suma(int a, int b)  ...endfun
suma(1, 2, 3)    ← 3 argumentos, se esperaban 2
```
```
Error: llamada a 'suma': se esperaban 2 argumento(s), se recibieron 3.
```

#### d) Tipos de argumentos incorrectos — `visit(FcallExp*)`
```
fun int suma(int a, int b)  ...endfun
suma(true, false)   ← bool ≠ int
```
```
Error: argumento 1 en llamada a 'suma': se esperaba 'int', se recibió 'bool'.
```

#### e) Tipo incompatible en asignación — `visit(AssignStm*)`
```
var bool a;
a = 42;     ← int ≠ bool
```
```
Error: tipos incompatibles en asignación a 'a': se esperaba 'bool', se recibió 'int'.
```

#### f) Variable no declarada — `visit(IdExp*)` y `visit(AssignStm*)`
```
fun int main()
    x = 5    ← x no fue declarada
endfun
```
```
Error: variable 'x' no declarada.
```

#### g) Operandos aritméticos de tipos distintos — `visit(BinaryExp*)`
```
var int a;
var float b;
print(a + b)   ← int ≠ float en operación aritmética
```
```
Error: operación aritmética requiere operandos del mismo tipo numérico (int o float).
```

#### h) Operandos de `and` deben ser `bool` — `visit(BinaryExp*)`
```
var int x;
print(x and x)   ← int no es bool
```
```
Error: operador 'and' requiere operandos bool.
```

#### i) Tipo `void` no válido para variables — `visit(VarDec*)`
```
var void x;    ← void no es tipo de variable válido
```
```
Error: el tipo 'void' no es válido para variables.
```

#### j) Tipo de argumento float en funciones — `visit(FcallExp*)`
```
fun float cuadrado(float x)  ...endfun
cuadrado(3)    ← int ≠ float
```
```
Error: argumento 1 en llamada a 'cuadrado': se esperaba 'float', se recibió 'int'.
```

---

### 5. EVALVisitor — migración a `EvalVisitor`

**Cambios en `visitor.h`:**
- Nueva clase abstracta `EvalVisitor` con `virtual Value visit(XxxExp*)` y `virtual void visit(XxxStm*)`.
- `EVALVisitor` ahora extiende `EvalVisitor` en lugar de `Visitor`.
- `Environment<int> env` → `Environment<Value> env`.
- `int retval` → `Value retval`.

**Cambios en `ast.h`:**
- Cada nodo tiene las tres sobrecargas de `accept()`:
  - `int accept(Visitor*)`
  - `Type* accept(TypeVisitor*)`
  - `Value accept(EvalVisitor*)`

**Cambios en `visitor.cpp`:**
- Sección nueva con todos los `accept(EvalVisitor*)`.
- `visit(BoolExp*)` → `Value((bool)(exp->value != 0))` — bool real, no entero.
- `visit(FloatExp*)` → `Value(exp->value)` — float nativo.
- `visit(BinaryExp*)` — aritmética de float con lambda `toFloat()`.
- `visit(PrintStm*)` → `v.print()` — imprime `true`/`false` para bool.
- `visit(VarDec*)` — inicialización por tipo: `bool` → `false`, `float` → `0.0f`, `int` → `0`.
- `visit(FcallExp*)` — void functions devuelven `Value::void_val()` en lugar de error.

---

### 6. Tabla de archivos modificados/creados

| Archivo | Tipo | Cambios |
|---------|------|---------|
| `semantic_types.h` | Modificado | Enum con `FLOAT`; `string_to_type("float")`; `type_names[5]` |
| `TypeChecker.h` | Modificado | `floatType`; `functions` como `FunDec*`; visita `FloatExp` |
| `TypeChecker.cpp` | Modificado | 10 validaciones implementadas; `floatType` en constructor; `add_function` guarda `FunDec*` |
| `value.h` | **Nuevo** | Clase `Value` con INT/FLOAT/BOOL/VOID_VAL |
| `token.h` | Modificado | Token `FLOAT_NUM` |
| `token.cpp` | Modificado | Case `FLOAT_NUM` en operador `<<` |
| `scanner.cpp` | Modificado | Reconocimiento de literales `3.14` → `FLOAT_NUM` |
| `ast.h` | Modificado | `FloatExp`; `accept(EvalVisitor*)` en todos los nodos |
| `visitor.h` | Modificado | Interfaz `EvalVisitor`; `EVALVisitor` extendida con `Value` |
| `visitor.cpp` | Modificado | `accept(EvalVisitor*)` implementados; `EVALVisitor` reescrito con `Value` |
| `parser.cpp` | Modificado | `parseF()` reconoce `FLOAT_NUM` → `FloatExp` |

---

## Ejemplo de programa rechazado

```
var int x,z;
fun bool suma(int a, int b)
    var int y;
    y = a + b;
    return(y)
endfun
fun int main()
    var bool a;
    a = suma(2,3);
    print(a);
    return(0)
endfun
```

Salida:
```
Parseo exitoso
=== Iniciando verificación de tipos ===
Error: tipo de retorno incorrecto: se esperaba 'bool', se retornó 'int'.
```

## Ejemplo de programa válido con float y bool

```
fun float cuadrado(float x)
    var float r;
    r = x * x;
    return(r)
endfun
fun int main()
    var int n;
    n = 4;
    print(n < 10);
    print(cuadrado(2.5));
    return(0)
endfun
```

Salida:
```
Parseo exitoso
=== Iniciando verificación de tipos ===
Revisión exitosa
=== Iniciando ejecución del programa ===
Interprete:
true
6.25
```

---

## Compilación y ejecución

```bash
g++ -std=c++17 -o compilador main.cpp scanner.cpp parser.cpp ast.cpp visitor.cpp TypeChecker.cpp token.cpp
./compilador <archivo.txt>
```
