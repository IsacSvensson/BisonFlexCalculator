#ifndef CALC_HPP
#define CALC_HPP

extern int yylineno;
void yyerror(char* s, ...);

/* Symboltable */
struct symbol{
    char* name;
    double value;
    struct ast* func;
    struct symlist* syms;
};

/* simple symtab of fixed size */
#define NHASH 9997
symbol symtab[NHASH];

symbol* lookup(char*);

/* list of symbols, for an argument list */
struct symlist
{
    symbol* sym;
    symlist* next;
};

symlist* newsymlist(symbol* sym, symlist* next);
void symlistfree(symlist* sl);

/* Node types:
 * + - * / |    Arithmetical operators
 * 0-7          Comparison ops, bit coded 04 equal, 02 less, 01 greater
 * M            Unary minus
 * L            Expression or statement list
 * I            IF statement
 * W            WHILE statement
 * N            Symbol ref
 * =            Assignment
 * S            List of symbols
 * F            Built in function call
 * C            User function call
 */

// Built in functions
enum bifs{
    B_sqrt = 1,
    B_exp,
    B_log,
    B_print
};

struct ast
{
    int nodetype;
    ast* left;
    ast* right;
};

struct fncall
{
    int nodetype;
    ast* left;
    bifs functype;
};

struct ufncall
{
    int nodetype;
    ast* left;
    symbol* s;
};

struct flow
{
    int nodetype;
    ast* cond;
    ast* tl;
    ast* el;
};

struct numval
{
    int nodetype;
    double number;
};

struct symval
{
    int nodetype;
    symbol* s;
};

struct symasgn
{
    int nodetype;
    symbol *s;
    ast* v;
};

struct symref
{
    int nodetype;
    symbol* s;
};



/* Build an AST */
ast* newast(int nodetype, ast* left, ast* right);
ast* newcmp(int cmptype, ast* left, ast* right);
ast* newfunc(int functype, ast* left);
ast* newcall(symbol* s, ast* left);
ast* newref(symbol* s);
ast* newasgn(symbol* s, ast* v);
ast* newflow(int nodetype, ast* cond, ast* tl, ast* tr);
ast* newnum(double d);

/* Define a function */
void dodef(symbol* name, symlist* syms, ast* stmts);

/* Evaluate an AST */
double eval(ast*);

/* Delete an free an AST */
void treefree(ast*);



#endif 