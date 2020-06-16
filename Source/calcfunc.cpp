#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cmath>
#include "calc.hpp"

/* simple symtab of fixed size */
#define NHASH 9997
struct symbol symtab[NHASH];

/* Symboltable */
static unsigned symhash(char* sym){
    unsigned int hash = 0;
    unsigned c;

    while (c = *sym++)
        hash = hash * 9 ^ c;
    
    return hash;
}

symbol* lookup(char* sym){
    symbol* sp = &symtab[symhash(sym)%NHASH];
    int scount = NHASH;

    while(--scount >= 0){
        if(sp->name && !strcmp(sp->name, sym))
            return sp;
        if(!sp->name){
            sp->name = strdup(sym);
            sp->value = 0;
            sp->func = nullptr;
            sp->syms = nullptr;
            return sp;
        }
        if(++sp >= symtab + NHASH) 
            sp = symtab; 
    }
    yyerror("Symbol table overflow\n");
    abort();
}

ast* newast(int nodetype, ast* l, ast* r){
    ast* a = new ast();

    if(!a){
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->left = l;
    a->right = r;
    return a;
}

ast* newnum(double d){
    numval* a = new numval();

    if(!a){
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = 'K';
    a->number = d;
    return (ast*)a;
}

ast* newcmp(int cmptype, ast* l, ast* r){
    ast* a = new ast();

    if (!a){
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = '0' + cmptype;
    a->left = l;
    a->right = r;
    return a;
}

ast* newfunc(int functype, ast* l){
    fncall* a = new fncall();

    if (!a){
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = 'F';
    a->left = l;
    a->functype = (bifs)functype;
    return (struct ast *)a;
}

ast* newcall(symbol* s, ast* l){
    ufncall* a = new ufncall();

    if (!a){
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = 'C';
    a->left = l;
    a->s = s;

    return (ast*)a;
}

ast* newref(symbol* s){
    symref* a = new symref();

    if (!a){
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = 'N';
    a->s = s;

    return (ast*)a;
}

ast* newasgn(symbol* s, ast* v){
    symasgn* a = new symasgn();

    if (!a){
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = '=';
    a->s = s;
    a->v = v;

    return (ast*)a;
}

ast* newflow(int nodetype, ast* cond, ast* tl, ast* el){
    flow* a = new flow();

    if (!a){
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->el = el;

    return (ast*)a;
}

void treefree(ast* a){
    switch (a->nodetype)
    {
    case '+': 
    case '-': 
    case '*': 
    case '/': 
    case '1': case '2': case '3': case '4': case '5': case '6':
    case 'L':
        treefree(a->right);
    case '|':
    case 'M': case 'C': case 'F':
        treefree(a->left);
    case 'K': case 'N':
        break;
    case '=':
        delete ((symasgn*)a)->v;
        break;
    case 'I': case 'W':
        delete ((flow*)a)->cond;
        if(((flow*)a)->tl)
            delete ((flow*)a)->tl;
        if(((flow*)a)->el)
            delete ((flow*)a)->el;
        break;
    default:
        printf("Internal error: Bad node %c\n", a->nodetype);
    }
    delete a;
}

symlist* newsymlist(symbol* sym, symlist* next){
    symlist* sl = new symlist();

    if (!sl){
        yyerror("Out of space");
        exit(0);
    }
    sl->sym = sym;
    sl->next = next;

    return sl;
}

void symlistfree(symlist* sl){
    symlist* nsl;

    while (sl){
        nsl = sl->next;
        delete sl;
        sl = nsl;
    }
}

static double callbuiltin(fncall*);
static double calluser(ufncall*);

double eval(ast* a){
    double v;

    if(!a){
        yyerror("Internal error: null eval");
        return 0.0;
    }

    switch(a->nodetype)
    {
    case 'K': v = (((numval*)a)->number); break;
    case 'N': v = (((symref*)a)->s->value); break;
    case '=': v = (((symasgn*)a)->s->value = eval(((symasgn*)a)->v)); break;
    case '+': v = eval(a->left) + eval(a->right); break;
    case '-': v = eval(a->left) - eval(a->right); break;
    case '*': v = eval(a->left) * eval(a->right); break;
    case '/': v = eval(a->left) / eval(a->right); break;
    case '|': v = eval(a->left); if(v < 0) v = -v; break;
    case 'M': v = -eval(a->left); break;

    case '1': v = (eval(a->left) < eval(a->right))? 1 : 0; break;
    case '2': v = (eval(a->left) > eval(a->right))? 1 : 0; break;
    case '3': v = (eval(a->left) != eval(a->right))? 1 : 0; break;
    case '4': v = (eval(a->left) == eval(a->right))? 1 : 0; break;
    case '5': v = (eval(a->left) >= eval(a->right))? 1 : 0; break;
    case '6': v = (eval(a->left) <= eval(a->right))? 1 : 0; break;

    case 'I':
        if(eval(((flow*)a)->cond) != 0){
            if(((flow*)a)->tl)
                v = eval(((flow*)a)->tl);
            else
                v = 0.0;
        }
        else
        {
            if(((flow*)a)->el)
                v = eval(((flow*)a)->el);
            else
                v = 0.0;
        }
        break;
    case 'W':
        v = 0.0;
        if (((flow*)a)->tl){
            while (eval(((flow*)a)->cond) != 0)
                v = eval(((flow*)a)->tl);
        }
        break;
    case 'L': eval(a->left); v = eval(a->right); break;
    case 'F':  v = callbuiltin((fncall*)a); break;
    case 'C':  v = calluser((ufncall*)a); break;
    default:
        printf("Internal error: Bad node %c\n", a->nodetype);
    }
    return v;
}

void yyerror(char* s, ...){
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

static double callbuiltin(fncall* f){
    enum bifs functype = f->functype;
    double v = eval(f->left);

    switch (functype)
    {
    case B_sqrt:
        return sqrt(v);
    case B_exp:
        return exp(v);
    case B_log:
        return log(v);
    case B_print:
        printf("= %4.4g\n", v);
        return v;
    default:
        yyerror("Unknown built in function %d", functype);
        return 0.0;
    }
}

void dodef(symbol* name, symlist* syms, ast* func){
     if(name->syms) 
        symlistfree(name->syms);
    if(name->func) 
        treefree(name->func);
    name->syms = syms;
    name->func = func;
}

static double calluser(ufncall* f){
    symbol* fn = f->s;
    symlist* sl;
    ast* args = f->left;
    double* oldval;
    double* newval;
    double v;
    int nargs;
    int i;
    
    if (!fn->func){
        yyerror("Call tp undefined function", fn->name);
        return 0;
    }

    sl = fn->syms;
    for (nargs = 0; sl; sl = sl->next)
        nargs++;
    
    oldval = new double[nargs];
    newval = new double[nargs];
    if(!oldval || !newval){
        yyerror("Out of space in %s", fn->name);
        return 0.0;
    }

    for(i = 0; i < nargs; i++){
        if(!args){
            yyerror("To few arguments in call to %s", fn->name);
            delete[] oldval;
            delete[] newval;
            return 0.0;
        }
        if(args->nodetype == 'L'){
            newval[i] = eval(args->left);
            args = args->right;
        } else {
            newval[i] = eval(args);
            args = nullptr;
        }
        
    }

    sl = fn->syms;
    for(i = 0; i < nargs; i++){
        symbol* s = sl->sym;

        oldval[i] = s->value;
        s->value = newval[i];
        sl = sl->next;
    }

    delete[] newval;

    v = eval(fn->func);

    sl = fn->syms;
    for(int i = 0; i < nargs; i++){
        symbol* s = sl->sym;

        s->value = oldval[i];
        sl = sl->next;
    }

    delete[] oldval;
    return v;
}