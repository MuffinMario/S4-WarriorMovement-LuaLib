extern "C" {
#include <setjmp.h>
    typedef int StkId;  /* index to stack elements */

#ifndef LUA_NUM_TYPE
#define LUA_NUM_TYPE double
#endif


    typedef LUA_NUM_TYPE real;

    typedef enum {
        LUA_T_USERDATA = 0,  /* tag default for userdata */
        LUA_T_NUMBER = -1,  /* fixed tag for numbers */
        LUA_T_STRING = -2,  /* fixed tag for strings */
        LUA_T_ARRAY = -3,  /* tag default for tables (or arrays) */
        LUA_T_PROTO = -4,  /* fixed tag for functions */
        LUA_T_CPROTO = -5,  /* fixed tag for Cfunctions */
        LUA_T_NIL = -6,  /* last "pre-defined" tag */
        LUA_T_CLOSURE = -7,
        LUA_T_CLMARK = -8,  /* mark for closures */
        LUA_T_PMARK = -9,  /* mark for Lua prototypes */
        LUA_T_CMARK = -10, /* mark for C prototypes */
        LUA_T_LINE = -11
    } lua_Type;


    typedef union {
        lua_CFunction f;  /* LUA_T_CPROTO, LUA_T_CMARK */
        real n;  /* LUA_T_NUMBER */
        struct TaggedString* ts;  /* LUA_T_STRING, LUA_T_USERDATA */
        struct TProtoFunc* tf;  /* LUA_T_PROTO, LUA_T_PMARK */
        struct Closure* cl;  /* LUA_T_CLOSURE, LUA_T_CLMARK */
        struct Hash* a;  /* LUA_T_ARRAY */
        int i;  /* LUA_T_LINE */
    } Value;
    typedef struct TObject {
        lua_Type ttype;
        Value value;
    } TObject;

    typedef struct GCnode {
        struct GCnode* next;
        int marked;
    } GCnode;
    /*
    ** String headers for string table
    */

    typedef struct TaggedString {
        GCnode head;
        unsigned long hash;
        int constindex;  /* hint to reuse constants (= -1 if this is a userdata) */
        union {
            struct {
                TObject globalval;
                long len;  /* if this is a string, here is its length */
            } s;
            struct {
                int tag;
                void* v;  /* if this is a userdata, here is its value */
            } d;
        } u;
        char str[1];   /* \0 byte already reserved */
    } TaggedString;

    /*
    ** "jmp_buf" may be an array, so it is better to make sure it has an
    ** address (and not that it *is* an address...)
    */
    struct lua_longjmp {
        jmp_buf b;
    };


    struct Stack {
        TObject* top;
        TObject* stack;
        TObject* last;
    };

    struct C_Lua_Stack {
        StkId base;  /* when Lua calls C or C calls Lua, points to */
                     /* the first slot after the last parameter. */
        StkId lua2C; /* points to first element of "array" lua2C */
        int num;     /* size of "array" lua2C */
    };


    typedef struct stringtable {
        int size;
        int nuse;  /* number of elements (including EMPTYs) */
        TaggedString** hash;
    } stringtable;


    enum Status { LOCK, HOLD, FREE, COLLECTED };

    struct ref {
        TObject o;
        enum Status status;
    };


    struct lua_State {
        /* thread-specific state */
        struct Stack stack;  /* Lua stack */
        struct C_Lua_Stack Cstack;  /* C2lua struct */
        struct lua_longjmp* errorJmp;  /* current error recover point */
        char* Mbuffer;  /* global buffer */
        int Mbuffbase;  /* current first position of Mbuffer */
        int Mbuffsize;  /* size of Mbuffer */
        int Mbuffnext;  /* next position to fill in Mbuffer */
        struct C_Lua_Stack* Cblocks;
        int numCblocks;  /* number of nested Cblocks */
        int debug;
        lua_CHFunction callhook;
        lua_LHFunction linehook;
        /* global state */
        GCnode rootproto;  /* list of all prototypes */
        GCnode rootcl;  /* list of all closures */
        GCnode roottable;  /* list of all tables */
        GCnode rootglobal;  /* list of strings with global values */
        stringtable* string_root;  /* array of hash tables for strings and udata */
        struct IM* IMtable;  /* table for tag methods */
        int last_tag;  /* last used tag in IMtable */
        struct ref* refArray;  /* locked objects */
        int refSize;  /* size of refArray */
        unsigned long GCthreshold;
        unsigned long nblocks;  /* number of 'blocks' currently allocated */
    };
}