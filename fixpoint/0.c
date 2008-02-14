/* 0.sf */
#ifdef PROFILE
#define host host_module_0
#endif
#define MODULE module_0
#define LOAD() 

/* standard includes */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
/* extra includes */
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

/* standard definitions */
#define REGS_SIZE 5000

typedef ptrdiff_t obj;        /* pointers are this size, lower bit zero */
typedef ptrdiff_t cxoint_t;   /* same thing, used as integer */
typedef struct {              /* type descriptor */
  const char *tname;          /* name (debug) */
  void (*free)(void*);        /* deallocator */
} cxtype_t;

#define notobjptr(o)          (((char*)(o) - (char*)cxg_heap) & cxg_hmask)
#define isobjptr(o)           (!notobjptr(o))
#define notaptr(o)            ((o) & 1)
#define isaptr(o)             (!notaptr(o))

#define obj_from_obj(o)       (o)
#define obj_from_objptr(p)    ((obj)(p))
#define obj_from_size(n)      (((cxoint_t)(n) << 1) | 1)

#define objptr_from_objptr(p) (p)
#define objptr_from_obj(o)    ((obj*)(o))

#define size_from_obj(o)      ((int)((o) >> 1))

#define obj_from_case(n)      obj_from_objptr(cases+(n))
#define case_from_obj(o)      (objptr_from_obj(o)-cases)
#define obj_from_ktrap()      obj_from_size(0x5D56F806)
#define obj_from_void(v)      ((void)(v), obj_from_size(0x6F56DF77))

#define bool_from_obj(o)      (o)
#define bool_from_bool(b)     (b)
#define bool_from_size(s)     (s)

#define void_from_void(v)     (void)(v)
#define void_from_obj(o)      (void)(o)

#define rreserve(m)           if (r > cxg_regs + REGS_SIZE - 2*(m)) r = cxm_rgc(r, r+(m))
#define hpushptr(p, pt, l)    (hreserve(2, l), *--hp = (obj)(p), *--hp = (obj)(pt), (obj)(hp+1))   
#define hbsz(s)               ((s) + 1) /* 1 extra word to store block size */
#define hreserve(n, l)        ((hp < cxg_heap + (n)) ? hp = cxm_hgc(r, r+(l), hp, n) : hp)
#define hendblk(n)            (*--hp = obj_from_size(n), (obj)(hp+1))
#define hblklen(p)            size_from_obj(((obj*)(p))[-1])
#define hblkref(p, i)         (((obj*)(p))[i])

typedef obj (*cxhost_t)(obj);
typedef struct cxroot_tag {
  int globc; obj **globv;
  struct cxroot_tag *next;
} cxroot_t;

extern obj *cxg_heap;
extern obj *cxg_hp;
extern cxoint_t cxg_hmask;
extern cxroot_t *cxg_rootp;
extern obj *cxm_rgc(obj *regs, obj *regp);
extern obj *cxm_hgc(obj *regs, obj *regp, obj *hp, size_t needs);
extern obj cxg_regs[REGS_SIZE];
extern void *cxm_cknull(void *p, char *msg);
#ifndef NDEBUG
extern int cxg_rc;
#endif

/* extra definitions */
/* basic object representation */
#define isimm(o, t) (((o) & 0xff) == (((t) << 1) | 1))
int getimmu(obj o, int t) {
  assert(isimm(o, t));
  return (int)((o >> 8) & 0xffffff);
}
int getimms(obj o, int t) {
  assert(isimm(o, t));
  return (int)((((o >> 8) & 0xffffff) ^ 0x800000) - 0x800000);
}
#ifdef NDEBUG
  #define getimmu(o, t) (int)(((o) >> 8) & 0xffffff)
  #define getimms(o, t) (int)(((((o) >> 8) & 0xffffff) ^ 0x800000) - 0x800000)
#else
  extern int getimmu(obj o, int t);
  extern int getimms(obj o, int t);
#endif
#define mkimm(o, t) ((((o) & 0xffffff) << 8) | ((t) << 1) | 1)
#ifndef NDEBUG
int isnative(obj o, cxtype_t *tp) {
  return isobjptr(o) && objptr_from_obj(o)[-1] == (obj)tp; 
}
void *getnative(obj o, cxtype_t *tp) {
  assert(isnative(o, tp));
  return (void*)(*objptr_from_obj(o));
}
#endif
#ifdef NDEBUG
   static int isnative(obj o, cxtype_t *tp) 
     { return isobjptr(o) && objptr_from_obj(o)[-1] == (obj)tp;  }
   #define getnative(o, t) ((void*)(*objptr_from_obj(o)))
#else
  extern int isnative(obj o, cxtype_t *tp);
  extern void *getnative(obj o, cxtype_t *tp);
#endif
int istagged(obj o, int t) {
  if (!isobjptr(o)) return 0;
  else { obj h = objptr_from_obj(o)[-1];
    return notaptr(h) && size_from_obj(h) >= 1 
      && hblkref(o, 0) == obj_from_size(t); }
}
obj cktagged(obj o, int t) {
  assert(istagged(o, t));
  return o;
}
int taggedlen(obj o, int t) {
  assert(istagged(o, t));
  return hblklen(o) - 1;
}
obj* taggedref(obj o, int t, int i) {
  int len; assert(istagged(o, t));
  len = hblklen(o);
  assert(i >= 0 && i < len-1);  
  return &hblkref(o, i+1);
}
extern int istagged(obj o, int t);
#ifdef NDEBUG
  #define cktagged(o, t) (o)
  #define taggedlen(o, t) (hblklen(o)-1) 
  #define taggedref(o, t, i) (&hblkref(o, (i)+1))
#else
  extern obj cktagged(obj o, int t);
  extern int taggedlen(obj o, int t);
  extern obj* taggedref(obj o, int t, int i); 
#endif
/* booleans */
#define TRUE_ITAG 0
typedef int bool_t;
#define is_bool_obj(o) (!((o) & ~(obj)1))
#define is_bool_bool(b) ((void)(b), 1)
#define void_from_bool(b) (void)(b)
#define obj_from_bool(b) ((b) ? mkimm(0, TRUE_ITAG) : 0)
/* fixnums */
#define FIXNUM_ITAG 1
typedef int fixnum_t;
#define is_fixnum_obj(o) (isimm(o, FIXNUM_ITAG))
#define is_fixnum_fixnum(i) ((void)(i), 1)
#define fixnum_from_obj(o) (getimms(o, FIXNUM_ITAG))
#define fixnum_from_fixnum(i) (i)
#define void_from_fixnum(i) (void)(i)
#define obj_from_fixnum(i) mkimm(i, FIXNUM_ITAG)
#define FIXNUM_MIN -8388608
#define FIXNUM_MAX 8388607
/* flonums */
static cxtype_t cxt_flonum = { "flonum", free };
cxtype_t *FLONUM_NTAG = &cxt_flonum;
extern cxtype_t *FLONUM_NTAG;
typedef double flonum_t;
#define is_flonum_obj(o) (isnative(o, FLONUM_NTAG))
#define is_flonum_flonum(f) ((void)(f), 1)
#define flonum_from_obj(o) (*(flonum_t*)getnative(o, FLONUM_NTAG))
#define flonum_from_flonum(l, f) (f)
#define void_from_flonum(l, f) (void)(f)
#define obj_from_flonum(l, f) hpushptr(dupflonum(f), FLONUM_NTAG, l)
extern flonum_t *dupflonum(flonum_t f);
flonum_t *dupflonum(flonum_t f) {
  flonum_t *pf = cxm_cknull(malloc(sizeof(flonum_t)), "malloc(flonum)");
  *pf = f; return pf;
}
/* characters */
#define CHAR_ITAG 2
typedef int char_t;
#define is_char_obj(o) (isimm(o, CHAR_ITAG))
#define is_char_char(i) ((i), 1)
#define char_from_obj(o) (getimms(o, CHAR_ITAG))
#define char_from_char(i) (i)
#define void_from_char(i) (void)(i)
#define obj_from_char(i) mkimm(i, CHAR_ITAG)
/* strings */
static cxtype_t cxt_string = { "string", free };
cxtype_t *STRING_NTAG = &cxt_string;
extern cxtype_t *STRING_NTAG;
#define isstring(o) (isnative(o, STRING_NTAG))
#define stringdata(o) ((int*)getnative(o, STRING_NTAG))
#define stringlen(o) (*stringdata(o))
#define stringchars(o) ((char*)(stringdata(o)+1))
#define hpushstr(l, s) hpushptr(s, STRING_NTAG, l)
char* stringref(obj o, int i) {
  int *d = stringdata(o);
  assert(i >= 0 && i < *d);  
  return ((char*)(d+1))+i;
}
#ifdef NDEBUG
  #define stringref(o, i) (stringchars(o)+(i))
#else
  extern char* stringref(obj o, int i);
#endif
extern int *newstring(char *s);
int *newstring(char *s) {
  int l, *d; assert(s); l = strlen(s); 
  d = cxm_cknull(malloc(sizeof(int)+l+1), "malloc(string)");
  *d = l; strcpy((char*)(d+1), s); return d;
}
extern int *allocstring(int n, int c);
int *allocstring(int n, int c) {
  int *d; char *s; assert(n+1 > 0); 
  d = cxm_cknull(malloc(sizeof(int)+n+1), "malloc(string)");
  *d = n; s = (char*)(d+1); memset(s, c, n); s[n] = 0;
  return d;
}
extern int *substring(int *d, int from, int to);
int *substring(int *d0, int from, int to) {
  int n = to-from, *d1; char *s0, *s1; assert(d0);
  assert(0 <= from && from <= to && to <= *d0); 
  d1 = cxm_cknull(malloc(sizeof(int)+n+1), "malloc(string)");
  *d1 = n; s0 = (char*)(d0+1); s1 = (char*)(d1+1); 
  memcpy(s1, s0+from, n); s1[n] = 0;
  return d1;
}
extern int *stringcat(int *d0, int *d1);
int *stringcat(int *d0, int *d1) {
  int l0 = *d0, l1 = *d1, n = l0+l1; char *s0, *s1, *s;
  int *d = cxm_cknull(malloc(sizeof(int)+n+1), "malloc(string)");
  *d = n; s = (char*)(d+1); s0 = (char*)(d0+1); s1 = (char*)(d1+1);
  memcpy(s, s0, l0); memcpy(s+l0, s1, l1); s[n] = 0;
  return d;
}
extern int *dupstring(int *d);
int *dupstring(int *d0) {
  int n = *d0, *d1 = cxm_cknull(malloc(sizeof(int)+n+1), "malloc(string)");
  memcpy(d1, d0, sizeof(int)+n+1);
  return d1;
}
extern void stringfill(int *d, int c);
void stringfill(int *d, int c) {
  int l = *d, i; char *s = (char*)(d+1);
  for (i = 0; i < l; ++i) s[i] = c;
}
extern int strcmp_ci(char *s1, char*s2);
int strcmp_ci(char *s1, char *s2) {
  int c1, c2, d;
  do { c1 = *s1++; c2 = *s2++; d = (unsigned)tolower(c1) - (unsigned)tolower(c2); }
  while (!d && c1 && c2);
  return d;
}
/* vectors */
#define VECTOR_BTAG 1
#define isvector(o) istagged(o, VECTOR_BTAG)
#define vectorref(v, i) *taggedref(v, VECTOR_BTAG, i)
#define vectorlen(v) taggedlen(v, VECTOR_BTAG)
/* boxes */
#define BOX_BTAG 2
#define isbox(o) istagged(o, BOX_BTAG)
#define boxref(o) *taggedref(o, BOX_BTAG, 0)
/* null */
#define NULL_ITAG 2
#define mknull() mkimm(0, NULL_ITAG)
#define isnull(o) ((o) == mkimm(0, NULL_ITAG))
/* pairs and lists */
#define PAIR_BTAG 3
#define ispair(o) istagged(o, PAIR_BTAG)
#define car(o) *taggedref(o, PAIR_BTAG, 0)
#define cdr(o) *taggedref(o, PAIR_BTAG, 1)
/* symbols */
#define SYMBOL_ITAG 3
#define issymbol(o) (isimm(o, SYMBOL_ITAG))
#define mksymbol(i) mkimm(i, SYMBOL_ITAG)
#define getsymbol(o) getimmu(o, SYMBOL_ITAG)
static struct { char **a; char ***v; size_t sz; size_t u; size_t maxu; } symt;
static unsigned long hashs(char *s) {
  unsigned long i = 0, l = (unsigned long)strlen(s), h = l;
  while (i < l) h = (h << 4) ^ (h >> 28) ^ s[i++];
  return h ^ (h  >> 10) ^ (h >> 20);
}
extern char *symbolname(int sym);
char *symbolname(int sym) {
  assert(sym >= 0); assert(sym < (int)symt.u);
  return symt.a[sym];
}
extern int internsym(char *name);
int internsym(char *name) {
  size_t i, j; /* based on a code (C) 1998, 1999 by James Clark. */
  if (symt.sz == 0) { /* init */
    symt.a = cxm_cknull(calloc(64, sizeof(char*)), "symtab[0]");
    symt.v = cxm_cknull(calloc(64, sizeof(char**)), "symtab[1]");
    symt.sz = 64, symt.maxu = 64 / 2;
    i = hashs(name) & (symt.sz-1);
  } else {
    unsigned long h = hashs(name);
    for (i = h & (symt.sz-1); symt.v[i]; i = (i-1) & (symt.sz-1))
      if (strcmp(name, *symt.v[i]) == 0) return (int)(symt.v[i] - symt.a);
    if (symt.u == symt.maxu) { /* rehash */
      size_t nsz = symt.sz * 2;
      char **na = cxm_cknull(calloc(nsz, sizeof(char*)), "symtab[2]");
      char ***nv = cxm_cknull(calloc(nsz, sizeof(char**)), "symtab[3]");
      for (i = 0; i < symt.sz; i++)
        if (symt.v[i]) {
          for (j = hashs(*symt.v[i]) & (nsz-1); nv[j]; j = (j-1) & (nsz-1)) ;
          nv[j] = symt.v[i] - symt.a + na;
        }
      free(symt.v); symt.v = nv; symt.sz = nsz; symt.maxu = nsz / 2;
      memcpy(na, symt.a, symt.u * sizeof(char*)); free(symt.a); symt.a = na; 
      for (i = h & (symt.sz-1); symt.v[i]; i = (i-1) & (symt.sz-1)) ;
    }
  }
  *(symt.v[i] = symt.a + symt.u) = 
    strcpy(cxm_cknull(malloc(strlen(name)+1), "symtab[4]"), name);
  return (int)((symt.u)++);
}
/* eof */
#define EOF_ITAG 127
#define mkeof() mkimm(-1, EOF_ITAG)
#define iseof(o) ((o) == mkimm(-1, EOF_ITAG))
/* input ports */
static void ipclose(void *vp) {
  /* FILE *fp = vp; assert(fp); 
   * cannot fclose(fp) here because of FILE reuse! */ 
}
static cxtype_t cxt_iport = { "iport", ipclose };
cxtype_t *IPORT_NTAG = &cxt_iport;
extern cxtype_t *IPORT_NTAG;
#define isiport(o) (isnative(o, IPORT_NTAG))
#define iportdata(o) ((FILE*)getnative(o, IPORT_NTAG))
#define mkiport(l, fp) hpushptr(fp, IPORT_NTAG, l)
/* output ports */
static void opclose(void *vp) {
  /* FILE *fp = vp; assert(fp); 
   * cannot fclose(fp) here because of FILE reuse! */ 
}
static cxtype_t cxt_oport = { "oport", opclose };
cxtype_t *OPORT_NTAG = &cxt_oport;
extern cxtype_t *OPORT_NTAG;
#define isoport(o) (isnative(o, OPORT_NTAG))
#define oportdata(o) ((FILE*)getnative(o, OPORT_NTAG))
#define mkoport(l, fp) hpushptr(fp, OPORT_NTAG, l)
extern int iseqv(obj x, obj y);
int iseqv(obj x, obj y) {
  obj h; if (x == y) return 1;
  if (!x || !y || notaptr(x) || notaptr(y) || notobjptr(x) || notobjptr(y)) return 0;
  if ((h = objptr_from_obj(x)[-1]) != objptr_from_obj(y)[-1]) return 0;
  if (h == (obj)FLONUM_NTAG) return *(flonum_t*)objptr_from_obj(x)[0] == *(flonum_t*)objptr_from_obj(y)[0]; 
  return 0;
}
extern obj ismemv(obj x, obj l);
obj ismemv(obj x, obj l) {
  if (!x || notaptr(x) || notobjptr(x)) {
    for (; l != mknull(); l = cdr(l)) 
      { if (car(l) == x) return l; }
  } else if (is_flonum_obj(x)) {
    flonum_t fx = flonum_from_obj(x); 
    for (; l != mknull(); l = cdr(l)) 
      { obj y = car(l); if (is_flonum_obj(y) && fx == flonum_from_obj(y)) return l; }
  } else { /* for others, memv == memq */
    for (; l != mknull(); l = cdr(l)) 
      { if (car(l) == x) return l; }
  } return 0;
}
extern obj isassv(obj x, obj l);
obj isassv(obj x, obj l) {
  if (!x || notaptr(x) || notobjptr(x)) {
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l); if (car(p) == x) return p; }
  } else if (is_flonum_obj(x)) {
    flonum_t fx = flonum_from_obj(x); 
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l), y = car(p); if (is_flonum_obj(y) && fx == flonum_from_obj(y)) return p; }
  } else { /* for others, assv == assq */
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l); if (car(p) == x) return p; }
  } return 0;
}
extern int isequal(obj x, obj y);
int isequal(obj x, obj y) {
  obj h; int i, n; loop: if (x == y) return 1;
  if (!x || !y || notaptr(x) || notaptr(y) || notobjptr(x) || notobjptr(y)) return 0;
  if ((h = objptr_from_obj(x)[-1]) != objptr_from_obj(y)[-1]) return 0;
  if (h == (obj)FLONUM_NTAG) return *(flonum_t*)objptr_from_obj(x)[0] == *(flonum_t*)objptr_from_obj(y)[0]; 
  if (h == (obj)STRING_NTAG) return strcmp(stringchars(x), stringchars(y)) == 0; 
  if (isaptr(h) || !(n = size_from_obj(h)) || hblkref(x, 0) != hblkref(y, 0)) return 0;
  for (i = 1; i < n-1; ++i) if (!isequal(hblkref(x, i), hblkref(y, i))) return 0;
  if (i == n-1) { x = hblkref(x, i); y = hblkref(y, i); goto loop; } else return 0; 
}
extern obj ismember(obj x, obj l);
obj ismember(obj x, obj l) {
  if (!x || notaptr(x) || notobjptr(x)) {
    for (; l != mknull(); l = cdr(l)) 
      { if (car(l) == x) return l; }
  } else if (is_flonum_obj(x)) {
    flonum_t fx = flonum_from_obj(x); 
    for (; l != mknull(); l = cdr(l)) 
      { obj y = car(l); if (is_flonum_obj(y) && fx == flonum_from_obj(y)) return l; }
  } else if (isstring(x)) {
    char *xs = stringchars(x);
    for (; l != mknull(); l = cdr(l)) 
      { obj y = car(l); if (isstring(y) && 0 == strcmp(xs, stringchars(y))) return l; }
  } else {
    for (; l != mknull(); l = cdr(l)) 
      { if (isequal(car(l), x)) return l; }
  } return 0;
}
extern obj isassoc(obj x, obj l);
obj isassoc(obj x, obj l) {
  if (!x || notaptr(x) || notobjptr(x)) {
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l); if (car(p) == x) return p; }
  } else if (is_flonum_obj(x)) {
    flonum_t fx = flonum_from_obj(x); 
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l), y = car(p); if (is_flonum_obj(y) && fx == flonum_from_obj(y)) return p; }
  } else if (isstring(x)) {
    char *xs = stringchars(x);
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l), y = car(p); if (isstring(y) && 0 == strcmp(xs, stringchars(y))) return p; }
  } else {
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l); if (isequal(car(p), x)) return p; }
  } return 0;
}

/* cx globals */
obj cx__2Acurrent_2Derror_2Dport_2A; /* *current-error-port* */
obj cx__2Acurrent_2Dinput_2Dport_2A; /* *current-input-port* */
obj cx__2Acurrent_2Doutput_2Dport_2A; /* *current-output-port* */
obj cx_fixnum_2D_3Estring; /* fixnum->string */
obj cx_flonum_2D_3Estring; /* flonum->string */
obj cx_fprintf_2A; /* fprintf* */
obj cx_fxexpt; /* fxexpt */
obj cx_list_3F; /* list? */
obj cx_make_2Dpromise; /* make-promise */
obj cx_reset; /* reset */
obj cx_set_2Dreset_2Dhandler_21; /* set-reset-handler! */
obj cx_string_2D_3Efixnum; /* string->fixnum */
obj cx_string_2D_3Eflonum; /* string->flonum */
obj cx_vector_2Dfill_21; /* vector-fill! */
obj cx_with_2Dinput_2Dfrom_2Dfile; /* with-input-from-file */
obj cx_with_2Doutput_2Dto_2Dfile; /* with-output-to-file */
obj cx_write_2F3; /* write/3 */
static obj cx__23103; /* constant #103 */
static obj cx__23129; /* constant #129 */
static obj cx__23131; /* constant #131 */
static obj cx__23134; /* constant #134 */
static obj cx__23146; /* constant #146 */
static obj cx__23159; /* constant #159 */
static obj cx__23160; /* constant #160 */
static obj cx__23166; /* constant #166 */
static obj cx__23167; /* constant #167 */
static obj cx__23169; /* constant #169 */
static obj cx__23176; /* constant #176 */
static obj cx__23178; /* constant #178 */
static obj cx__23180; /* constant #180 */
static obj cx__2384; /* constant #84 */

/* helper functions */
/* vector-fill! */
static obj cxs_vector_2Dfill_21(obj v14_v, obj v13_x)
{ 
  { /* letrec */
    obj v18_i;
  { /* let */
    obj v299_tmp = obj_from_fixnum(0);
    /* tail call */
    v18_i = (v299_tmp);
    goto s_loop;
  }
  s_loop:
  if ((fixnum_from_obj(v18_i) == (vectorlen((v14_v))))) {
    return ((0) ? obj_from_bool(0) : obj_from_void(0));
  } else {
    (void) obj_from_void(vectorref((v14_v), fixnum_from_obj(v18_i)) = (v13_x));
  { /* let */
    obj v298_tmp = obj_from_fixnum(fixnum_from_obj(v18_i) + (1));
    /* tail call */
    v18_i = (v298_tmp);
    goto s_loop;
  }
  }
  }
}

/* gc roots */
static obj *globv[] = {
  &cx__2Acurrent_2Derror_2Dport_2A,
  &cx__2Acurrent_2Dinput_2Dport_2A,
  &cx__2Acurrent_2Doutput_2Dport_2A,
  &cx__23103,
  &cx__23129,
  &cx__23131,
  &cx__23134,
  &cx__23146,
  &cx__23159,
  &cx__23160,
  &cx__23166,
  &cx__23167,
  &cx__23169,
  &cx__23176,
  &cx__23178,
  &cx__23180,
  &cx__2384,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[31] = {
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,
};

/* host procedure */
#define MAX_LIVEREGS 14
static obj host(obj pc)
{
  register obj *r = cxg_regs;
  register obj *hp = cxg_hp;
#ifndef NDEBUG
  register int rc = cxg_rc;
#endif
  jump: 
  switch (case_from_obj(pc)) {

case 0: /* load module */
    cx__2384 = (hpushstr(0, newstring("#&")));
    cx__23103 = (hpushstr(0, newstring("#(")));
    { static char s[] = { 35, 92, 0 };
    cx__23129 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 35, 92, 115, 112, 97, 99, 101, 0 };
    cx__23131 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 35, 92, 110, 101, 119, 108, 105, 110, 101, 0 };
    cx__23134 = (hpushstr(0, newstring(s))); }
    cx__23146 = (hpushstr(0, newstring(" . ")));
    cx__23159 = (hpushstr(0, newstring("#<unknown>")));
    cx__23160 = (hpushstr(0, newstring("#<procedure>")));
    cx__23166 = (hpushstr(0, newstring("#f")));
    cx__23167 = (hpushstr(0, newstring("#t")));
    cx__23169 = (hpushstr(0, newstring("()")));
    cx__23176 = (hpushstr(0, newstring("#<oport>")));
    cx__23178 = (hpushstr(0, newstring("#<iport>")));
    cx__23180 = (hpushstr(0, newstring("#<eof>")));
    { static obj c[] = { obj_from_case(1) }; cx_fxexpt = (obj)c; }
    { static obj c[] = { obj_from_case(4) }; cx_vector_2Dfill_21 = (obj)c; }
    { static obj c[] = { obj_from_case(5) }; cx_list_3F = (obj)c; }
    { static obj c[] = { obj_from_case(6) }; cx_fixnum_2D_3Estring = (obj)c; }
    { static obj c[] = { obj_from_case(7) }; cx_flonum_2D_3Estring = (obj)c; }
    { static obj c[] = { obj_from_case(8) }; cx_string_2D_3Efixnum = (obj)c; }
    { static obj c[] = { obj_from_case(9) }; cx_string_2D_3Eflonum = (obj)c; }
    { static obj c[] = { obj_from_case(10) }; cx_make_2Dpromise = (obj)c; }
    cx__2Acurrent_2Dinput_2Dport_2A = (mkiport(0, stdin));
    { static obj c[] = { obj_from_case(13) }; cx_with_2Dinput_2Dfrom_2Dfile = (obj)c; }
    cx__2Acurrent_2Doutput_2Dport_2A = (mkoport(0, stdout));
    cx__2Acurrent_2Derror_2Dport_2A = (mkoport(0, stderr));
    { static obj c[] = { obj_from_case(15) }; cx_with_2Doutput_2Dto_2Dfile = (obj)c; }
    { static obj c[] = { obj_from_case(17) }; cx_write_2F3 = (obj)c; }
    { static obj c[] = { obj_from_case(25) }; cx_fprintf_2A = (obj)c; }
    { static obj c[] = { obj_from_case(29) }; cx_reset = (obj)c; }
    { static obj c[] = { obj_from_case(30) }; cx_set_2Dreset_2Dhandler_21 = (obj)c; }
    r[0] = obj_from_void(0);
    r[1+0] = r[0];
    pc = 0; /* exit from module init */
    r[1+1] = r[0];  
    r += 1; /* shift reg wnd */
    assert(rc = 2);
    goto jump;

case 1: /* fxexpt k x y */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k x y */
    r[3+0] = r[0];  
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_ex;

s_ex: /* k y x */
    { const fixnum_t v730_y = fixnum_from_obj(r[1]);
  if (((v730_y) == 0)) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_fixnum(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((fixnum_from_obj(r[2]) == 0)) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_fixnum(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((v730_y) < (0))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_fixnum(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((v730_y) == (1))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((v730_y) % 2 != 0)) {
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(2);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = obj_from_fixnum((v730_y) - (1));
    /* r[2] */    
    goto s_ex;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(3);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = obj_from_fixnum((v730_y) / (2));
    /* r[2] */    
    goto s_ex;
  }
  }
  }
  }
  } } 

case 2: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k x */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_fixnum(fixnum_from_obj(r[3]) * fixnum_from_obj(r[1]));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 3: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { const fixnum_t v729_r = fixnum_from_obj(r[1]);
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_fixnum((v729_r) * (v729_r));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump; } 

case 4: /* vector-fill! k v x */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k v x */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_vector_2Dfill_21((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 5: /* list? k o */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k o */
    { /* list? */
    obj l = (r[1]), s = l;
    for (;;) {
    if (isnull(l)) { r[2] = obj_from_bool(1); break; }
    else if (!ispair(l)) { r[2] = obj_from_bool(0); break; }
    else if ((l = cdr(l)) == s) { r[2] = obj_from_bool(0); break; }
    else if (isnull(l)) { r[2] = obj_from_bool(1); break; }
    else if (!ispair(l)) { r[2] = obj_from_bool(0); break; }
    else if ((l = cdr(l)) == s) { r[2] = obj_from_bool(0); break; }
    else s = cdr(s); } }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 6: /* fixnum->string k n r */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k n r */
    { /* fixnum->string */
    char buf[35], *s = buf + sizeof(buf) - 1; 
    int neg = 0;
    int num = fixnum_from_obj(r[1]);
    int radix = fixnum_from_obj(r[2]);
    if (num < 0) { neg = 1; num = -num; }
    *s = 0;
    do { int d = num % radix; *--s = d < 10 ? d + '0' : d - 10 + 'a'; }
    while (num /= radix);
    if (neg) *--s = '-';
    r[3] = (hpushstr(3, newstring(s))); }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 7: /* flonum->string k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k x */
    { /* flonum->string */
    char buf[30], *s;
    sprintf(buf, "%.17g", flonum_from_obj(r[1]));
    for (s = buf; *s != 0; s++) if (*s == 'e' || *s == '.') break;
    if (*s == 0) {  *s++ = '.'; *s++ = '0'; *s = 0; }
    r[2] = (hpushstr(2, newstring(buf))); }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 8: /* string->fixnum k s r */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k s r */
    { /* string->fixnum */
    char *e, *s = stringchars((r[1]));
    int radix = fixnum_from_obj(r[2]);
    long l;
    if (s[0] == '#' && (s[1] == 'b' || s[1] == 'B')) s += 2, radix = 2;
    else if (s[0] == '#' && (s[1] == 'o' || s[1] == 'O')) s += 2, radix = 8;
    else if (s[0] == '#' && (s[1] == 'd' || s[1] == 'D')) s += 2, radix = 10;
    else if (s[0] == '#' && (s[1] == 'x' || s[1] == 'X')) s += 2, radix = 16;
    l = (errno = 0, strtol(s, &e, radix));
    if (errno || l < FIXNUM_MIN || l > FIXNUM_MAX || e == s || *e) r[3] = obj_from_bool(0);
    else r[3] = obj_from_fixnum((int)l); }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 9: /* string->flonum k s */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k s */
    { /* string->flonum */
    char *e, *s = stringchars((r[1]));
    double d = (errno = 0, strtod(s, &e));
    if (errno || e == s || *e) r[2] = obj_from_bool(0);
    else r[2] = obj_from_flonum(2, d); }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 10: /* make-promise k proc */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k proc */
    hreserve(hbsz(1), 2); /* 2 live regs */
    *--hp = obj_from_bool(0);
    r[2] = (hendblk(1));
    hreserve(hbsz(1), 3); /* 3 live regs */
    *--hp = obj_from_bool(0);
    r[3] = (hendblk(1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(11);
    r[2] = (hendblk(3+1));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 11: /* clo k */
    assert(rc == 2);
    { obj* p = objptr_from_obj(r[0]);
    r[1+1] = p[1];
    r[1+2] = p[2];
    r[1+3] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* k proc result result-ready? */
  if (bool_from_obj(objptr_from_obj(r[3])[0])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (objptr_from_obj(r[2])[0]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(12);
    r[4] = (hendblk(3+1));
    r[5+0] = r[1];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  }

case 12: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k result result-ready? */
  if (bool_from_obj(objptr_from_obj(r[4])[0])) {
    r[5] = (objptr_from_obj(r[3])[0]);
  } else {
    (void)(objptr_from_obj(r[4])[0] = obj_from_bool(1));
    (void)(objptr_from_obj(r[3])[0] = (r[1]));
    r[5] = (objptr_from_obj(r[3])[0]);
  }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 13: /* with-input-from-file k fn thunk */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k fn thunk */
    r[3] = (cx__2Acurrent_2Dinput_2Dport_2A);
    r[4] = (mkiport(4, cxm_cknull(fopen(stringchars((r[1])), "r"), "fopen")));
    cx__2Acurrent_2Dinput_2Dport_2A = r[4];  
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(14);
    r[5] = (hendblk(3+1));
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 14: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k p0 p1 */
    (void)(fclose(iportdata((r[4]))));
    cx__2Acurrent_2Dinput_2Dport_2A = r[3];  
    r[5] = r[1];  
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 15: /* with-output-to-file k fn thunk */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k fn thunk */
    r[3] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[4] = (mkoport(4, cxm_cknull(fopen(stringchars((r[1])), "w"), "fopen")));
    cx__2Acurrent_2Doutput_2Dport_2A = r[4];  
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(16);
    r[5] = (hendblk(3+1));
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 16: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k p0 p1 */
    (void)(fclose(oportdata((r[4]))));
    cx__2Acurrent_2Doutput_2Dport_2A = r[3];  
    r[5] = r[1];  
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 17: /* write/3 k x d? p */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_write_2F3: /* k x d? p */
  if ((iseof((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(fputs(stringchars((cx__23180)), oportdata((r[3]))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isiport((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(fputs(stringchars((cx__23178)), oportdata((r[3]))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isoport((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(fputs(stringchars((cx__23176)), oportdata((r[3]))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((issymbol((r[1])))) {
    r[4] = (hpushstr(4, newstring(symbolname(getsymbol((r[1]))))));
    r[4] = obj_from_void(fputs(stringchars((r[4])), oportdata((r[3]))));
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[1])))) {
    (void)(fputc((40), oportdata((r[3]))));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(20);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    goto s_loop;
  } else {
  if ((is_fixnum_obj(r[1]))) {
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_void(fprintf(oportdata((r[3])), "%d", fixnum_from_obj(r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((is_flonum_obj(r[1]))) {
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_void(fprintf(oportdata((r[3])), "%.15g", flonum_from_obj(r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(fputs(stringchars((cx__23169)), oportdata((r[3]))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((is_bool_obj(r[1]))) {
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_void(fputs(stringchars((bool_from_obj(r[1]) ? (cx__23167) : (cx__23166))), oportdata((r[3]))));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((is_char_obj(r[1]))) {
    { const char_t v728_x = char_from_obj(r[1]);
  if (bool_from_obj(r[2])) {
    r[4] = obj_from_void(fputc((v728_x), oportdata((r[3]))));
  } else {
  if (((v728_x) == (10))) {
    r[4] = obj_from_void(fputs(stringchars((cx__23134)), oportdata((r[3]))));
  } else {
  if (((v728_x) == (32))) {
    r[4] = obj_from_void(fputs(stringchars((cx__23131)), oportdata((r[3]))));
  } else {
    (void)(fputs(stringchars((cx__23129)), oportdata((r[3]))));
    r[4] = obj_from_void(fputc((v728_x), oportdata((r[3]))));
  }
  }
  }
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump; } 
  } else {
  if ((isstring((r[1])))) {
  if (bool_from_obj(r[2])) {
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_void(fputs(stringchars((r[1])), oportdata((r[3]))));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    (void)(fputc((34), oportdata((r[3]))));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(21);
    r[4] = (hendblk(2+1));
    r[5+0] = r[4];  
    r[5+1] = obj_from_fixnum(0);
    r[5+2] = r[3];  
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v584;
  }
  } else {
  if ((isvector((r[1])))) {
    (void)(fputs(stringchars((cx__23103)), oportdata((r[3]))));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(22);
    r[4] = (hendblk(2+1));
  if ((!((vectorlen((r[1]))) == (0)))) {
    r[5+0] = r[4];  
    r[5+1] = obj_from_fixnum(0);
    r[5+2] = r[2];  
    r[5+3] = r[3];  
    r[5+4] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v561;
  } else {
    r[5+0] = obj_from_ktrap();
    r[5+1] = obj_from_void(0);
    r[5+2] = r[0];  
    r[5+3] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v573;
  }
  } else {
  if ((isbox((r[1])))) {
    (void)(fputs(stringchars((cx__2384)), oportdata((r[3]))));
    /* r[0] */    
    r[1] = (boxref((r[1])));
    /* r[2] */    
    /* r[3] */    
    goto gs_write_2F3;
  } else {
    { /* procedure? */
    obj o = (r[1]), h;
    if (!isobjptr(o)) r[4] = obj_from_bool(0);
    h = objptr_from_obj(o)[-1];
    r[4] = obj_from_bool(notaptr(h) && size_from_obj(h) >= 1 && isaptr(hblkref(o, 0))); }
  if (bool_from_obj(r[4])) {
    r[4] = obj_from_void(fputs(stringchars((cx__23160)), oportdata((r[3]))));
  } else {
    r[4] = obj_from_void(fputs(stringchars((cx__23159)), oportdata((r[3]))));
  }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }

case 18: /* clo k x */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_loop: /* k x d? p */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(18);
    r[4] = (hendblk(2+1));
    hreserve(hbsz(5+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(19);
    r[4] = (hendblk(5+1));
    r[0] = r[4];  
    r[1] = (car((r[1])));
    /* r[2] */    
    /* r[3] */    
    goto gs_write_2F3;

case 19: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek  d? loop k p x */
    r[7] = (cdr((r[6])));
    r[7] = obj_from_bool(ispair((r[7])));
  if (bool_from_obj(r[7])) {
    (void)(fputc((32), oportdata((r[5]))));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[6])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[7] = (cdr((r[6])));
    r[7] = obj_from_bool(isnull((r[7])));
  if (bool_from_obj(r[7])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    (void)(fputs(stringchars((cx__23146)), oportdata((r[5]))));
    r[0] = r[4];  
    r[1] = (cdr((r[6])));
    /* r[2] */    
    r[3] = r[5];  
    goto gs_write_2F3;
  }
  }

case 20: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k p */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(fputc((41), oportdata((r[3]))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v584: /* k i p x */
    { const fixnum_t v726_i = fixnum_from_obj(r[1]);
  if (((v726_i) == (stringlen((r[3]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { const char_t v727_c = (*stringref((r[3]), (v726_i)));
    (void)((((v727_c) == (34)) || ((v727_c) == (92))) ? (void)(fputc((92), oportdata((r[2])))) : (void)(0));
    (void)(fputc((v727_c), oportdata((r[2])))); } 
    /* r[0] */    
    r[1] = obj_from_fixnum((v726_i) + (1));
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v584;
  } } 

case 21: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k p */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(fputc((34), oportdata((r[3]))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 22: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v573: /* ek  k p */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(fputc((41), oportdata((r[3]))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 23: /* clo k i */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_loop_v561: /* k i d? p x */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(23);
    r[5] = (hendblk(3+1));
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = obj_from_case(24);
    r[5] = (hendblk(5+1));
    r[0] = r[5];  
    r[1] = (vectorref((r[4]), fixnum_from_obj(r[1])));
    /* r[2] */    
    /* r[3] */    
    goto gs_write_2F3;

case 24: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop k p i x */
    { const fixnum_t v725_i = fixnum_from_obj(r[5]);
  if ((!((v725_i) == ((vectorlen((r[6]))) - (1))))) {
    (void)(fputc((32), oportdata((r[4]))));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = obj_from_fixnum((v725_i) + (1));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } } 

case 25: /* fprintf* k port fstr olst */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k port fstr olst */
    { /* string->list */
    int c = (stringlen((r[2])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    s = stringchars((r[2])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[4] = (l); }
    r[5+0] = r[0];  
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v532;

case 26: /* clo k flst olst */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v532: /* k flst olst port */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(26);
    r[4] = (hendblk(1+1));
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (car((r[1])));
    r[5] = obj_from_bool(char_from_obj(r[5]) == (126));
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[1])));
    r[5] = obj_from_bool(ispair((r[5])));
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[1])));
    r[5] = (car((r[5])));
  if ((char_from_obj(r[5]) == (97))) {
  if ((isnull((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(27);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = (car((r[2])));
    r[2] = obj_from_bool(1);
    /* r[3] */    
    goto gs_write_2F3;
  }
  } else {
  if ((char_from_obj(r[5]) == (115))) {
  if ((isnull((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(28);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = (car((r[2])));
    r[2] = obj_from_bool(0);
    /* r[3] */    
    goto gs_write_2F3;
  }
  } else {
  if ((char_from_obj(r[5]) == (37))) {
    (void)(fputc('\n', oportdata((r[3]))));
    r[6] = (cdr((r[1])));
    r[6] = (cdr((r[6])));
    /* r[0] */    
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v532;
  } else {
  if ((char_from_obj(r[5]) == (126))) {
    (void)(fputc((126), oportdata((r[3]))));
    r[6] = (cdr((r[1])));
    r[6] = (cdr((r[6])));
    /* r[0] */    
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v532;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[5] = (car((r[1])));
    (void)(fputc(char_from_obj(r[5]), oportdata((r[3]))));
    r[5] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v532;
  }
  }

case 27: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop olst flst k */
    r[6] = (cdr((r[4])));
    r[6] = (cdr((r[6])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    r[3] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 28: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop olst flst k */
    r[6] = (cdr((r[4])));
    r[6] = (cdr((r[6])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    r[3] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 29: /* reset k */
    assert(rc == 2);
    r += 1; /* shift reg. wnd */
    /* k */
    r[1+0] = r[0];  
    pc = objptr_from_obj(r[1+0])[0];
    r[1+1] = obj_from_ktrap();
    r[1+2] = obj_from_void(exit(1));
    r += 1; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 30: /* set-reset-handler! k fn */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k fn */
    cx_reset = r[1];  
    r[2] = obj_from_void(0);
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

default: /* inter-host call */
    cxg_hp = hp;
    cxm_rgc(r, r + MAX_LIVEREGS);
#ifndef NDEBUG
    cxg_rc = rc;
#endif
    return pc;
  }
}

/* module load */
void MODULE(void)
{
  obj pc;
  if (!root.next) {
    root.next = cxg_rootp;
    cxg_rootp = &root;
    LOAD();
    pc = obj_from_case(0);
    assert((cxg_rc = 0, 1));
    while (pc) pc = (*(cxhost_t*)pc)(pc); 
    assert(cxg_rc == 2);
  }
}
