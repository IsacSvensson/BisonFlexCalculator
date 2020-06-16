#ifndef CALC_HPP
#define CALC_HPP

/* Symboltable */
struct symbol{
    char* name;
    double value;
    struct ast* func;
    struct symlist* syms;
};

struct symbol* lookup(char*);

/* list of symbols, for an argument list */
struct symlist
{
    struct symbol* sym;
    struct symlist* next;
};

struct symlist* newsymlist(struct symbol* sym, struct symlist* next);
void symlistfree(struct symlist* sl);

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
    struct ast* left;
    struct ast* right;
};

struct fncall
{
    int nodetype;
    struct ast* left;
    enum bifs functype;
};

struct ufncall
{
    int nodetype;
    struct ast* left;
    struct symbol* s;
};

struct flow
{
    int nodetype;
    struct ast* cond;
    struct ast* tl;
    struct ast* el;
};

struct numval
{
    int nodetype;
    double number;
};

struct symval
{
    int nodetype;
    struct symbol* s;
};

struct symasgn
{
    int nodetype;
    struct symbol *s;
    struct ast* v;
};

struct symref
{
    int nodetype;
    struct symbol* s;
};



/* Build an AST */
struct ast* newast(int nodetype, struct ast* left, struct ast* right);
struct ast* newcmp(int cmptype, struct ast* left, struct ast* right);
struct ast* newfunc(int functype, struct ast* left);
struct ast* newcall(struct symbol* s, struct ast* left);
struct ast* newref(struct symbol* s);
struct ast* newasgn(struct symbol* s, struct ast* v);
struct ast* newflow(int nodetype, struct ast* cond, struct ast* tl, struct ast* tr);
struct ast* newnum(double d);

/* Define a function */
void dodef(struct symbol* name, struct symlist* syms, struct ast* stmts);

/* Evaluate an AST */
double eval(struct ast*);

/* Delete an free an AST */
void treefree(struct ast*);

/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(char *s, ...);

extern int debug;
void dumpast(struct ast *a, int level);


#endif 