/* 4.sf */
#define MODULE module_4
#define LOAD() module_3(); module_0(); 
extern void module_0(void); /* 0.sf */
extern void module_3(void); /* 3.sf */

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
#ifdef NDEBUG
  #define getimmu(o, t) (int)(((o) >> 8) & 0xffffff)
  #define getimms(o, t) (int)(((((o) >> 8) & 0xffffff) ^ 0x800000) - 0x800000)
#else
  extern int getimmu(obj o, int t);
  extern int getimms(obj o, int t);
#endif
#define mkimm(o, t) ((((o) & 0xffffff) << 8) | ((t) << 1) | 1)
#ifdef NDEBUG
   static int isnative(obj o, cxtype_t *tp) 
     { return isobjptr(o) && objptr_from_obj(o)[-1] == (obj)tp;  }
   #define getnative(o, t) ((void*)(*objptr_from_obj(o)))
#else
  extern int isnative(obj o, cxtype_t *tp);
  extern void *getnative(obj o, cxtype_t *tp);
#endif
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
extern cxtype_t *FLONUM_NTAG;
typedef double flonum_t;
#define is_flonum_obj(o) (isnative(o, FLONUM_NTAG))
#define is_flonum_flonum(f) ((void)(f), 1)
#define flonum_from_obj(o) (*(flonum_t*)getnative(o, FLONUM_NTAG))
#define flonum_from_flonum(l, f) (f)
#define void_from_flonum(l, f) (void)(f)
#define obj_from_flonum(l, f) hpushptr(dupflonum(f), FLONUM_NTAG, l)
extern flonum_t *dupflonum(flonum_t f);
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
extern cxtype_t *STRING_NTAG;
#define isstring(o) (isnative(o, STRING_NTAG))
#define stringdata(o) ((int*)getnative(o, STRING_NTAG))
#define stringlen(o) (*stringdata(o))
#define stringchars(o) ((char*)(stringdata(o)+1))
#define hpushstr(l, s) hpushptr(s, STRING_NTAG, l)
#ifdef NDEBUG
  #define stringref(o, i) (stringchars(o)+(i))
#else
  extern char* stringref(obj o, int i);
#endif
extern int *newstring(char *s);
extern int *allocstring(int n, int c);
extern int *substring(int *d, int from, int to);
extern int *stringcat(int *d0, int *d1);
extern int *dupstring(int *d);
extern void stringfill(int *d, int c);
extern int strcmp_ci(char *s1, char*s2);
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
extern char *symbolname(int sym);
extern int internsym(char *name);
/* eof */
#define EOF_ITAG 127
#define mkeof() mkimm(-1, EOF_ITAG)
#define iseof(o) ((o) == mkimm(-1, EOF_ITAG))
/* input ports */
extern cxtype_t *IPORT_NTAG;
#define isiport(o) (isnative(o, IPORT_NTAG))
#define iportdata(o) ((FILE*)getnative(o, IPORT_NTAG))
#define mkiport(l, fp) hpushptr(fp, IPORT_NTAG, l)
/* output ports */
extern cxtype_t *OPORT_NTAG;
#define isoport(o) (isnative(o, OPORT_NTAG))
#define oportdata(o) ((FILE*)getnative(o, OPORT_NTAG))
#define mkoport(l, fp) hpushptr(fp, OPORT_NTAG, l)

/* cx globals */
extern obj cx__2Acurrent_2Derror_2Dport_2A; /* *current-error-port* */
extern obj cx_c_2Derror_2A; /* c-error* */
extern obj cx_c_2Dmangle; /* c-mangle */
extern obj cx_equal_3F; /* equal? */
extern obj cx_exp_2Dctype; /* exp-ctype */
extern obj cx_exp_2Dvinfo; /* exp-vinfo */
extern obj cx_fprintf_2A; /* fprintf* */
extern obj cx_labelapp_2Dexp_3F; /* labelapp-exp? */
extern obj cx_member; /* member */
extern obj cx_prim_2Dcexp_3F; /* prim-cexp? */
extern obj cx_reduce_2Dleft; /* reduce-left */
extern obj cx_reduce_2Dright; /* reduce-right */
extern obj cx_reduce_2Dright_2Fright_2Dseed; /* reduce-right/right-seed */
extern obj cx_reset; /* reset */
extern obj cx_symbol_2A; /* symbol* */
extern obj cx_timestamp; /* timestamp */
extern obj cx_typeassert_2Dprim_2Dctype; /* typeassert-prim-ctype */
extern obj cx_typecheck_2Dprim_2Dctype; /* typecheck-prim-ctype */
extern obj cx_var_2Dassigned_2Din_2Dexp_3F; /* var-assigned-in-exp? */
extern obj cx_var_2Dassignment_2Dcount; /* var-assignment-count */
extern obj cx_var_2Donly_2Dapplied_2Din_2Dexp_3F; /* var-only-applied-in-exp? */
extern obj cx_var_2Dreference_2Dcount; /* var-reference-count */
extern obj cx_var_2Dreferenced_2Din_2Dexp_3F; /* var-referenced-in-exp? */
extern obj cx_write_2F3; /* write/3 */
obj cx_analyze_2Dglobals; /* analyze-globals */
obj cx_beta_2Dsubstitute; /* beta-substitute */
obj cx_box_2Dexp; /* box-exp */
obj cx_box_2Dexp_3F; /* box-exp? */
obj cx_boxref_2Dexp; /* boxref-exp */
obj cx_boxref_2Dexp_3F; /* boxref-exp? */
obj cx_boxset_2Dexp; /* boxset-exp */
obj cx_boxset_2Dexp_3F; /* boxset-exp? */
obj cx_constant_2Dfold; /* constant-fold */
obj cx_fix_2Dletrecs; /* fix-letrecs */
obj cx_letrec_2A_2Dexp; /* letrec*-exp */
obj cx_letrec_2A_2Dexp_2D_3Ebody; /* letrec*-exp->body */
obj cx_letrec_2A_2Dexp_2D_3Erands; /* letrec*-exp->rands */
obj cx_letrec_2A_2Dexp_3F; /* letrec*-exp? */
obj cx_letrec_2Dexp; /* letrec-exp */
obj cx_letrec_2Dexp_3F; /* letrec-exp? */
obj cx_remove_2Dassignments; /* remove-assignments */
obj cx_the_2Dbox_2Dprim; /* the-box-prim */
obj cx_the_2Dboxref_2Dprim; /* the-boxref-prim */
obj cx_the_2Dboxset_2Dprim; /* the-boxset-prim */
obj cx_varassign_2A_2Dexp; /* varassign*-exp */
obj cx_varassign_2A_2Dexp_2D_3Eids; /* varassign*-exp->ids */
obj cx_varassign_2A_2Dexp_3F; /* varassign*-exp? */
static obj cx__231570; /* constant #1570 */
static obj cx__231594; /* constant #1594 */
static obj cx__231618; /* constant #1618 */
static obj cx__231687; /* constant #1687 */
static obj cx__231894; /* constant #1894 */
static obj cx__232044; /* constant #2044 */
static obj cx__232045; /* constant #2045 */
static obj cx__232047; /* constant #2047 */
static obj cx__232057; /* constant #2057 */
static obj cx__232060; /* constant #2060 */
static obj cx__232063; /* constant #2063 */
static obj cx__232071; /* constant #2071 */
static obj cx__232891; /* constant #2891 */
static obj cx__233677; /* constant #3677 */
static obj cx__234516; /* constant #4516 */
static obj cx__23647; /* constant #647 */
static obj cx__23651; /* constant #651 */
static obj cx__23653; /* constant #653 */
static obj cx__23657; /* constant #657 */
static obj cx__2369; /* constant #69 */
static obj cx_begin_2Dexp_3F_23185; /* constant begin-exp?#185 */
static obj cx_curry_2Dexp_3F_234038; /* constant curry-exp?#4038 */
static obj cx_keep_23531; /* constant keep#531 */
static obj cx_void_2Dexp_3F_23120; /* constant void-exp?#120 */

/* helper functions */
/* let-exp?#215 */
static obj cxs_let_2Dexp_3F_23215(obj v217_exp)
{ 
  if (bool_from_obj((isvector((v217_exp))) ? (((vectorlen((v217_exp))) == (3)) ? obj_from_bool((vectorref((v217_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v221_rator = (vectorref((v217_exp), (1)));
  if (bool_from_obj((isvector((v221_rator))) ? (((vectorlen((v221_rator))) == (3)) ? obj_from_bool((vectorref((v221_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v4900_tmp;
    obj v4899_tmp;
    { /* length */
    int n; obj l = (vectorref((v217_exp), (2)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4900_tmp = obj_from_fixnum(n); };
    { /* length */
    int n; obj l = (vectorref((v221_rator), (1)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4899_tmp = obj_from_fixnum(n); };
    return obj_from_bool(fixnum_from_obj(v4899_tmp) == fixnum_from_obj(v4900_tmp));
  }
  } else {
    return obj_from_bool(0);
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* degenerate-let-exp->body#1083 */
static obj cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083(obj v1085_exp)
{ 
    return ((isnull((vectorref((v1085_exp), (2))))) ? (vectorref((vectorref((v1085_exp), (1))), (2))) : (car((vectorref((v1085_exp), (2))))));
}

/* null-let-exp?#1109 */
static obj cxs_null_2Dlet_2Dexp_3F_231109(obj v1111_exp)
{ 
  if (bool_from_obj((isvector((v1111_exp))) ? (((vectorlen((v1111_exp))) == (3)) ? obj_from_bool((vectorref((v1111_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if ((isnull((vectorref((v1111_exp), (2)))))) {
  { /* let */
    obj v1115_rator = (vectorref((v1111_exp), (1)));
    return (bool_from_obj((isvector((v1115_rator))) ? (((vectorlen((v1115_rator))) == (3)) ? obj_from_bool((vectorref((v1115_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool(isnull((vectorref((v1115_rator), (1))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-lambda-exp?#1155 */
static obj cxs_identity_2Dlambda_2Dexp_3F_231155(obj v1157_exp)
{ 
  if (bool_from_obj((isvector((v1157_exp))) ? (((vectorlen((v1157_exp))) == (3)) ? obj_from_bool((vectorref((v1157_exp), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v1164_body = (vectorref((v1157_exp), (2)));
    obj v1165_ids = (vectorref((v1157_exp), (1)));
    return ((ispair((v1165_ids))) ? ((isnull((cdr((v1165_ids))))) ? (bool_from_obj((isvector((v1164_body))) ? (((vectorlen((v1164_body))) == (2)) ? obj_from_bool((vectorref((v1164_body), (0))) == (mksymbol(internsym("var-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool((car((v1165_ids))) == (vectorref((v1164_body), (1)))) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-let-exp?#1142 */
static obj cxs_identity_2Dlet_2Dexp_3F_231142(obj v1144_exp)
{ 
  if (bool_from_obj((isvector((v1144_exp))) ? (((vectorlen((v1144_exp))) == (3)) ? obj_from_bool((vectorref((v1144_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if (bool_from_obj(cxs_identity_2Dlambda_2Dexp_3F_231155((vectorref((v1144_exp), (1)))))) {
  { /* let */
    obj v1148_rands = (vectorref((v1144_exp), (2)));
    return ((ispair((v1148_rands))) ? obj_from_bool(isnull((cdr((v1148_rands))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* degenerate-let-exp?#1106 */
static obj cxs_degenerate_2Dlet_2Dexp_3F_231106(obj v1108_exp)
{ 
  { /* let */
    obj v1141_x = (cxs_null_2Dlet_2Dexp_3F_231109((v1108_exp)));
    return (bool_from_obj(v1141_x) ? (v1141_x) : (cxs_identity_2Dlet_2Dexp_3F_231142((v1108_exp))));
  }
}

/* gvarassign-exp?#1935 */
static obj cxs_gvarassign_2Dexp_3F_231935(obj v1937_exp)
{ 
  if (bool_from_obj((isvector((v1937_exp))) ? (((vectorlen((v1937_exp))) == (3)) ? obj_from_bool((vectorref((v1937_exp), (0))) == (mksymbol(internsym("varassign-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v1943_id = (vectorref((v1937_exp), (1)));
    return obj_from_bool(fixnum_from_obj(car((cdr((cdr((v1943_id))))))) < (0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* global-id-private-constant?#2010 */
static obj cxs_global_2Did_2Dprivate_2Dconstant_3F_232010(obj v2012_id)
{ 
    return ((fixnum_from_obj(car((cdr((cdr((v2012_id))))))) < (0)) ? obj_from_bool((car((cdr((cdr((cdr((v2012_id))))))))) == (mksymbol(internsym("private")))) : obj_from_bool(0));
}

/* rec-exp?#2149 */
static obj cxs_rec_2Dexp_3F_232149(obj v2151_exp)
{ 
  if (bool_from_obj((isvector((v2151_exp))) ? (((vectorlen((v2151_exp))) == (4)) ? obj_from_bool((vectorref((v2151_exp), (0))) == (mksymbol(internsym("fix-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v2158_body = (vectorref((v2151_exp), (3)));
    obj v2159_ids = (vectorref((v2151_exp), (1)));
    return ((ispair((v2159_ids))) ? ((isnull((cdr((v2159_ids))))) ? (bool_from_obj((isvector((v2158_body))) ? (((vectorlen((v2158_body))) == (2)) ? obj_from_bool((vectorref((v2158_body), (0))) == (mksymbol(internsym("var-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool((car((v2159_ids))) == (vectorref((v2158_body), (1)))) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* loop-exp?#2853 */
static obj cxs_loop_2Dexp_3F_232853(obj v2855_exp)
{ 
    return (bool_from_obj((isvector((v2855_exp))) ? (((vectorlen((v2855_exp))) == (3)) ? obj_from_bool((vectorref((v2855_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? (cxs_rec_2Dexp_3F_232149((vectorref((v2855_exp), (1))))) : obj_from_bool(0));
}

/* global-id-constant?#4696 */
static obj cxs_global_2Did_2Dconstant_3F_234696(obj v4698_id)
{ 
    return ((fixnum_from_obj(car((cdr((cdr((v4698_id))))))) < (0)) ? (car((cdr((cdr((cdr((v4698_id))))))))) : obj_from_bool(0));
}

/* letrec*-exp->body */
static obj cxs_letrec_2A_2Dexp_2D_3Ebody(obj v482_exp)
{ 
  { /* letrec */
    obj v499_ids;
    obj v498_body;
  { /* let */
    obj v4898_tmp = (vectorref((vectorref((v482_exp), (1))), (2)));
    obj v4897_tmp = (vectorref((vectorref((v482_exp), (1))), (1)));
    /* tail call */
    v499_ids = (v4897_tmp);
    v498_body = (v4898_tmp);
    goto s_skip_2Dinits;
  }
  s_skip_2Dinits:
  if ((isnull((v499_ids)))) {
    return (v498_body);
  } else {
  { /* let */
    obj v4896_tmp = (vectorref((vectorref((v498_body), (1))), (2)));
    obj v4895_tmp = (cdr((v499_ids)));
    /* tail call */
    v499_ids = (v4895_tmp);
    v498_body = (v4896_tmp);
    goto s_skip_2Dinits;
  }
  }
  }
}

/* gc roots */
static obj *globv[] = {
  &cx_the_2Dbox_2Dprim,
  &cx_the_2Dboxref_2Dprim,
  &cx_the_2Dboxset_2Dprim,
  &cx__231570,
  &cx__231594,
  &cx__231618,
  &cx__231687,
  &cx__231894,
  &cx__232044,
  &cx__232045,
  &cx__232047,
  &cx__232057,
  &cx__232060,
  &cx__232063,
  &cx__232071,
  &cx__232891,
  &cx__233677,
  &cx__234516,
  &cx__23647,
  &cx__23651,
  &cx__23653,
  &cx__23657,
  &cx__2369,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[457] = {
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,
};

/* host procedure */
#define MAX_LIVEREGS 41
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
    cx__2369 = (hpushstr(0, newstring("void(0)")));
    { static obj c[] = { obj_from_case(1) }; cx_void_2Dexp_3F_23120 = (obj)c; }
    { static obj c[] = { obj_from_case(2) }; cx_begin_2Dexp_3F_23185 = (obj)c; }
    { static obj c[] = { obj_from_case(4) }; cx_keep_23531 = (obj)c; }
    { static char s[] = { 46, 10, 0 };
    cx__23647 = (hpushstr(0, newstring(s))); }
    cx__23651 = (hpushstr(0, newstring("no clause matches ~s")));
    cx__23653 = (hpushstr(0, newstring("Error: ")));
    cx__23657 = (hpushstr(0, newstring("Error in ~a: ")));
    { static char s[] = { 104, 114, 101, 115, 101, 114, 118, 101, 40, 104, 98, 115, 122, 40, 49, 41, 44, 32, 36, 108, 105, 118, 101, 41, 59, 32, 47, 42, 32, 36, 108, 105, 118, 101, 32, 108, 105, 118, 101, 32, 114, 101, 103, 115, 32, 42, 47, 10, 32, 32, 32, 32, 42, 45, 45, 104, 112, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 10, 32, 32, 32, 32, 36, 114, 101, 116, 117, 114, 110, 32, 111, 98, 106, 40, 104, 101, 110, 100, 98, 108, 107, 40, 49, 41, 41, 59, 0 };
    cx__231570 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 111, 98, 106, 40, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 91, 48, 93, 41, 0 };
    cx__231594 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 111, 98, 106, 40, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 91, 48, 93, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 41, 0 };
    cx__231618 = (hpushstr(0, newstring(s))); }
    cx__231687 = (hpushstr(0, newstring("-box")));
    cx__231894 = (hpushstr(0, newstring("boxing error???")));
    cx__232044 = (hpushstr(0, newstring("")));
    { static char s[] = { 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 0 };
    cx__232045 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 44, 32, 0 };
    cx__232047 = (hpushstr(0, newstring(s))); }
    cx__232057 = (hpushstr(0, newstring("))")));
    cx__232060 = (hpushstr(0, newstring("(")));
    cx__232063 = (hpushstr(0, newstring("cxs_")));
    cx__232071 = (hpushstr(0, newstring("obj(")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("?!-effect")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("!-effect")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?-effect")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__232891 = (hendblk(3)); }
    r[0] = (hpushstr(0, newstring("void")));
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = (mknull());
    *--hp = r[0];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    r[1] = (hpushstr(1, newstring("obj")));
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__233677 = (hendblk(3)); }
    { static obj c[] = { obj_from_case(7) }; cx_curry_2Dexp_3F_234038 = (obj)c; }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*?-effect")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?-effect")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("*-effect")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__234516 = (hendblk(3)); }
    { static obj c[] = { obj_from_case(9) }; cx_varassign_2A_2Dexp = (obj)c; }
    { static obj c[] = { obj_from_case(16) }; cx_varassign_2A_2Dexp_3F = (obj)c; }
    { static obj c[] = { obj_from_case(20) }; cx_varassign_2A_2Dexp_2D_3Eids = (obj)c; }
    { static obj c[] = { obj_from_case(24) }; cx_letrec_2Dexp = (obj)c; }
    { static obj c[] = { obj_from_case(29) }; cx_letrec_2Dexp_3F = (obj)c; }
    { static obj c[] = { obj_from_case(36) }; cx_letrec_2A_2Dexp = (obj)c; }
    { static obj c[] = { obj_from_case(42) }; cx_letrec_2A_2Dexp_3F = (obj)c; }
    { static obj c[] = { obj_from_case(48) }; cx_letrec_2A_2Dexp_2D_3Erands = (obj)c; }
    { static obj c[] = { obj_from_case(50) }; cx_letrec_2A_2Dexp_2D_3Ebody = (obj)c; }
    { static obj c[] = { obj_from_case(51) }; cx_fix_2Dletrecs = (obj)c; }
    cx_the_2Dbox_2Dprim = (cx__231570);
    { static obj c[] = { obj_from_case(183) }; cx_box_2Dexp = (obj)c; }
    { static obj c[] = { obj_from_case(184) }; cx_box_2Dexp_3F = (obj)c; }
    cx_the_2Dboxref_2Dprim = (cx__231594);
    { static obj c[] = { obj_from_case(185) }; cx_boxref_2Dexp = (obj)c; }
    { static obj c[] = { obj_from_case(186) }; cx_boxref_2Dexp_3F = (obj)c; }
    cx_the_2Dboxset_2Dprim = (cx__231618);
    { static obj c[] = { obj_from_case(187) }; cx_boxset_2Dexp = (obj)c; }
    { static obj c[] = { obj_from_case(188) }; cx_boxset_2Dexp_3F = (obj)c; }
    { static obj c[] = { obj_from_case(189) }; cx_remove_2Dassignments = (obj)c; }
    { static obj c[] = { obj_from_case(223) }; cx_analyze_2Dglobals = (obj)c; }
    { static obj c[] = { obj_from_case(323) }; cx_constant_2Dfold = (obj)c; }
    { static obj c[] = { obj_from_case(376) }; cx_beta_2Dsubstitute = (obj)c; }
    r[0] = obj_from_void(0);
    r[1+0] = r[0];
    pc = 0; /* exit from module init */
    r[1+1] = r[0];  
    r += 1; /* shift reg wnd */
    assert(rc = 2);
    goto jump;

case 1: /* void-exp?#120 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_void_2Dexp_3F_23120: /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2+0] = (cx_equal_3F);
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = r[0];  
    r[2+2] = (vectorref((r[1]), (2)));
    r[2+3] = (cx__2369);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 2: /* begin-exp?#185 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_begin_2Dexp_3F_23185: /* k exp */
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[2] = (vectorref((r[1]), (1)));
    r[2] = (vectorref((r[2]), (1)));
    r[3] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[3]), (2)));
  if ((ispair((r[2])))) {
    r[4] = (cdr((r[2])));
    r[4] = obj_from_bool(isnull((r[4])));
  if (bool_from_obj(r[4])) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(3);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_var_2Dreferenced_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[2])));
    r[5+3] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
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
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 3: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = r[2];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(!bool_from_obj(r[1]));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 4: /* keep#531 k f lst */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_keep_23531: /* k f lst */
  if ((isnull((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(4);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(5);
    r[3] = (hendblk(4+1));
    r[4+0] = r[1];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[2])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 5: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r keep f k lst */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(6);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    r[3] = (cdr((r[5])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    r[3] = (cdr((r[5])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 6: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k lst */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = (car((r[3])));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 7: /* curry-exp?#4038 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_curry_2Dexp_3F_234038: /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(8);
    r[4] = (hendblk(3+1));
    r[5+0] = (cx_labelapp_2Dexp_3F);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 8: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k body */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[0] = r[3];  
    r[1] = r[2];  
    r[2] = r[5];  
    goto s_loop;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop: /* k ids rands */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[2])))) {
    r[3] = (car((r[2])));
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (2))) {
    r[4] = (vectorref((r[3]), (0)));
    r[3] = obj_from_bool((r[4]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (car((r[2])));
    r[3] = (vectorref((r[3]), (1)));
    r[4] = (car((r[1])));
    r[3] = obj_from_bool((r[3]) == (r[4]));
  if (bool_from_obj(r[3])) {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop;
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
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
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
  }

case 9: /* varassign*-exp k ids rands */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_varassign_2A_2Dexp: /* k ids rands */
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(12);
    r[3] = (hendblk(3+1));
    r[0] = r[3];  
    /* r[1] */    
    goto s_loop_v10666;

s_loop_v10666: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(10);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10666;
  }

case 10: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[5] = (cdr((r[4])));
    r[5] = (car((r[5])));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(11);
    r[6] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 11: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r sym */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 12: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k rands */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(15);
    r[5] = (hendblk(3+1));
    r[6+0] = r[5];  
    r[6+1] = r[2];  
    r[6+2] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_recur;

s_recur: /* k ids tmp-ids */
  if ((isnull((r[1])))) {
    { /* vector */
    hreserve(hbsz(4+1), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__2369);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(4+1)); }
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(13);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_recur;
  }

case 13: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k ids tmp-ids */
    r[5] = (car((r[4])));
    { /* vector */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(2+1)); }
    r[6] = (car((r[3])));
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(14);
    r[6] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
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
    /* ek r k exp1 r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 15: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k rands r */
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 16: /* varassign*-exp? k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_varassign_2A_2Dexp_3F: /* k exp */
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[2] = (vectorref((r[1]), (1)));
    r[2] = (vectorref((r[2]), (1)));
    r[3] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[3]), (2)));
    r[4+0] = r[0];  
    r[4+1] = r[2];  
    r[4+2] = r[3];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_assigns_3F;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 17: /* clo k tmp-ids body */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_assigns_3F: /* k tmp-ids body ids */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(17);
    r[4] = (hendblk(1+1));
    hreserve(hbsz(5+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(18);
    r[4] = (hendblk(5+1));
  if ((isnull((r[1])))) {
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_void_2Dexp_3F_23120;
  } else {
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(17);
    r[5] = (hendblk(1+1));
    r[6+0] = obj_from_ktrap();
    r[6+1] = obj_from_bool(0);
    r[6+2] = r[5];  
    r[6+3] = r[3];  
    r[6+4] = r[1];  
    r[6+5] = r[2];  
    r[6+6] = r[0];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v10612;
  }

case 18: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
s_l_v10612: /* ek r assigns? ids tmp-ids body k */
  if (bool_from_obj(r[1])) {
    r[7+0] = r[6];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(19);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = r[5];  
    goto gs_begin_2Dexp_3F_23185;
  }

case 19: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r assigns? k ids tmp-ids body */
  if (bool_from_obj(r[1])) {
    r[7] = (vectorref((r[6]), (2)));
    r[7] = (car((r[7])));
    r[8] = (vectorref((r[6]), (1)));
    r[8] = (vectorref((r[8]), (2)));
  if ((isvector((r[7])))) {
  if (((vectorlen((r[7]))) == (3))) {
    r[9] = (vectorref((r[7]), (0)));
    r[9] = obj_from_bool((r[9]) == (mksymbol(internsym("varassign-exp"))));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[9] = (vectorref((r[7]), (1)));
    r[10] = (vectorref((r[7]), (2)));
  if ((isvector((r[10])))) {
  if (((vectorlen((r[10]))) == (2))) {
    r[11] = (vectorref((r[10]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[10]), (1)));
    r[12] = (car((r[5])));
    r[11] = obj_from_bool((r[11]) == (r[12]));
  if (bool_from_obj(r[11])) {
    { /* memq */
    obj x = (r[9]), l = r[4];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[11] = (l == mknull() ? obj_from_bool(0) : l); }
    r[9] = obj_from_bool(!bool_from_obj(r[11]));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (cdr((r[5])));
    r[3] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 20: /* varassign*-exp->ids k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_varassign_2A_2Dexp_2D_3Eids: /* k exp */
    r[2] = (vectorref((r[1]), (1)));
    r[2] = (vectorref((r[2]), (2)));
    /* r[0] */    
    r[1] = r[2];  
    goto s_assignees;

case 21: /* clo k body */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_assignees: /* k body */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(21);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(22);
    r[2] = (hendblk(3+1));
    r[0] = r[2];  
    /* r[1] */    
    goto gs_void_2Dexp_3F_23120;

case 22: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r assignees body k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(23);
    r[5] = (hendblk(2+1));
    r[6] = (vectorref((r[3]), (1)));
    r[6] = (vectorref((r[6]), (2)));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 23: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k body */
    r[4] = (vectorref((r[3]), (2)));
    r[4] = (car((r[4])));
    r[4] = (vectorref((r[4]), (1)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 24: /* letrec-exp k ids rands body */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k ids rands body */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(25);
    r[4] = (hendblk(4+1));
    r[0] = r[4];  
    /* r[1] */    
    /* r[2] */    
    goto gs_varassign_2A_2Dexp;

case 25: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r rands k ids body */
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(26);
    r[6] = (hendblk(5+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 26: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r rands k ids r body */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[9] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[10] = (hendblk(3+1)); }
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = r[3];  
    *--hp = obj_from_case(28);
    r[11] = (hendblk(3+1));
    r[0] = (r[11]);
    r[1] = r[2];  
    goto s_loop_v10558;

s_loop_v10558: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(27);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10558;
  }

case 27: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(4+1), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__2369);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 28: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r ids */
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 29: /* letrec-exp? k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_letrec_2Dexp_3F: /* k exp */
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[2] = (vectorref((r[1]), (1)));
    r[2] = (vectorref((r[2]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[4]), (2)));
  if ((ispair((r[2])))) {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(32);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    goto s_loop_v10531;
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
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 30: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop_v10531: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isnull((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(30);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(31);
    r[2] = (hendblk(3+1));
    r[0] = r[2];  
    r[1] = (car((r[1])));
    goto gs_void_2Dexp_3F_23120;
  }

case 31: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 32: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k body */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(33);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_begin_2Dexp_3F_23185;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 33: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k body */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[5] = (car((r[5])));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(34);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[5];  
    goto gs_varassign_2A_2Dexp_3F;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 34: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp1 ids k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(35);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto gs_varassign_2A_2Dexp_2D_3Eids;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 35: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k */
    r[4+0] = (cx_equal_3F);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[2];  
    r[4+3] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 36: /* letrec*-exp k ids rands body */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k ids rands body */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(39);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    goto s_recur_v10478;

s_recur_v10478: /* k ids rands body */
  if ((isnull((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    r[5] = (cdr((r[2])));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(37);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    goto s_recur_v10478;
  }

case 37: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k ids rands */
    r[5] = (car((r[4])));
    r[6] = (car((r[3])));
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(38);
    r[6] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 38: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp1 r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 39: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r rands k ids */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(41);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v10463;

s_loop_v10463: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(40);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10463;
  }

case 40: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(4+1), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__2369);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 41: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r ids */
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 42: /* letrec*-exp? k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_letrec_2A_2Dexp_3F: /* k exp */
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[2] = (vectorref((r[1]), (1)));
    r[2] = (vectorref((r[2]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[4]), (2)));
  if ((ispair((r[2])))) {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(45);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    goto s_loop_v10438;
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
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 43: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop_v10438: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isnull((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(43);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(44);
    r[2] = (hendblk(3+1));
    r[0] = r[2];  
    r[1] = (car((r[1])));
    goto gs_void_2Dexp_3F_23120;
  }

case 44: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 45: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r body ids k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_inits_3F;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 46: /* clo k ids body */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
s_inits_3F: /* k ids body */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(46);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(47);
    r[3] = (hendblk(4+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_begin_2Dexp_3F_23185;
  }

case 47: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r inits? k ids body */
  if (bool_from_obj(r[1])) {
    r[6] = (vectorref((r[5]), (2)));
    r[6] = (car((r[6])));
    r[7] = (vectorref((r[5]), (1)));
    r[7] = (vectorref((r[7]), (2)));
  if ((isvector((r[6])))) {
  if (((vectorlen((r[6]))) == (3))) {
    r[8] = (vectorref((r[6]), (0)));
    r[8] = obj_from_bool((r[8]) == (mksymbol(internsym("varassign-exp"))));
  } else {
    r[8] = obj_from_bool(0);
  }
  } else {
    r[8] = obj_from_bool(0);
  }
  if (bool_from_obj(r[8])) {
    r[8] = (vectorref((r[6]), (1)));
    r[9] = (car((r[4])));
    r[8] = obj_from_bool((r[8]) == (r[9]));
  } else {
    r[8] = obj_from_bool(0);
  }
  if (bool_from_obj(r[8])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (cdr((r[4])));
    r[3] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 48: /* letrec*-exp->rands k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_letrec_2A_2Dexp_2D_3Erands: /* k exp */
    r[2] = (vectorref((r[1]), (1)));
    r[2] = (vectorref((r[2]), (1)));
    r[3] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[3]), (2)));
    /* r[0] */    
    r[1] = r[2];  
    r[2] = r[3];  
    goto s_inits;

s_inits: /* k ids body */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (vectorref((r[2]), (1)));
    r[4] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(49);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_inits;
  }

case 49: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k body */
    r[4] = (vectorref((r[3]), (2)));
    r[4] = (car((r[4])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 50: /* letrec*-exp->body k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_letrec_2A_2Dexp_2D_3Ebody((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 51: /* fix-letrecs k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(53);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(54);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(55);
    r[4] = (hendblk(0+1));
    hreserve(hbsz(0+1), 5); /* 5 live regs */
    *--hp = obj_from_case(52);
    r[5] = (hendblk(0+1));
    r[6+0] = r[0];  
    r[6+1] = r[1];  
    r[6+2] = r[4];  
    r[6+3] = r[3];  
    r[6+4] = r[2];  
    r[6+5] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_fix;

case 52: /* clo k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k b */
    r[2] = (vectorref((r[1]), (1)));
    r[2] = obj_from_bool((mksymbol(internsym("u"))) == (r[2]));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 53: /* clo k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k b */
    r[2] = (vectorref((r[1]), (1)));
    r[2] = obj_from_bool((mksymbol(internsym("s"))) == (r[2]));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 54: /* clo k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k b */
    r[2] = (vectorref((r[1]), (1)));
    r[2] = obj_from_bool((mksymbol(internsym("l"))) == (r[2]));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 55: /* clo k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k b */
    r[2] = (vectorref((r[1]), (1)));
    r[2] = obj_from_bool((mksymbol(internsym("c"))) == (r[2]));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 56: /* clo k ids lams body */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k ids lams body l-bnd? s-bnd? */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(56);
    r[6] = (hendblk(2+1));
    hreserve(hbsz(7+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(61);
    r[6] = (hendblk(7+1));
    r[7+0] = r[6];  
    r[7+1] = r[1];  
    r[7+2] = r[2];  
    r[7+3] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v10304;

s_loop_v10304: /* k id id ids */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    r[5] = (cdr((r[2])));
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(57);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    goto s_loop_v10304;
  }

case 57: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k id id */
    r[6] = (car((r[5])));
    r[7] = (car((r[4])));
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(58);
    r[8] = (hendblk(5+1));
    r[0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 58: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids lam id k r */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(59);
    r[7] = (hendblk(2+1));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = obj_from_case(60);
    r[7] = (hendblk(3+1));
    r[8+0] = r[7];  
    r[8+1] = r[2];  
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v10315;

case 59: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v10315: /* k id r */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[1])));
    { /* assq */
    obj x = (r[3]), l = (r[2]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[4] = (l == mknull() ? obj_from_bool(0) : p); }
    r[3] = obj_from_bool(!bool_from_obj(r[4]));
  if (bool_from_obj(r[3])) {
    r[3] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v10315;
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

case 60: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k lam id */
  if (bool_from_obj(r[1])) {
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("s")));
    *--hp = (mksymbol(internsym("bnd")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("l")));
    *--hp = (mksymbol(internsym("bnd")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 61: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r l-bnd? s-bnd? refix k lams ids body */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(62);
    r[9] = (hendblk(7+1));
    r[10+0] = r[9];  
    r[10+1] = r[2];  
    r[10+2] = r[1];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_keep_23531;

case 62: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r r s-bnd? refix k lams ids body */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(63);
    r[9] = (hendblk(6+1));
    r[0] = r[9];  
    r[1] = r[3];  
    /* r[2] */    
    goto gs_keep_23531;

case 63: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r refix k lams ids body r */
  if ((isnull((r[1])))) {
  if ((isnull((r[7])))) {
    r[8] = r[6];  
  } else {
    { /* vector */
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("fix-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(4+1)); }
  }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = r[7];  
    *--hp = obj_from_case(65);
    r[8] = (hendblk(5+1));
    r[0] = r[8];  
    r[1] = r[7];  
    goto s_loop_v10281;
  }

s_loop_v10281: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(64);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10281;
  }

case 64: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 65: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r r refix body r k */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(67);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = r[2];  
    goto s_loop_v10266;

s_loop_v10266: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(66);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10266;
  }

case 66: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (3)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 67: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r refix body r r k */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(68);
    r[7] = (hendblk(2+1));
    r[8+0] = r[2];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[4];  
    r[8+3] = r[1];  
    r[8+4] = r[3];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 68: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(70);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop_v10247;

s_loop_v10247: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(69);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10247;
  }

case 69: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (3)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 70: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k r */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(72);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v10232;

s_loop_v10232: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(71);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10232;
  }

case 71: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 72: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 73: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
s_fix: /* k exp c-bnd? l-bnd? s-bnd? u-bnd? */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(73);
    r[6] = (hendblk(4+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[7] = (vectorref((r[1]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7+0] = r[0];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[7] = (vectorref((r[1]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("varassign-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7] = (vectorref((r[1]), (1)));
    r[8] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[0];  
    *--hp = obj_from_case(74);
    r[9] = (hendblk(2+1));
    r[0] = r[9];  
    r[1] = r[8];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_fix;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[7] = (vectorref((r[1]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7] = (vectorref((r[1]), (1)));
    r[8] = (vectorref((r[1]), (2)));
    r[9] = (vectorref((r[1]), (3)));
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[0];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = obj_from_case(75);
    r[10] = (hendblk(4+1));
    r[0] = (r[10]);
    r[1] = r[9];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_fix;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231106((r[1])))) {
    r[7] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083((r[1])));
    /* r[0] */    
    r[1] = r[7];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_fix;
  } else {
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(56);
    r[7] = (hendblk(2+1));
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(158);
    r[8] = (hendblk(0+1));
    hreserve(hbsz(9+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[8];  
    *--hp = obj_from_case(78);
    r[7] = (hendblk(9+1));
    r[0] = r[7];  
    /* r[1] */    
    goto gs_letrec_2Dexp_3F;
  }
  }
  }
  }

case 74: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 75: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp fix test-exp k */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(76);
    r[6] = (hendblk(4+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 76: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix test-exp k r */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(77);
    r[6] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 77: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("if-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 78: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek r classify-bindings c-bnd? l-bnd? s-bnd? u-bnd? refix fix k exp */
  if (bool_from_obj(r[1])) {
    r[11] = (vectorref((r[10]), (1)));
    r[11] = (vectorref((r[11]), (1)));
    r[12] = (vectorref((r[10]), (1)));
    r[12] = (vectorref((r[12]), (2)));
    r[13] = (vectorref((r[12]), (2)));
    r[12] = (car((r[13])));
    r[12] = (vectorref((r[12]), (2)));
    r[13] = (vectorref((r[10]), (1)));
    r[13] = (vectorref((r[13]), (2)));
    r[14] = (vectorref((r[13]), (1)));
    r[13] = (vectorref((r[14]), (2)));
    hreserve(hbsz(8+1), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (r[13]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(79);
    r[14] = (hendblk(8+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[14]);
    r[2] = (r[11]);
    r[3] = (r[12]);
    r[4] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(114);
    r[11] = (hendblk(9+1));
    r[0] = (r[11]);
    r[1] = (r[10]);
    goto gs_letrec_2A_2Dexp_3F;
  }

case 79: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r c-bnd? l-bnd? s-bnd? u-bnd? body refix fix k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(80);
    r[10] = (hendblk(8+1));
    r[11+0] = (r[10]);
    r[11+1] = r[2];  
    r[11+2] = r[1];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_keep_23531;

case 80: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r l-bnd? s-bnd? r u-bnd? body refix fix k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(81);
    r[10] = (hendblk(8+1));
    r[0] = (r[10]);
    r[1] = r[2];  
    r[2] = r[4];  
    goto gs_keep_23531;

case 81: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r s-bnd? r u-bnd? body refix fix r k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(82);
    r[10] = (hendblk(8+1));
    r[0] = (r[10]);
    r[1] = r[2];  
    r[2] = r[3];  
    goto gs_keep_23531;

case 82: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r r u-bnd? r body refix fix r k */
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(83);
    r[10] = (hendblk(7+1));
    r[0] = (r[10]);
    r[1] = r[3];  
    /* r[2] */    
    goto gs_keep_23531;

case 83: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r r body refix fix r r k */
    hreserve(hbsz(8+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(85);
    r[9] = (hendblk(8+1));
    r[0] = r[9];  
    r[1] = r[2];  
    goto s_loop_v10141;

s_loop_v10141: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(84);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10141;
  }

case 84: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 85: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r r body r refix fix r r k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(88);
    r[10] = (hendblk(8+1));
    r[0] = (r[10]);
    r[1] = r[2];  
    r[2] = r[6];  
    goto s_loop_v10120;

s_loop_v10120: /* k id fix */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(86);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v10120;
  }

case 86: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(87);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (vectorref((r[5]), (3)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 87: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 88: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r body r refix r fix r r k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(89);
    r[10] = (hendblk(8+1));
  if ((isnull((r[7])))) {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(106);
    r[11] = (hendblk(3+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 89: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r r refix r r fix r r k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(92);
    r[10] = (hendblk(8+1));
    r[0] = (r[10]);
    r[1] = r[2];  
    r[2] = r[6];  
    goto s_loop_v10095;

s_loop_v10095: /* k id fix */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(90);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v10095;
  }

case 90: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(91);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (vectorref((r[5]), (3)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 91: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 92: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r r refix r r fix r r k */
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(93);
    r[10] = (hendblk(7+1));
    hreserve(hbsz(0+1), 11); /* 11 live regs */
    *--hp = obj_from_case(104);
    r[11] = (hendblk(0+1));
    r[12+0] = (cx_reduce_2Dright_2Fright_2Dseed);
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[10]);
    r[12+2] = (r[11]);
    r[12+3] = r[2];  
    r[12+4] = r[1];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 93: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r refix r r fix r r k */
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(94);
    r[9] = (hendblk(4+1));
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[9];  
    r[10+2] = r[4];  
    r[10+3] = r[3];  
    r[10+4] = r[1];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 94: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix r r k */
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(96);
    r[6] = (hendblk(5+1));
    r[0] = r[6];  
    r[1] = r[3];  
    goto s_loop_v10072;

s_loop_v10072: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(95);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10072;
  }

case 95: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(4+1), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__2369);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 96: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix r r k r */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(99);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v10051;

s_loop_v10051: /* k id fix */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(97);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v10051;
  }

case 97: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(98);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (vectorref((r[5]), (3)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 98: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 99: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r k r r */
    { fixnum_t v10765_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10765_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v10765_tmp);
    hreserve(hbsz(3)*c, 7); /* 7 live regs */
    l = r[1];   t = r[6];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[7] = (o); } }
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(101);
    r[8] = (hendblk(4+1));
    r[0] = r[8];  
    r[1] = r[2];  
    goto s_loop_v10036;

s_loop_v10036: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(100);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10036;
  }

case 100: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 101: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k r r */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(103);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[2];  
    goto s_loop_v10021;

s_loop_v10021: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(102);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v10021;
  }

case 102: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 103: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r r */
    { fixnum_t v10764_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10764_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v10764_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[1];   t = r[5];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[6] = (o); } }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 104: /* clo k exp1 exp2 */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k exp1 exp2 */
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(105);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_timestamp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 105: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp1 exp2 */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 106: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix r k */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(108);
    r[5] = (hendblk(4+1));
    r[0] = r[5];  
    r[1] = r[3];  
    goto s_loop_v9972;

s_loop_v9972: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(107);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v9972;
  }

case 107: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 108: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix r k r */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_case(111);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9951;

s_loop_v9951: /* k id fix */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(109);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9951;
  }

case 109: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(110);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (vectorref((r[5]), (3)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 110: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 111: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k r */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(112);
    r[5] = (hendblk(2+1));
    r[6+0] = r[5];  
    r[6+1] = r[2];  
    r[6+2] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_varassign_2A_2Dexp;

case 112: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(113);
    r[4] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 113: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 114: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek r classify-bindings l-bnd? s-bnd? u-bnd? c-bnd? refix fix k exp */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(115);
    r[11] = (hendblk(9+1));
    r[0] = (r[11]);
    r[1] = (r[10]);
    goto gs_letrec_2A_2Dexp_2D_3Erands;
  } else {
  if ((isvector((r[10])))) {
  if (((vectorlen((r[10]))) == (4))) {
    r[11] = (vectorref((r[10]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[10]), (1)));
    r[12] = (vectorref((r[10]), (2)));
    r[13] = (vectorref((r[10]), (3)));
    hreserve(hbsz(3+1), 14); /* 14 live regs */
    *--hp = (r[11]);
    *--hp = (r[12]);
    *--hp = r[9];  
    *--hp = obj_from_case(146);
    r[14] = (hendblk(3+1));
    r[0] = (r[14]);
    r[1] = (r[13]);
    r[2] = r[8];  
    goto s_loop_v9685;
  } else {
  if ((isvector((r[10])))) {
  if (((vectorlen((r[10]))) == (3))) {
    r[11] = (vectorref((r[10]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[10]), (1)));
    r[12] = (vectorref((r[10]), (2)));
    hreserve(hbsz(3+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = (r[11]);
    *--hp = r[8];  
    *--hp = obj_from_case(149);
    r[13] = (hendblk(3+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    r[2] = r[8];  
    goto s_loop_v9662;
  } else {
  if ((isvector((r[10])))) {
  if (((vectorlen((r[10]))) == (3))) {
    r[11] = (vectorref((r[10]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[10]), (1)));
    r[12] = (vectorref((r[10]), (2)));
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = obj_from_case(151);
    r[13] = (hendblk(2+1));
    r[0] = r[8];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[10])))) {
  if (((vectorlen((r[10]))) == (3))) {
    r[11] = (vectorref((r[10]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[10]), (1)));
    r[12] = (vectorref((r[10]), (2)));
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = obj_from_case(152);
    r[13] = (hendblk(2+1));
    r[0] = r[8];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[10])))) {
  if (((vectorlen((r[10]))) == (3))) {
    r[11] = (vectorref((r[10]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[10]), (1)));
    r[12] = (vectorref((r[10]), (2)));
    hreserve(hbsz(3+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = (r[11]);
    *--hp = r[8];  
    *--hp = obj_from_case(153);
    r[13] = (hendblk(3+1));
    r[0] = r[8];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[11] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[11]))));
    hreserve(hbsz(3+1), 12); /* 12 live regs */
    *--hp = r[9];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = obj_from_case(155);
    r[12] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (r[11]);
    r[3] = (cx__23657);
    r[4] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (cx__23653);
    r[3] = obj_from_bool(1);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }
  }
  }
  }

case 115: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek r classify-bindings l-bnd? s-bnd? u-bnd? c-bnd? refix fix k exp */
    r[11] = (vectorref((r[10]), (1)));
    r[11] = (vectorref((r[11]), (1)));
    r[12] = (cxs_letrec_2A_2Dexp_2D_3Ebody((r[10])));
    hreserve(hbsz(8+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (r[12]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(116);
    r[13] = (hendblk(8+1));
    r[14+0] = r[2];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[13]);
    r[14+2] = (r[11]);
    r[14+3] = r[1];  
    r[14+4] = (r[12]);
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 116: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r l-bnd? s-bnd? u-bnd? c-bnd? body refix fix k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(117);
    r[10] = (hendblk(8+1));
    r[11+0] = (r[10]);
    r[11+1] = r[2];  
    r[11+2] = r[1];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_keep_23531;

case 117: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r s-bnd? r u-bnd? c-bnd? body refix fix k */
    hreserve(hbsz(9+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(118);
    r[10] = (hendblk(9+1));
    r[0] = (r[10]);
    r[1] = r[5];  
    r[2] = r[3];  
    goto gs_keep_23531;

case 118: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek r s-bnd? r r u-bnd? c-bnd? body refix fix k */
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(119);
    r[11] = (hendblk(9+1));
    r[0] = (r[11]);
    r[1] = r[2];  
    r[2] = r[4];  
    goto gs_keep_23531;

case 119: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r u-bnd? c-bnd? body refix fix r k */
    hreserve(hbsz(10+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(121);
    r[11] = (hendblk(10+1));
    r[0] = (r[11]);
    r[1] = r[2];  
    goto s_loop_v9880;

s_loop_v9880: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(120);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v9880;
  }

case 120: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 121: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r u-bnd? c-bnd? body refix fix r r k */
    hreserve(hbsz(10+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(124);
    r[12] = (hendblk(10+1));
    r[0] = (r[12]);
    r[1] = r[2];  
    r[2] = r[8];  
    goto s_loop_v9859;

s_loop_v9859: /* k id fix */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(122);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9859;
  }

case 122: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(123);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (vectorref((r[5]), (3)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 123: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 124: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10]; }
    r += 1; /* shift reg. wnd */
    /* ek r r u-bnd? c-bnd? body refix r fix r r k */
    hreserve(hbsz(9+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(125);
    r[12] = (hendblk(9+1));
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(142);
    r[13] = (hendblk(2+1));
    r[0] = (r[12]);
    r[1] = (r[13]);
    /* r[2] */    
    goto gs_keep_23531;

case 125: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek r c-bnd? body refix r r fix r r k */
    hreserve(hbsz(7+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(131);
    r[11] = (hendblk(7+1));
    r[12+0] = (r[11]);
    r[12+1] = r[1];  
    r[12+2] = r[2];  
    r[12+3] = r[7];  
    r[12+4] = r[3];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_recur_v9816;

s_recur_v9816: /* k cub* c-bnd? fix body */
  if ((isnull((r[1])))) {
    r[5+0] = r[3];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (car((r[1])));
    r[6] = (cdr((r[1])));
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(126);
    r[7] = (hendblk(4+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_recur_v9816;
  }

case 126: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r c-bnd? fix b k */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(127);
    r[6] = (hendblk(2+1));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_case(129);
    r[6] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 127: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(128);
    r[4] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 128: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 129: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix k b */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(130);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (vectorref((r[4]), (3)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (vectorref((r[4]), (3)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 130: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k b */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = (vectorref((r[3]), (2)));
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 131: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r refix r r fix r r k */
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(132);
    r[9] = (hendblk(4+1));
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[9];  
    r[10+2] = r[4];  
    r[10+3] = r[3];  
    r[10+4] = r[1];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 132: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix r r k */
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(134);
    r[6] = (hendblk(5+1));
    r[0] = r[6];  
    r[1] = r[3];  
    goto s_loop_v9797;

s_loop_v9797: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(133);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v9797;
  }

case 133: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(4+1), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__2369);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 134: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix r r k r */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(137);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v9776;

s_loop_v9776: /* k id fix */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(135);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9776;
  }

case 135: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(136);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (vectorref((r[5]), (3)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 136: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 137: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r k r r */
    { fixnum_t v10763_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10763_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v10763_tmp);
    hreserve(hbsz(3)*c, 7); /* 7 live regs */
    l = r[1];   t = r[6];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[7] = (o); } }
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(139);
    r[8] = (hendblk(4+1));
    r[0] = r[8];  
    r[1] = r[2];  
    goto s_loop_v9761;

s_loop_v9761: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(138);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v9761;
  }

case 138: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 139: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k r r */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(141);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[2];  
    goto s_loop_v9746;

s_loop_v9746: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(140);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v9746;
  }

case 140: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 141: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r r */
    { fixnum_t v10762_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10762_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v10762_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[1];   t = r[5];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[6] = (o); } }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 142: /* clo k b */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k b c-bnd? u-bnd? */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(143);
    r[4] = (hendblk(3+1));
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 143: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r u-bnd? b k */
  if (bool_from_obj(r[1])) {
    r[5+0] = r[4];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop_v9685: /* k id fix */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(144);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9685;
  }

case 144: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(145);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 145: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 146: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k prim effect */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v9662: /* k id fix */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(147);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9662;
  }

case 147: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(148);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 148: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 149: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix rator k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(150);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 150: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 151: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k ids */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 152: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("letcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 153: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fix cont-exp k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(154);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 154: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("withcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 155: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(156);
    r[5] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[3];  
    r[7+3] = (cx__23651);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 156: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(157);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23647);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 157: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 158: /* clo k ids rands body */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k ids rands body */
    hreserve(hbsz(1), 4); /* 4 live regs */
    *--hp = obj_from_void(0);
    r[4] = (hendblk(1));
    hreserve(hbsz(0+1), 5); /* 5 live regs */
    *--hp = obj_from_case(173);
    r[5] = (hendblk(0+1));
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(159);
    r[5] = (hendblk(5+1));
    r[6+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = r[3];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 159: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r fl-simple? rands ids k rr&b-vi */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(160);
    r[7] = (hendblk(5+1));
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(171);
    r[8] = (hendblk(0+1));
    r[9+0] = (cx_reduce_2Dright);
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[7];  
    r[9+2] = r[8];  
    r[9+3] = r[1];  
    r[9+4] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 160: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r fl-simple? rands ids k rr&b-vi */
    (void)(objptr_from_obj(r[6])[0] = (r[1]));
    r[7+0] = r[5];  
    r[7+1] = r[4];  
    r[7+2] = r[3];  
    r[7+3] = r[6];  
    r[7+4] = r[2];  
    r[7+5] = r[4];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v9502;

s_loop_v9502: /* k id id rr&b-vi fl-simple? ids */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    r[7] = (cdr((r[2])));
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(161);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = r[6];  
    r[2] = r[7];  
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v9502;
  }

case 161: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r rr&b-vi fl-simple? ids k id id */
    r[8] = (car((r[7])));
    r[9] = (car((r[6])));
    hreserve(hbsz(1), 10); /* 10 live regs */
    *--hp = obj_from_void(0);
    r[10] = (hendblk(1));
    hreserve(hbsz(7+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(162);
    r[11] = (hendblk(7+1));
    hreserve(hbsz(1+1), 12); /* 12 live regs */
    *--hp = r[8];  
    *--hp = obj_from_case(170);
    r[12] = (hendblk(1+1));
    r[0] = (r[11]);
    r[1] = (r[12]);
    r[2] = (objptr_from_obj(r[2])[0]);
    goto gs_keep_23531;

case 162: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r fl-simple? ids exp id k r id-vu* */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(164);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    /* r[1] */    
    goto s_loop_v9559;

s_loop_v9559: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(163);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v9559;
  }

case 163: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (cdr((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 164: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r fl-simple? ids exp id k r id-vu* */
    (void)(objptr_from_obj(r[8])[0] = (r[1]));
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(165);
    r[9] = (hendblk(2+1));
    r[10] = (objptr_from_obj(r[8])[0]);
    r[10] = obj_from_bool(isnull((r[10])));
  if (bool_from_obj(r[10])) {
    { /* vector */
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("u")));
    *--hp = (mksymbol(internsym("bnd")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[10] = (hendblk(4+1)); }
    r[0] = obj_from_ktrap();
    r[1] = (r[10]);
    r[2] = r[6];  
    r[3] = r[7];  
    goto s_l_v9550;
  } else {
    r[10] = (objptr_from_obj(r[8])[0]);
    hreserve(hbsz(5+1), 11); /* 11 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[9];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(166);
    r[11] = (hendblk(5+1));
    r[0] = (r[11]);
    r[1] = (r[10]);
    goto s_loop_v9544;
  }

case 165: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v9550: /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v9544: /* k id */
  if ((!(isnull((r[1]))))) {
    r[2] = (car((r[1])));
    r[2] = (vectorref((r[2]), (1)));
  if (bool_from_obj(r[2])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[3];  
    goto s_loop_v9544;
  }
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 166: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r fl-simple? ids k exp id */
  if (bool_from_obj(r[1])) {
    { /* vector */
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("c")));
    *--hp = (mksymbol(internsym("bnd")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(4+1)); }
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(167);
    r[7] = (hendblk(3+1));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(168);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 167: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp id */
  if (bool_from_obj(r[1])) {
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("s")));
    *--hp = (mksymbol(internsym("bnd")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (3))) {
    r[5] = (vectorref((r[3]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("l")));
    *--hp = (mksymbol(internsym("bnd")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
  } else {
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("c")));
    *--hp = (mksymbol(internsym("bnd")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
  }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 168: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp ids k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(169);
    r[5] = (hendblk(2+1));
    r[0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 169: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k */
    r[4+0] = r[3];  
    r[4+1] = r[2];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v9521;

s_loop_v9521: /* k id r */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[1])));
    { /* assq */
    obj x = (r[3]), l = (r[2]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[4] = (l == mknull() ? obj_from_bool(0) : p); }
    r[3] = obj_from_bool(!bool_from_obj(r[4]));
  if (bool_from_obj(r[3])) {
    r[3] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9521;
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

case 170: /* clo k id&vu */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id&vu id */
    r[3] = (car((r[1])));
    r[3] = obj_from_bool((r[3]) == (r[2]));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 171: /* clo k exp vi */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k exp vi */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(172);
    r[3] = (hendblk(2+1));
    r[4+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 172: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k vi */
    { fixnum_t v10761_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10761_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v10761_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[1];   t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[4] = (o); } }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 173: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(173);
    r[2] = (hendblk(0+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[1]), (2)));
    r[5] = (vectorref((r[1]), (3)));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[6];  
    /* r[2] */    
    goto s_loop_v9461;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[1]), (3)));
  if (((r[3]) == (mksymbol(internsym("no-effect"))))) {
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v9450;
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
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[4]), (2)));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(180);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9439;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[1]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    goto s_loop_v9422;
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
  }
  }

case 174: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v9461: /* k id fl-simple? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(174);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(175);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 175: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 176: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v9450: /* k id fl-simple? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(176);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(177);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 177: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 178: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v9439: /* k id fl-simple? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(178);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(179);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 179: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 180: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r fl-simple? body k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 181: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v9422: /* k id fl-simple? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(181);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(182);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 182: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 183: /* box-exp k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (cx_the_2Dbox_2Dprim);
    *--hp = (mksymbol(internsym("*-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[2] = (hendblk(4+1)); }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 184: /* box-exp? k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (2)));
    r[2] = obj_from_bool((r[2]) == (cx_the_2Dbox_2Dprim));
  } else {
    r[2] = obj_from_bool(0);
  }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 185: /* boxref-exp k box */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k box */
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (cx_the_2Dboxref_2Dprim);
    *--hp = (mksymbol(internsym("?-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[2] = (hendblk(4+1)); }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 186: /* boxref-exp? k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (2)));
    r[2] = obj_from_bool((r[2]) == (cx_the_2Dboxref_2Dprim));
  } else {
    r[2] = obj_from_bool(0);
  }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 187: /* boxset-exp k box exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k box exp */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx_the_2Dboxset_2Dprim);
    *--hp = (mksymbol(internsym("!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(4+1)); }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 188: /* boxset-exp? k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (2)));
    r[2] = obj_from_bool((r[2]) == (cx_the_2Dboxset_2Dprim));
  } else {
    r[2] = obj_from_bool(0);
  }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 189: /* remove-assignments k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    r[2+0] = r[0];  
    r[2+1] = r[1];  
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_box_2Dsubst;

case 190: /* clo k exp substs */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
s_box_2Dsubst: /* k exp substs */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(190);
    r[3] = (hendblk(0+1));
    r[4+0] = r[0];  
    r[4+1] = r[1];  
    r[4+2] = r[3];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_box;

case 191: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_box: /* k exp box-subst substs */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(191);
    r[4] = (hendblk(2+1));
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_231935((r[1])))) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = obj_from_case(192);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    goto s_box;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    { /* assq */
    obj x = (r[5]), l = (r[3]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[6] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[6])) {
    r[7] = (cdr((r[6])));
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx_the_2Dboxref_2Dprim);
    *--hp = (mksymbol(internsym("?-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
  } else {
    r[5] = r[1];  
  }
    r[6+0] = r[0];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("varassign-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    { /* assq */
    obj x = (r[5]), l = (r[3]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[7] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[7])) {
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[0];  
    *--hp = obj_from_case(193);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    goto s_box;
  } else {
    r[8+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[0];  
    r[8+2] = (cx__231894);
    r[8+3] = (mknull());
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    r[7] = (vectorref((r[1]), (3)));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = obj_from_case(194);
    r[8] = (hendblk(4+1));
    r[0] = r[8];  
    r[1] = r[7];  
    /* r[2] */    
    /* r[3] */    
    goto s_box;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231106((r[1])))) {
    r[5] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083((r[1])));
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_box;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    r[7] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[0];  
    *--hp = obj_from_case(199);
    r[8] = (hendblk(3+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v9317;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(202);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    r[2] = r[4];  
    goto s_loop_v9294;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    r[7] = (vectorref((r[1]), (3)));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_case(204);
    r[8] = (hendblk(4+1));
    r[0] = r[8];  
    r[1] = r[7];  
    /* r[2] */    
    /* r[3] */    
    goto s_box;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    r[7+0] = r[0];  
    r[7+1] = r[5];  
    r[7+2] = (mknull());
    r[7+3] = (mknull());
    r[7+4] = r[3];  
    r[7+5] = r[2];  
    r[7+6] = r[6];  
    r[7+7] = r[5];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v9218;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(213);
    r[7] = (hendblk(6+1));
    r[8+0] = (cx_var_2Dassigned_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[5];  
    r[8+3] = r[6];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(218);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    goto s_box;
  } else {
    r[5] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[5]))));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_case(220);
    r[6] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    r[3] = (cx__23657);
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__23653);
    r[3] = obj_from_bool(1);
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
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

case 192: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 193: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k tmp */
    r[4] = (cdr((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx_the_2Dboxset_2Dprim);
    *--hp = (mksymbol(internsym("!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 194: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp box test-exp k */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(195);
    r[6] = (hendblk(4+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 195: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r box test-exp k r */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(196);
    r[6] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 196: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("if-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v9317: /* k id box */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(197);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9317;
  }

case 197: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r box id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(198);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 198: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 199: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k prim effect */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v9294: /* k id box */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(200);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9294;
  }

case 200: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r box id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(201);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 201: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 202: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r box rator k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(203);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 203: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 204: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r box lams k ids */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(207);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9261;

s_loop_v9261: /* k id box */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(205);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9261;
  }

case 205: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r box id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(206);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 206: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 207: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r ids */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("fix-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 208: /* clo k in-ids let-ids let-rands substs */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3]; }
    r += 1; /* shift reg. wnd */
s_loop_v9218: /* k in-ids let-ids let-rands substs box-subst body ids */
  if ((isnull((r[1])))) {
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = r[0];  
    *--hp = obj_from_case(209);
    r[8] = (hendblk(4+1));
    r[9+0] = r[5];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[6];  
    r[9+3] = r[4];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[8] = (car((r[1])));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(208);
    r[9] = (hendblk(3+1));
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[9];  
    *--hp = obj_from_case(210);
    r[9] = (hendblk(7+1));
    r[10+0] = (cx_var_2Dassigned_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[9];  
    r[10+2] = r[8];  
    r[10+3] = r[6];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 209: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k ids let-rands let-ids */
    { fixnum_t v10759_tmp;
    { /* length */
    int n; obj l = r[5];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10759_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v10759_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[5];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[6] = (o); } }
    { fixnum_t v10760_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10760_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v10760_tmp);
    hreserve(hbsz(3)*c, 7); /* 7 live regs */
    l = r[4];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[7] = (o); } }
    { /* vector */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[9] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 210: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop substs let-rands id let-ids in-ids k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(211);
    r[9] = (hendblk(7+1));
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__231687);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11] = (cdr((r[5])));
    r[11] = (car((r[11])));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[0] = (cx_symbol_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = (cdr((r[7])));
    r[9+3] = r[6];  
    r[9+4] = r[4];  
    r[9+5] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 211: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop substs let-rands id let-ids in-ids k */
    hreserve(hbsz(8+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(212);
    r[9] = (hendblk(8+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 212: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop substs let-rands id let-ids in-ids k r */
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(2+1), 12); /* 12 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[12] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (mknull());
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (cx_the_2Dbox_2Dprim);
    *--hp = (mksymbol(internsym("*-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[12] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = r[4];  
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = (r[10]);
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[13] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[3];  
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14+0] = r[2];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = r[8];  
    r[14+2] = (cdr((r[7])));
    r[14+3] = (r[11]);
    r[14+4] = (r[12]);
    r[14+5] = (r[13]);
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 213: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r box box-subst substs body k id */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(214);
    r[8] = (hendblk(5+1));
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__231687);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10] = (cdr((r[7])));
    r[10] = (car((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = (cx_symbol_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(217);
    r[8] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 214: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r box-subst substs body k id */
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(215);
    r[7] = (hendblk(6+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 215: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r box-subst substs body k id r */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(216);
    r[9] = (hendblk(3+1));
    { /* vector */
    hreserve(hbsz(2+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[10] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[3];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[4];  
    r[3] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 216: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[6] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx_the_2Dbox_2Dprim);
    *--hp = (mksymbol(internsym("*-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[6] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("letcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 217: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("letcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 218: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r box cont-exp k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(219);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 219: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("withcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 220: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(221);
    r[5] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[3];  
    r[7+3] = (cx__23651);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 221: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(222);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23647);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 222: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 223: /* analyze-globals k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(224);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(1), 3); /* 3 live regs */
    *--hp = obj_from_void(0);
    r[3] = (hendblk(1));
    (void)(objptr_from_obj(r[3])[0] = (mknull()));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(260);
    r[4] = (hendblk(2+1));
    hreserve(hbsz(5+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(249);
    r[4] = (hendblk(5+1));
    r[5+0] = r[4];  
    r[5+1] = r[1];  
    r[5+2] = (mknull());
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_gls;

case 224: /* clo k exp substs */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k exp substs */
    /* r[0] */    
    /* r[1] */    
    /* r[2] */    
    goto s_gsub;

case 225: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_gsub: /* k exp substs */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(225);
    r[3] = (hendblk(1+1));
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_231935((r[1])))) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
    { /* assq */
    obj x = (r[4]), l = (r[2]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[6] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[6])) {
    r[7] = (cdr((r[6])));
    r[7] = (cdr((r[7])));
    r[7] = (cdr((r[7])));
    r[7] = obj_from_bool(isnull((r[7])));
  if (bool_from_obj(r[7])) {
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[0];  
    *--hp = obj_from_case(226);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_gsub;
  } else {
    r[7] = (cdr((r[6])));
    r[7] = (cdr((r[7])));
    r[7] = (cdr((r[7])));
    r[7] = (car((r[7])));
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = obj_from_case(227);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_gsub;
  }
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    { /* assq */
    obj x = (r[4]), l = (r[2]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[5] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[5])) {
    r[6] = (cdr((r[5])));
    r[6] = (cdr((r[6])));
    r[4] = (car((r[6])));
  } else {
    r[4] = r[1];  
  }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (3)));
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(228);
    r[7] = (hendblk(4+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    goto s_gsub;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231106((r[1])))) {
    r[4] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083((r[1])));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_gsub;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = obj_from_case(233);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    r[2] = r[3];  
    goto s_loop_v9093;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(236);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[5];  
    r[2] = r[3];  
    goto s_loop_v9070;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (3)));
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_case(238);
    r[7] = (hendblk(4+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    goto s_gsub;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = obj_from_case(242);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_gsub;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = obj_from_case(243);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_gsub;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(244);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_gsub;
  } else {
    r[4] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[4]))));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_case(246);
    r[5] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    r[3] = (cx__23657);
    r[4] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cx__23653);
    r[3] = obj_from_bool(1);
    /* r[4] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
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

case 226: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k tmp */
    r[4] = (cdr((r[3])));
    r[4] = (car((r[4])));
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 227: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 228: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp gsub test-exp k */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(229);
    r[6] = (hendblk(4+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 229: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r gsub test-exp k r */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(230);
    r[6] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 230: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("if-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v9093: /* k id gsub */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(231);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9093;
  }

case 231: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r gsub id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(232);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 232: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 233: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k prim effect */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v9070: /* k id gsub */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(234);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9070;
  }

case 234: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r gsub id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(235);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 235: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 236: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r gsub rator k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(237);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 237: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 238: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r gsub lams k ids */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(241);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9037;

s_loop_v9037: /* k id gsub */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(239);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v9037;
  }

case 239: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r gsub id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(240);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 240: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 241: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r ids */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("fix-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 242: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k ids */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 243: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("letcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 244: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r gsub cont-exp k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(245);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 245: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("withcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 246: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(247);
    r[5] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[3];  
    r[7+3] = (cx__23651);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 247: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(248);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23647);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 248: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 249: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r finalize-substs ag-subst exp k stack-function-candidates */
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(250);
    r[7] = (hendblk(4+1));
    r[8+0] = r[2];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 250: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r ag-subst exp k stack-function-candidates */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(251);
    r[6] = (hendblk(2+1));
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[3];  
    r[7+3] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 251: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k stack-function-candidates */
    r[4] = (objptr_from_obj(r[3])[0]);
    { fixnum_t v10758_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10758_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v10758_tmp);
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    l = r[4];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[4] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 252: /* clo k exp substs */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_gls: /* k exp substs exp */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(252);
    r[4] = (hendblk(1+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_231935((r[1])))) {
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (1)));
  if (bool_from_obj(cxs_global_2Did_2Dprivate_2Dconstant_3F_232010((r[6])))) {
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[0];  
    *--hp = obj_from_case(253);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_gls;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(254);
    r[7] = (hendblk(5+1));
    r[8+0] = (cx_var_2Dassignment_2Dcount);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[6];  
    r[8+3] = r[3];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("varassign-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (2)));
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_gls;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_gls;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[5] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[5]), (2)));
    r[6] = (vectorref((r[1]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8+0] = (cx_reduce_2Dleft);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[0];  
    r[8+2] = r[4];  
    r[8+3] = r[2];  
    r[8+4] = r[7];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (3)));
    r[6+0] = (cx_reduce_2Dleft);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[0];  
    r[6+2] = r[4];  
    r[6+3] = r[2];  
    r[6+4] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (1)));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8+0] = (cx_reduce_2Dleft);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[0];  
    r[8+2] = r[4];  
    r[8+3] = r[2];  
    r[8+4] = r[7];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (3)));
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_gls;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (2)));
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_gls;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (1)));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8+0] = (cx_reduce_2Dleft);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[0];  
    r[8+2] = r[4];  
    r[8+3] = r[2];  
    r[8+4] = r[7];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[5] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[5]))));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_case(257);
    r[6] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    r[3] = (cx__23657);
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__23653);
    r[3] = obj_from_bool(1);
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
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

case 253: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id exp */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 254: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r gls substs k exp id */
  if ((fixnum_from_obj(r[1]) == (1))) {
    r[7] = (cdr((r[6])));
    r[7] = (car((r[7])));
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(255);
    r[8] = (hendblk(6+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 255: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r gls substs k id exp sym */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(1);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = obj_from_fixnum(-fixnum_from_obj(r[1]));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(256);
    r[9] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[6];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 256: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id r exp */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 257: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(258);
    r[5] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[3];  
    r[7+3] = (cx__23651);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 258: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(259);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23647);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 259: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 260: /* clo k substs */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k substs stack-function-candidates ag-subst */
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(277);
    r[4] = (hendblk(0+1));
    hreserve(hbsz(0+1), 5); /* 5 live regs */
    *--hp = obj_from_case(299);
    r[5] = (hendblk(0+1));
    hreserve(hbsz(0+1), 6); /* 6 live regs */
    *--hp = obj_from_case(317);
    r[6] = (hendblk(0+1));
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(263);
    r[4] = (hendblk(5+1));
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(261);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_reduce_2Dleft);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = r[4];  
    r[5+3] = (mknull());
    r[5+4] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 261: /* clo k id&cid&exp substs */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id&cid&exp substs finalize */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(262);
    r[4] = (hendblk(2+1));
    r[5+0] = r[3];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r[5+3] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 262: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k substs */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 263: /* clo k id&cid&exp substs */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4];
    r[1+7] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* k id&cid&exp substs ag-simple? ag-inlineable? ag-stack-function? stack-function-candidates ag-subst */
    r[8] = (car((r[1])));
    r[9] = (cdr((r[1])));
    r[9] = (car((r[9])));
    r[10] = (cdr((r[1])));
    r[10] = (cdr((r[10])));
    r[10] = (car((r[10])));
  if (bool_from_obj(cxs_rec_2Dexp_3F_232149((r[10])))) {
    hreserve(hbsz(5+1), 11); /* 11 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(263);
    r[11] = (hendblk(5+1));
    hreserve(hbsz(5+1), 12); /* 12 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = (r[11]);
    *--hp = obj_from_case(264);
    r[11] = (hendblk(5+1));
    r[12] = (vectorref((r[10]), (2)));
    r[12] = (car((r[12])));
    { /* vector */
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[13] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (mknull());
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14] = (vectorref((r[10]), (1)));
    r[14] = (car((r[14])));
    { /* cons */ 
    hreserve(hbsz(3), 15); /* 15 live regs */
    *--hp = (r[13]);
    *--hp = (r[14]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (mknull());
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14+0] = r[7];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[11]);
    r[14+2] = (r[12]);
    r[14+3] = (r[13]);
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = r[2];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = obj_from_case(266);
    r[11] = (hendblk(8+1));
    hreserve(hbsz(7+1), 12); /* 12 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[11]);
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = r[2];  
    *--hp = r[7];  
    *--hp = obj_from_case(274);
    r[11] = (hendblk(7+1));
    r[12+0] = r[3];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = (r[10]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 264: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r finalize substs k id cid */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(265);
    r[7] = (hendblk(3+1));
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[7];  
    r[9+2] = r[8];  
    r[9+3] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 265: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id cid */
    r[5] = (cdr((r[1])));
    r[5] = (cdr((r[5])));
    r[5] = (cdr((r[5])));
    r[5] = obj_from_bool(isnull((r[5])));
  if (bool_from_obj(r[5])) {
    { /* vector */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
  } else {
    r[5] = r[1];  
  }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 266: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r ag-stack-function? ag-subst substs exp cid id stack-function-candidates k */
  if (bool_from_obj(r[1])) {
    r[10+0] = r[9];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = obj_from_ktrap();
    r[10+2] = r[1];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(267);
    r[10] = (hendblk(7+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[5];  
    r[3] = r[7];  
    /* r[4] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }

case 267: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r ag-subst substs exp k cid id stack-function-candidates */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(268);
    r[9] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    { /* vector */
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[9] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 268: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k cid id stack-function-candidates */
    r[6] = (vectorref((r[1]), (1)));
    r[7] = (objptr_from_obj(r[5])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    (void)(objptr_from_obj(r[5])[0] = (r[7]));
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(270);
    r[7] = (hendblk(4+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto s_loop_v8824;

s_loop_v8824: /* k ids */
  if ((ispair((r[1])))) {
    r[2] = (cdr((r[1])));
    r[2] = obj_from_bool(ispair((r[2])));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(269);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8824;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cx__232045);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cx__232044);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 269: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* string-append */
    int *d = stringcat(stringdata((cx__232047)), stringdata((r[1])));
    r[3] = (hpushstr(3, d)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 270: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id cid ids */
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(271);
    r[6] = (hendblk(5+1));
    r[7] = (cdr((r[3])));
    r[7] = (car((r[7])));
    r[7] = (hpushstr(8, newstring(symbolname(getsymbol((r[7]))))));
    r[0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[7];  
    r[3] = (cx__232063);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 271: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id cid ids r */
    { /* string-append */
    int *d = stringcat(stringdata((r[6])), stringdata((cx__232057)));
    r[7] = (hpushstr(7, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232060)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232071)), stringdata((r[7])));
    r[8] = (hpushstr(8, d)); }
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(273);
    r[9] = (hendblk(5+1));
    r[0] = r[9];  
    r[1] = r[5];  
    goto s_loop_v8805;

s_loop_v8805: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(272);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8805;
  }

case 272: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    { /* vector */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 273: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id cid ids r */
    { /* vector */
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("?!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(4+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
  if (bool_from_obj(cxs_global_2Did_2Dprivate_2Dconstant_3F_232010((r[4])))) {
    { /* vector */
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__2369);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[9] = (hendblk(4+1)); }
  } else {
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[9] = (hendblk(3+1)); }
  }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 274: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r ag-subst substs exp ag-inlineable? k id cid */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(275);
    r[9] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 275: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r ag-inlineable? k id cid */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(276);
    r[6] = (hendblk(4+1));
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 276: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id r cid */
  if (bool_from_obj(r[1])) {
  if (bool_from_obj(cxs_global_2Did_2Dprivate_2Dconstant_3F_232010((r[5])))) {
    { /* vector */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = (cx__2369);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[6] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
  }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 277: /* clo k exp self-id substs */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
s_ag_2Dstack_2Dfunction_3F: /* k exp self-id substs */
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(298);
    r[4] = (hendblk(0+1));
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231106((r[1])))) {
    r[5] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083((r[1])));
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_ag_2Dstack_2Dfunction_3F;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(278);
    r[7] = (hendblk(4+1));
    r[0] = r[7];  
    r[1] = r[5];  
    r[2] = r[3];  
    goto s_subst_2Dlocals;
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

case 278: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r self-id subst-locals body k */
    r[6+0] = r[5];  
    r[6+1] = r[4];  
    r[6+2] = r[1];  
    r[6+3] = (mknull());
    r[6+4] = obj_from_bool(1);
    r[6+5] = r[2];  
    r[6+6] = r[3];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_stack_2Dfn_3F;

case 279: /* clo k exp substs labels tail? */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2]; }
    r += 1; /* shift reg. wnd */
s_stack_2Dfn_3F: /* k exp substs labels tail? self-id subst-locals */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(279);
    r[7] = (hendblk(2+1));
    r[8+0] = r[0];  
    r[8+1] = r[1];  
    r[8+2] = r[5];  
    r[8+3] = r[6];  
    r[8+4] = r[4];  
    r[8+5] = r[7];  
    r[8+6] = r[3];  
    r[8+7] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_sfn_3F;

case 280: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
s_sfn_3F: /* k exp self-id subst-locals tail? stack-fn? labels substs */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(280);
    r[8] = (hendblk(6+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[9] = (vectorref((r[1]), (0)));
    r[9] = obj_from_bool((r[9]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[9] = (vectorref((r[1]), (1)));
    { /* assq */
    obj x = (r[9]), l = (r[7]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[10] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[10])) {
    r[11] = (cdr((r[10])));
    r[11] = (cdr((r[11])));
    r[11] = obj_from_bool(isnull((r[11])));
  if (bool_from_obj(r[11])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[12] = (cdr((r[10])));
    r[12] = (cdr((r[12])));
    r[12] = (car((r[12])));
  if ((isvector((r[12])))) {
  if (((vectorlen((r[12]))) == (2))) {
    r[13] = (vectorref((r[12]), (0)));
    r[13] = obj_from_bool((r[13]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[13] = obj_from_bool(0);
  }
  } else {
    r[13] = obj_from_bool(0);
  }
  if (bool_from_obj(r[13])) {
    r[13] = (vectorref((r[12]), (1)));
    r[13] = obj_from_bool((r[9]) == (r[13]));
  } else {
    r[13] = obj_from_bool(0);
  }
  if (bool_from_obj(r[13])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    /* r[0] */    
    r[1] = (r[12]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    goto s_sfn_3F;
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
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[9] = (vectorref((r[1]), (0)));
    r[9] = obj_from_bool((r[9]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[9] = (vectorref((r[1]), (1)));
    r[10] = (vectorref((r[1]), (2)));
    r[11] = (vectorref((r[1]), (3)));
    hreserve(hbsz(4+1), 12); /* 12 live regs */
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[8];  
    *--hp = obj_from_case(281);
    r[12] = (hendblk(4+1));
    r[13+0] = r[5];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = (r[12]);
    r[13+2] = r[9];  
    r[13+3] = r[7];  
    r[13+4] = r[6];  
    r[13+5] = obj_from_bool(0);
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[9] = (vectorref((r[1]), (1)));
    r[9] = (vectorref((r[9]), (1)));
    r[10] = (vectorref((r[1]), (2)));
    r[11] = (vectorref((r[1]), (1)));
    r[11] = (vectorref((r[11]), (2)));
    hreserve(hbsz(8+1), 12); /* 12 live regs */
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_case(286);
    r[12] = (hendblk(8+1));
    r[0] = (r[12]);
    r[1] = (r[10]);
    r[2] = r[5];  
    r[3] = r[6];  
    r[4] = r[7];  
    goto s_loop_v8702;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[9] = (vectorref((r[1]), (0)));
    r[9] = obj_from_bool((r[9]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[9] = (vectorref((r[1]), (1)));
    r[10] = (vectorref((r[1]), (3)));
    { /* memq */
    obj x = (r[9]), l = (cx__232891);
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[11] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[11])) {
    /* r[0] */    
    r[1] = (r[10]);
    r[2] = r[5];  
    r[3] = r[6];  
    r[4] = r[7];  
    goto s_loop_v8679;
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
  if (bool_from_obj(cxs_loop_2Dexp_3F_232853((r[1])))) {
    r[9] = (vectorref((r[1]), (1)));
    r[10] = (vectorref((r[9]), (1)));
    r[9] = (car((r[10])));
    r[10] = (vectorref((r[1]), (1)));
    r[11] = (vectorref((r[10]), (2)));
    r[10] = (car((r[11])));
    r[11] = (vectorref((r[1]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (mknull());
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (mknull());
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(2+1), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[14] = (hendblk(2+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 15); /* 15 live regs */
    *--hp = (r[11]);
    *--hp = (r[14]);
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[14] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(4+1), 15); /* 15 live regs */
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (mksymbol(internsym("fix-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[12] = (hendblk(4+1)); }
    /* r[0] */    
    r[1] = (r[12]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    goto s_sfn_3F;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[9] = (vectorref((r[1]), (0)));
    r[9] = obj_from_bool((r[9]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[9] = (vectorref((r[1]), (1)));
    r[10] = (vectorref((r[1]), (2)));
    r[11] = (vectorref((r[1]), (3)));
    { fixnum_t v10757_tmp;
    { /* length */
    int n; obj l = r[9];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10757_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v10757_tmp);
    hreserve(hbsz(3)*c, 12); /* 12 live regs */
    l = r[9];   t = r[6];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[12] = (o); } }
    hreserve(hbsz(6+1), 13); /* 13 live regs */
    *--hp = r[0];  
    *--hp = (r[10]);
    *--hp = (r[12]);
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_case(290);
    r[13] = (hendblk(6+1));
    r[14+0] = r[5];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[13]);
    r[14+2] = (r[11]);
    r[14+3] = r[7];  
    r[14+4] = (r[12]);
    r[14+5] = r[4];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[9] = (vectorref((r[1]), (0)));
    r[9] = obj_from_bool((r[9]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[9] = (vectorref((r[1]), (1)));
    r[10] = (vectorref((r[1]), (2)));
  if ((isvector((r[9])))) {
  if (((vectorlen((r[9]))) == (2))) {
    r[11] = (vectorref((r[9]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[9]), (1)));
  if (((r[11]) == (r[2]))) {
    /* r[0] */    
    r[1] = (r[10]);
    r[2] = r[5];  
    r[3] = r[6];  
    r[4] = r[7];  
    goto s_loop_v8645;
  } else {
  if (bool_from_obj(r[4])) {
    { /* memq */
    obj x = (r[11]), l = r[6];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[12] = (l == mknull() ? obj_from_bool(0) : l); }
  } else {
    r[12] = obj_from_bool(0);
  }
  if (bool_from_obj(r[12])) {
    /* r[0] */    
    r[1] = (r[10]);
    r[2] = r[5];  
    r[3] = r[6];  
    r[4] = r[7];  
    goto s_loop_v8634;
  } else {
    { /* assq */
    obj x = (r[11]), l = (r[7]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[12] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[12])) {
    { bool_t v10756_tmp;
    r[13] = (cdr((r[12])));
    r[13] = (cdr((r[13])));
    v10756_tmp = (isnull((r[13])));
    r[13] = obj_from_bool(!(v10756_tmp)); }
  if (bool_from_obj(r[13])) {
    r[13] = (car((r[12])));
    r[14] = (cdr((r[12])));
    r[14] = (cdr((r[14])));
    r[14] = (car((r[14])));
    { bool_t v10754_tmp;
  if ((isvector((r[14])))) {
  if (((vectorlen((r[14]))) == (2))) {
    r[15] = (vectorref((r[14]), (0)));
    v10754_tmp = ((r[15]) == (mksymbol(internsym("var-exp"))));
  } else {
    v10754_tmp = (0);
  }
  } else {
    v10754_tmp = (0);
  }
    r[15] = obj_from_bool(!(v10754_tmp)); }
  if (bool_from_obj(r[15])) {
    r[15] = (r[15]);
  } else {
    { bool_t v10755_tmp;
    r[16] = (vectorref((r[14]), (1)));
    v10755_tmp = ((r[13]) == (r[16]));
    r[15] = obj_from_bool(!(v10755_tmp)); }
  }
  if (bool_from_obj(r[15])) {
    { /* vector */
    hreserve(hbsz(3+1), 15); /* 15 live regs */
    *--hp = (r[10]);
    *--hp = (r[14]);
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[15] = (hendblk(3+1)); }
    /* r[0] */    
    r[1] = (r[15]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    goto s_sfn_3F;
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
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
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
  }
  }
  }

case 281: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r sfn? then-exp else-exp k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[5];  
    r[1] = r[6];  
    /* r[2] */    
    goto s_loop_v8715;
  } else {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 282: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v8715: /* k id sfn? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(282);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(283);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 283: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 284: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_loop_v8702: /* k id stack-fn? labels substs */
  if ((isnull((r[1])))) {
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = obj_from_bool(isnull((r[1])));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(284);
    r[5] = (hendblk(3+1));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(285);
    r[5] = (hendblk(3+1));
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[1])));
    r[6+3] = r[4];  
    r[6+4] = r[3];  
    r[6+5] = obj_from_bool(0);
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 285: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 286: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r subst-locals substs ids stack-fn? tail? labels body k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(287);
    r[10] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[9];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 287: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r stack-fn? tail? labels body k */
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[5];  
    r[7+3] = r[1];  
    r[7+4] = r[4];  
    r[7+5] = r[3];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 288: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_loop_v8679: /* k id stack-fn? labels substs */
  if ((isnull((r[1])))) {
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = obj_from_bool(isnull((r[1])));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(288);
    r[5] = (hendblk(3+1));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(289);
    r[5] = (hendblk(3+1));
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[1])));
    r[6+3] = r[4];  
    r[6+4] = r[3];  
    r[6+5] = obj_from_bool(0);
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 289: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 290: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r subst-locals substs stack-fn? labels lams k */
  if (bool_from_obj(r[1])) {
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v8658;
  } else {
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 291: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
s_loop_v8658: /* k id subst-locals substs stack-fn? labels */
  if ((isnull((r[1])))) {
    r[6+0] = r[0];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = obj_from_bool(isnull((r[1])));
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (car((r[1])));
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(291);
    r[7] = (hendblk(4+1));
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(292);
    r[7] = (hendblk(6+1));
    r[8+0] = r[2];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = (vectorref((r[6]), (1)));
    r[8+3] = r[3];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 292: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r stack-fn? labels lam loop id k */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(293);
    r[8] = (hendblk(3+1));
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = (vectorref((r[4]), (2)));
    r[9+3] = r[1];  
    r[9+4] = r[3];  
    r[9+5] = obj_from_bool(1);
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 293: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 294: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_loop_v8645: /* k id stack-fn? labels substs */
  if ((isnull((r[1])))) {
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = obj_from_bool(isnull((r[1])));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(294);
    r[5] = (hendblk(3+1));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(295);
    r[5] = (hendblk(3+1));
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[1])));
    r[6+3] = r[4];  
    r[6+4] = r[3];  
    r[6+5] = obj_from_bool(0);
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 295: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 296: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_loop_v8634: /* k id stack-fn? labels substs */
  if ((isnull((r[1])))) {
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = obj_from_bool(isnull((r[1])));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(296);
    r[5] = (hendblk(3+1));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(297);
    r[5] = (hendblk(3+1));
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[1])));
    r[6+3] = r[4];  
    r[6+4] = r[3];  
    r[6+5] = obj_from_bool(0);
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 297: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 298: /* clo k ids substs */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
s_subst_2Dlocals: /* k ids substs */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (car((r[1])));
    r[5] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_subst_2Dlocals;
  }

case 299: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_ag_2Dinlineable_3F: /* k exp */
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231106((r[1])))) {
    r[2] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083((r[1])));
    /* r[0] */    
    r[1] = r[2];  
    goto s_ag_2Dinlineable_3F;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (3)));
    r[2] = obj_from_bool(((r[2]) == (mksymbol(internsym("no-effect")))) && (isnull((r[3]))));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(303);
    r[4] = (hendblk(0+1));
    r[5+0] = r[0];  
    r[5+1] = r[3];  
    r[5+2] = (mknull());
    r[5+3] = r[4];  
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_inlineable_3F;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }

case 300: /* clo k body substs */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
s_inlineable_3F: /* k body substs ag-inlineable-app-rands? ids */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(300);
    r[5] = (hendblk(2+1));
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231106((r[1])))) {
    r[6] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083((r[1])));
    /* r[0] */    
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_inlineable_3F;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[6] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[6]), (1)));
    r[7] = (vectorref((r[1]), (2)));
    r[8] = (vectorref((r[1]), (1)));
    r[8] = (vectorref((r[8]), (2)));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[0];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = obj_from_case(302);
    r[9] = (hendblk(3+1));
    r[0] = r[9];  
    r[1] = r[6];  
    r[2] = r[7];  
    goto s_loop_v8588;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[6] = (vectorref((r[1]), (0)));
    r[6] = obj_from_bool((r[6]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[6] = obj_from_bool(0);
  }
  } else {
    r[6] = obj_from_bool(0);
  }
  if (bool_from_obj(r[6])) {
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[0];  
    r[6+2] = (vectorref((r[1]), (3)));
    r[6+3] = r[2];  
    r[6+4] = r[4];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
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

s_loop_v8588: /* k id id */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(301);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v8588;
  }

case 301: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id id */
    r[5] = (car((r[4])));
    r[6] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 302: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r inlineable? body k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 303: /* clo k rands substs lambda-ids */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k rands substs lambda-ids */
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(314);
    r[4] = (hendblk(0+1));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(306);
    r[4] = (hendblk(3+1));
    hreserve(hbsz(0+1), 5); /* 5 live regs */
    *--hp = obj_from_case(313);
    r[5] = (hendblk(0+1));
    r[6+0] = r[4];  
    r[6+1] = r[1];  
    r[6+2] = r[5];  
    r[6+3] = r[2];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v8545;

s_loop_v8545: /* k id ag-lookup substs */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(304);
    r[5] = (hendblk(4+1));
    r[0] = r[5];  
    r[1] = r[4];  
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v8545;
  }

case 304: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r ag-lookup substs id k */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(305);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (car((r[4])));
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 305: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 306: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ag-constant? lambda-ids k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(307);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(311);
    r[6] = (hendblk(1+1));
    r[7+0] = r[5];  
    r[7+1] = r[6];  
    r[7+2] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_keep_23531;

case 307: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r lambda-ids k */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(308);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    /* r[1] */    
    goto s_loop_v8532;

s_loop_v8532: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isnull((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (2))) {
    r[3] = (vectorref((r[2]), (0)));
    r[2] = obj_from_bool((r[3]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[2];  
    goto s_loop_v8532;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 308: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  r lambda-ids k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(310);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v8517;

s_loop_v8517: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(309);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8517;
  }

case 309: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (1)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 310: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r lambda-ids k */
    r[0] = r[3];  
    /* r[1] */    
    /* r[2] */    
    goto s_loop_v8508;

s_loop_v8508: /* k aids lambda-ids */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[1])));
    r[4] = (cdr((r[1])));
    { /* memq */
    obj x = (r[3]), l = r[2];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[5] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[5])) {
    { /* memq */
    obj x = (r[3]), l = r[4];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[5] = (l == mknull() ? obj_from_bool(0) : l); }
    r[5] = obj_from_bool(!bool_from_obj(r[5]));
  if (bool_from_obj(r[5])) {
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v8508;
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
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 311: /* clo k x */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k x ag-constant? */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(312);
    r[3] = (hendblk(1+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 312: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = r[2];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(!bool_from_obj(r[1]));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 313: /* clo k exp substs */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
s_ag_2Dlookup: /* k exp substs */
    { bool_t v10753_tmp;
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[3] = (vectorref((r[1]), (0)));
    v10753_tmp = ((r[3]) == (mksymbol(internsym("var-exp"))));
  } else {
    v10753_tmp = (0);
  }
  } else {
    v10753_tmp = (0);
  }
    r[3] = obj_from_bool(!(v10753_tmp)); }
  if (bool_from_obj(r[3])) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isnull((r[2])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (car((r[2])));
    r[4] = (car((r[4])));
    r[3] = obj_from_bool((r[3]) == (r[4]));
  if (bool_from_obj(r[3])) {
    r[3] = (car((r[2])));
    r[3] = (cdr((r[3])));
    r[4] = (cdr((r[2])));
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_ag_2Dlookup;
  } else {
    r[3] = (cdr((r[2])));
    /* r[0] */    
    /* r[1] */    
    r[2] = r[3];  
    goto s_ag_2Dlookup;
  }
  }
  }

case 314: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (3)));
  if (((r[2]) == (mksymbol(internsym("no-effect"))))) {
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(314);
    r[4] = (hendblk(0+1));
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v8439;
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
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(1);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 315: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v8439: /* k id ag-constant? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(315);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(316);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 316: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 317: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_ag_2Dsimple_3F: /* k exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(317);
    r[2] = (hendblk(0+1));
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231106((r[1])))) {
    r[3] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083((r[1])));
    /* r[0] */    
    r[1] = r[3];  
    goto s_ag_2Dsimple_3F;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (3)));
    /* r[0] */    
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v8409;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(320);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    goto s_ag_2Dsimple_3F;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (2)));
    /* r[0] */    
    r[1] = r[3];  
    goto s_ag_2Dsimple_3F;
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
  }

case 318: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v8409: /* k id ag-simple? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(318);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(319);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 319: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 320: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ag-simple? rands k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v8394;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 321: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v8394: /* k id ag-simple? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(321);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(322);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 322: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 323: /* constant-fold k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    r[2+0] = r[0];  
    r[2+1] = r[1];  
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_const_2Dfold;

case 324: /* clo k exp env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
s_const_2Dfold: /* k exp env */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(324);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(368);
    r[4] = (hendblk(0+1));
    r[5+0] = r[0];  
    r[5+1] = r[1];  
    r[5+2] = r[3];  
    r[5+3] = r[4];  
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_cf;

case 325: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_cf: /* k exp const-fold fold-typecheck-ctype env */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(325);
    r[5] = (hendblk(3+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[6] = (vectorref((r[1]), (0)));
    r[6] = obj_from_bool((r[6]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[6] = obj_from_bool(0);
  }
  } else {
    r[6] = obj_from_bool(0);
  }
  if (bool_from_obj(r[6])) {
    r[6+0] = r[0];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_231935((r[1])))) {
    r[6] = (vectorref((r[1]), (1)));
    r[7] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[0];  
    *--hp = obj_from_case(326);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[7];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_cf;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[6] = (vectorref((r[1]), (0)));
    r[6] = obj_from_bool((r[6]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[6] = obj_from_bool(0);
  }
  } else {
    r[6] = obj_from_bool(0);
  }
  if (bool_from_obj(r[6])) {
    r[6] = (vectorref((r[1]), (1)));
    r[7] = (vectorref((r[1]), (2)));
    r[8] = (vectorref((r[1]), (3)));
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = obj_from_case(327);
    r[9] = (hendblk(6+1));
    r[0] = r[9];  
    r[1] = r[8];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_cf;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231106((r[1])))) {
    r[6] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083((r[1])));
    /* r[0] */    
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_cf;
  } else {
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(332);
    r[6] = (hendblk(5+1));
    r[0] = r[6];  
    /* r[1] */    
    goto gs_begin_2Dexp_3F_23185;
  }
  }
  }
  }

case 326: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 327: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp cf test-exp fold-typecheck-ctype env k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(328);
    r[8] = (hendblk(6+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 328: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r cf test-exp fold-typecheck-ctype env r k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(329);
    r[8] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 329: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r fold-typecheck-ctype env r r k */
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(330);
    r[7] = (hendblk(4+1));
    r[8+0] = r[2];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[1];  
    r[8+3] = r[3];  
    r[8+4] = r[5];  
    r[8+5] = r[4];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 330: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r k r */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(331);
    r[6] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  } else {
    { /* vector */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("if-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[6] = (hendblk(4+1)); }
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 331: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 332: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r env cf const-fold k exp */
  if (bool_from_obj(r[1])) {
    r[7] = (vectorref((r[6]), (2)));
    r[7] = (car((r[7])));
    r[8] = (vectorref((r[6]), (1)));
    r[8] = (vectorref((r[8]), (2)));
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(333);
    r[9] = (hendblk(5+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[6])))) {
    r[7] = (vectorref((r[6]), (1)));
    r[7] = (vectorref((r[7]), (1)));
    r[8] = (vectorref((r[6]), (2)));
    r[9] = (vectorref((r[6]), (1)));
    r[9] = (vectorref((r[9]), (2)));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[2];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = obj_from_case(345);
    r[10] = (hendblk(5+1));
    r[0] = (r[10]);
    r[1] = r[8];  
    r[2] = r[3];  
    goto s_loop_v8242;
  } else {
  if ((isvector((r[6])))) {
  if (((vectorlen((r[6]))) == (4))) {
    r[7] = (vectorref((r[6]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7] = (vectorref((r[6]), (1)));
    r[8] = (vectorref((r[6]), (2)));
    r[9] = (vectorref((r[6]), (3)));
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = obj_from_case(352);
    r[10] = (hendblk(3+1));
    r[0] = (r[10]);
    r[1] = r[9];  
    r[2] = r[3];  
    goto s_loop_v8188;
  } else {
  if ((isvector((r[6])))) {
  if (((vectorlen((r[6]))) == (3))) {
    r[7] = (vectorref((r[6]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7] = (vectorref((r[6]), (1)));
    r[8] = (vectorref((r[6]), (2)));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = obj_from_case(355);
    r[9] = (hendblk(3+1));
    r[0] = r[9];  
    r[1] = r[8];  
    r[2] = r[3];  
    goto s_loop_v8165;
  } else {
  if ((isvector((r[6])))) {
  if (((vectorlen((r[6]))) == (4))) {
    r[7] = (vectorref((r[6]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7] = (vectorref((r[6]), (1)));
    r[8] = (vectorref((r[6]), (2)));
    r[9] = (vectorref((r[6]), (3)));
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = obj_from_case(357);
    r[10] = (hendblk(4+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[6])))) {
  if (((vectorlen((r[6]))) == (3))) {
    r[7] = (vectorref((r[6]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7] = (vectorref((r[6]), (1)));
    r[8] = (vectorref((r[6]), (2)));
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = obj_from_case(361);
    r[9] = (hendblk(2+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[6])))) {
  if (((vectorlen((r[6]))) == (3))) {
    r[7] = (vectorref((r[6]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7] = (vectorref((r[6]), (1)));
    r[8] = (vectorref((r[6]), (2)));
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = obj_from_case(362);
    r[9] = (hendblk(2+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[6])))) {
  if (((vectorlen((r[6]))) == (3))) {
    r[7] = (vectorref((r[6]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7] = (vectorref((r[6]), (1)));
    r[8] = (vectorref((r[6]), (2)));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_case(363);
    r[9] = (hendblk(3+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[7] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[7]))));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(365);
    r[8] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[7];  
    r[3] = (cx__23657);
    r[4] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = (cx__23653);
    r[3] = obj_from_bool(1);
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
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

case 333: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r env cf const-fold exp2 k */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(334);
    r[7] = (hendblk(5+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[8] = (vectorref((r[1]), (0)));
    r[8] = obj_from_bool((r[8]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[8] = obj_from_bool(0);
  }
  } else {
    r[8] = obj_from_bool(0);
  }
  if (bool_from_obj(r[8])) {
    r[8] = (vectorref((r[1]), (3)));
    r[9] = (vectorref((r[1]), (2)));
    { fixnum_t v10752_tmp;
    { /* length */
    int n; obj l = r[8];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10752_tmp = (n); }
    r[10] = obj_from_bool((v10752_tmp) == (1)); }
  if (bool_from_obj(r[10])) {
    r[10] = (car((r[8])));
  if ((isvector((r[10])))) {
  if (((vectorlen((r[10]))) == (2))) {
    r[11] = (vectorref((r[10]), (0)));
    r[10] = obj_from_bool((r[11]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[10] = obj_from_bool(0);
  }
  } else {
    r[10] = obj_from_bool(0);
  }
  if (bool_from_obj(r[10])) {
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[2];  
    *--hp = r[7];  
    *--hp = obj_from_case(339);
    r[10] = (hendblk(3+1));
    r[0] = (cx_typeassert_2Dprim_2Dctype);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[10+0] = obj_from_ktrap();
    r[10+1] = obj_from_bool(0);
    r[10+2] = r[3];  
    r[10+3] = r[4];  
    r[10+4] = r[5];  
    r[10+5] = r[6];  
    r[10+6] = r[1];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8286;
  }
  } else {
    r[10+0] = obj_from_ktrap();
    r[10+1] = obj_from_bool(0);
    r[10+2] = r[3];  
    r[10+3] = r[4];  
    r[10+4] = r[5];  
    r[10+5] = r[6];  
    r[10+6] = r[1];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8286;
  }
  } else {
    r[8+0] = obj_from_ktrap();
    r[8+1] = obj_from_bool(0);
    r[8+2] = r[3];  
    r[8+3] = r[4];  
    r[8+4] = r[5];  
    r[8+5] = r[6];  
    r[8+6] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8286;
  }

case 334: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
s_l_v8286: /* ek r cf const-fold exp2 k r */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(335);
    r[7] = (hendblk(2+1));
    r[8+0] = r[3];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[4];  
    r[8+3] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(337);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 335: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(336);
    r[4] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 336: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 337: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(338);
    r[4] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 338: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 339: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k env rands */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(340);
    r[5] = (hendblk(4+1));
    r[0] = (cx_exp_2Dctype);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[4])));
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 340: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k env rands r */
  if (bool_from_obj(r[5])) {
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(341);
    r[6] = (hendblk(5+1));
    r[0] = (cx_member);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    r[3] = (cx__233677);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 341: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k env r rands */
  if ((!bool_from_obj(r[1]))) {
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(342);
    r[7] = (hendblk(4+1));
  if ((!bool_from_obj(r[2]))) {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(!bool_from_obj(r[2]));
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    goto s_l_v8268;
  } else {
    r[0] = (cx_member);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    /* r[2] */    
    r[3] = (cx__233677);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 342: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
s_l_v8268: /* ek r k env r rands */
  if (bool_from_obj(r[1])) {
    r[6] = (car((r[5])));
    r[6] = (vectorref((r[6]), (1)));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop_v8242: /* k id cf */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(343);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v8242;
  }

case 343: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cf id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(344);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 344: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 345: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r const-fold body k ids env */
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(348);
    r[7] = (hendblk(6+1));
    r[8+0] = r[7];  
    r[8+1] = r[5];  
    r[8+2] = r[1];  
    r[8+3] = r[6];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v8219;

s_loop_v8219: /* k id id env */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    r[5] = (cdr((r[2])));
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(346);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    goto s_loop_v8219;
  }

case 346: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r env k id id */
    r[6] = (car((r[5])));
    r[7] = (car((r[4])));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(347);
    r[8] = (hendblk(3+1));
    r[9+0] = (cx_exp_2Dctype);
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[7];  
    r[9+3] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 347: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r id */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 348: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r const-fold body k r ids env */
    { fixnum_t v10751_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10751_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v10751_tmp);
    hreserve(hbsz(3)*c, 8); /* 8 live regs */
    l = r[1];   t = r[7];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[8] = (o); } }
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(349);
    r[9] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[3];  
    r[3] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 349: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r ids */
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v8188: /* k id cf */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(350);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v8188;
  }

case 350: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cf id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(351);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 351: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 352: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k prim effect */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v8165: /* k id cf */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(353);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v8165;
  }

case 353: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cf id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(354);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 354: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 355: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r rator cf k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(356);
    r[5] = (hendblk(3+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 356: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cf k r */
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[5])))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 357: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cf lams k ids */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(360);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v8132;

s_loop_v8132: /* k id cf */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(358);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v8132;
  }

case 358: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cf id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(359);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 359: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 360: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r ids */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("fix-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 361: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k ids */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 362: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("letcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 363: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cf cont-exp k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(364);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 364: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("withcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 365: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(366);
    r[5] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[3];  
    r[7+3] = (cx__23651);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 366: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(367);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23647);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 367: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 368: /* clo k exp env h-exp e-exp */
    assert(rc == 6);
    r += 1; /* shift reg. wnd */
    /* k exp env h-exp e-exp */
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[5] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[5]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    r[7] = (vectorref((r[1]), (1)));
    r[7] = (vectorref((r[7]), (2)));
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(368);
    r[8] = (hendblk(0+1));
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[8];  
    *--hp = obj_from_case(371);
    r[8] = (hendblk(6+1));
    r[9+0] = r[8];  
    r[9+1] = r[5];  
    r[9+2] = r[6];  
    r[9+3] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v8052;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (3)));
    { fixnum_t v10750_tmp;
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10750_tmp = (n); }
    r[7] = obj_from_bool((v10750_tmp) == (1)); }
  if (bool_from_obj(r[7])) {
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = obj_from_case(372);
    r[7] = (hendblk(4+1));
    r[8+0] = (cx_exp_2Dctype);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = (car((r[6])));
    r[8+3] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
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
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

s_loop_v8052: /* k id id env */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    r[5] = (cdr((r[2])));
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(369);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    goto s_loop_v8052;
  }

case 369: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r env k id id */
    r[6] = (car((r[5])));
    r[7] = (car((r[4])));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(370);
    r[8] = (hendblk(3+1));
    r[9+0] = (cx_exp_2Dctype);
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[7];  
    r[9+3] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 370: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r id */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 371: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r fold-typecheck-ctype e-exp h-exp body k env */
    { fixnum_t v10749_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10749_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v10749_tmp);
    hreserve(hbsz(3)*c, 8); /* 8 live regs */
    l = r[1];   t = r[7];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[8] = (o); } }
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[6];  
    r[9+2] = r[5];  
    r[9+3] = r[8];  
    r[9+4] = r[4];  
    r[9+5] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 372: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r prim k e-exp h-exp */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(373);
    r[6] = (hendblk(4+1));
    r[0] = (cx_typecheck_2Dprim_2Dctype);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 373: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k e-exp h-exp r */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(374);
    r[6] = (hendblk(5+1));
    r[7+0] = (cx_member);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[1];  
    r[7+3] = (cx__233677);
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 374: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r k e-exp h-exp r r */
  if ((!bool_from_obj(r[1]))) {
  if (bool_from_obj(r[6])) {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(375);
    r[7] = (hendblk(5+1));
    r[0] = (cx_member);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    r[3] = (cx__233677);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 375: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r k e-exp h-exp r r */
  if ((!bool_from_obj(r[1]))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((strcmp(stringchars((r[5])), stringchars((r[6]))) == 0) ? (r[4]) : (r[3]));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 376: /* beta-substitute k exp hold-cps-invariants? hold-lifting-invariants? */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k exp hold-cps-invariants? hold-lifting-invariants? */
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(377);
    r[4] = (hendblk(0+1));
    r[5+0] = r[0];  
    r[5+1] = r[1];  
    r[5+2] = (mknull());
    r[5+3] = r[3];  
    r[5+4] = r[4];  
    r[5+5] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_beta_2Dsubst;

case 377: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    { bool_t v10748_tmp;
    r[3] = (cdr((r[2])));
    r[3] = (cdr((r[3])));
    r[3] = (car((r[3])));
    v10748_tmp = (fixnum_from_obj(r[3]) < (0));
    r[3] = obj_from_bool(!(v10748_tmp)); }
    r[2] = (bool_from_obj(r[3]) ? (r[3]) : (cxs_global_2Did_2Dconstant_3F_234696((r[2]))));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (3)));
  if (((r[2]) == (mksymbol(internsym("no-effect"))))) {
  if ((isnull((r[4])))) {
    r[5+0] = (cx_prim_2Dcexp_3F);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
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
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 378: /* clo k exp substs */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3]; }
    r += 1; /* shift reg. wnd */
s_beta_2Dsubst: /* k exp substs hold-lifting-invariants? bs-zero-cost-ref-transparent? hold-cps-invariants? */
    hreserve(hbsz(0+1), 6); /* 6 live regs */
    *--hp = obj_from_case(449);
    r[6] = (hendblk(0+1));
    hreserve(hbsz(0+1), 7); /* 7 live regs */
    *--hp = obj_from_case(439);
    r[7] = (hendblk(0+1));
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(425);
    r[8] = (hendblk(0+1));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(378);
    r[9] = (hendblk(3+1));
    r[10+0] = r[0];  
    r[10+1] = r[1];  
    r[10+2] = r[3];  
    r[10+3] = r[4];  
    r[10+4] = r[6];  
    r[10+5] = r[8];  
    r[10+6] = r[5];  
    r[10+7] = r[7];  
    r[10+8] = r[9];  
    r[10+9] = r[2];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_bs;

case 379: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
s_bs: /* k exp hold-lifting-invariants? bs-zero-cost-ref-transparent? bs-ref-transparent? bs-arg-of-application? hold-cps-invariants? bs-removable-if-dead? beta-subst substs */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(379);
    r[10] = (hendblk(8+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[11] = (vectorref((r[1]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[1]), (1)));
    { /* assq */
    obj x = (r[11]), l = (r[9]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[12] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[12])) {
    r[13] = (cdr((r[12])));
    r[11] = (car((r[13])));
  } else {
    r[11] = r[1];  
  }
    r[12+0] = r[0];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = obj_from_ktrap();
    r[12+2] = (r[11]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_231935((r[1])))) {
    r[11] = (vectorref((r[1]), (1)));
    r[12] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = (r[11]);
    *--hp = r[0];  
    *--hp = obj_from_case(380);
    r[13] = (hendblk(2+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_bs;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[11] = (vectorref((r[1]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[1]), (1)));
    r[12] = (vectorref((r[1]), (2)));
    r[13] = (vectorref((r[1]), (3)));
    hreserve(hbsz(4+1), 14); /* 14 live regs */
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = (r[12]);
    *--hp = obj_from_case(381);
    r[14] = (hendblk(4+1));
    r[0] = (r[14]);
    r[1] = (r[13]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_bs;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231106((r[1])))) {
    r[11] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231083((r[1])));
    /* r[0] */    
    r[1] = (r[11]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_bs;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[11] = (vectorref((r[1]), (1)));
    r[11] = (vectorref((r[11]), (1)));
    r[12] = (vectorref((r[1]), (2)));
    r[13] = (vectorref((r[1]), (1)));
    r[13] = (vectorref((r[13]), (2)));
    hreserve(hbsz(11+1), 14); /* 14 live regs */
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = (r[13]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(386);
    r[14] = (hendblk(11+1));
    r[0] = (r[14]);
    r[1] = (r[12]);
    r[2] = (r[10]);
    goto s_loop_v7935;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[11] = (vectorref((r[1]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[1]), (1)));
    r[12] = (vectorref((r[1]), (2)));
    r[13] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 14); /* 14 live regs */
    *--hp = (r[11]);
    *--hp = (r[12]);
    *--hp = r[0];  
    *--hp = obj_from_case(408);
    r[14] = (hendblk(3+1));
    r[0] = (r[14]);
    r[1] = (r[13]);
    r[2] = (r[10]);
    goto s_loop_v7792;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[11] = (vectorref((r[1]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[1]), (1)));
    r[12] = (vectorref((r[1]), (2)));
    hreserve(hbsz(8+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(424);
    r[13] = (hendblk(8+1));
    hreserve(hbsz(4+1), 14); /* 14 live regs */
    *--hp = r[0];  
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = (r[13]);
    *--hp = obj_from_case(411);
    r[13] = (hendblk(4+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    r[2] = (r[10]);
    goto s_loop_v7769;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[11] = (vectorref((r[1]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[1]), (1)));
    r[12] = (vectorref((r[1]), (2)));
    r[13] = (vectorref((r[1]), (3)));
    hreserve(hbsz(4+1), 14); /* 14 live regs */
    *--hp = (r[11]);
    *--hp = r[0];  
    *--hp = (r[12]);
    *--hp = (r[10]);
    *--hp = obj_from_case(413);
    r[14] = (hendblk(4+1));
    r[0] = (r[14]);
    r[1] = (r[13]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_bs;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[11] = (vectorref((r[1]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[1]), (1)));
    r[12] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = (r[11]);
    *--hp = r[0];  
    *--hp = obj_from_case(417);
    r[13] = (hendblk(2+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_bs;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[11] = (vectorref((r[1]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[1]), (1)));
    r[12] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = (r[11]);
    *--hp = r[0];  
    *--hp = obj_from_case(418);
    r[13] = (hendblk(2+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_bs;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[11] = (vectorref((r[1]), (0)));
    r[11] = obj_from_bool((r[11]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[1]), (1)));
    r[12] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 13); /* 13 live regs */
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = obj_from_case(419);
    r[13] = (hendblk(3+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_bs;
  } else {
    r[11] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[11]))));
    hreserve(hbsz(3+1), 12); /* 12 live regs */
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = r[1];  
    *--hp = obj_from_case(421);
    r[12] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (r[11]);
    r[3] = (cx__23657);
    r[4] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (cx__23653);
    r[3] = obj_from_bool(1);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
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

case 380: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 381: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp bs test-exp k */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(382);
    r[6] = (hendblk(4+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 382: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs test-exp k r */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(383);
    r[6] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 383: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("if-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v7935: /* k id bs */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(384);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7935;
  }

case 384: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(385);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 385: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 386: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* ek r hold-lifting-invariants? bs-zero-cost-ref-transparent? bs-ref-transparent? bs-arg-of-application? hold-cps-invariants? bs-removable-if-dead? beta-subst body substs ids k */
    r[13+0] = (r[12]);
    r[13+1] = (r[11]);
    r[13+2] = r[1];  
    r[13+3] = (mknull());
    r[13+4] = (mknull());
    r[13+5] = (r[10]);
    r[13+6] = r[2];  
    r[13+7] = r[3];  
    r[13+8] = r[4];  
    r[13+9] = r[5];  
    r[13+10] = r[6];  
    r[13+11] = r[7];  
    r[13+12] = r[8];  
    r[13+13] = r[9];  
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v7811;

case 387: /* clo k in-ids in-rands out-ids out-rands substs */
    assert(rc == 7);
    { obj* p = objptr_from_obj(r[0]);
    r[1+6] = p[1];
    r[1+7] = p[2];
    r[1+8] = p[3];
    r[1+9] = p[4];
    r[1+10] = p[5];
    r[1+11] = p[6];
    r[1+12] = p[7];
    r[1+13] = p[8]; }
    r += 1; /* shift reg. wnd */
s_loop_v7811: /* k in-ids in-rands out-ids out-rands substs hold-lifting-invariants? bs-zero-cost-ref-transparent? bs-ref-transparent? bs-arg-of-application? hold-cps-invariants? bs-removable-if-dead? beta-subst body */
    hreserve(hbsz(8+1), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(387);
    r[14] = (hendblk(8+1));
  if ((isnull((r[1])))) {
  if ((isnull((r[3])))) {
    r[15+0] = (r[12]);
    pc = objptr_from_obj(r[15+0])[0];
    r[15+1] = r[0];  
    r[15+2] = (r[13]);
    r[15+3] = r[5];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 15); /* 15 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = obj_from_case(390);
    r[15] = (hendblk(3+1));
    r[16+0] = (r[12]);
    pc = objptr_from_obj(r[16+0])[0];
    r[16+1] = (r[15]);
    r[16+2] = (r[13]);
    r[16+3] = r[5];  
    r += 16; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  } else {
    r[15] = (car((r[1])));
    r[16] = (car((r[2])));
    hreserve(hbsz(5+1), 17); /* 17 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[14]);
    *--hp = obj_from_case(389);
    r[17] = (hendblk(5+1));
    hreserve(hbsz(6+1), 18); /* 18 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = (r[14]);
    *--hp = obj_from_case(388);
    r[18] = (hendblk(6+1));
    hreserve(hbsz(18+1), 19); /* 19 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = (r[17]);
    *--hp = (r[15]);
    *--hp = (r[16]);
    *--hp = (r[18]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = (r[13]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[14]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(391);
    r[17] = (hendblk(18+1));
    r[18+0] = (cx_var_2Dreference_2Dcount);
    pc = objptr_from_obj(r[18+0])[0];
    r[18+1] = (r[17]);
    r[18+2] = (r[15]);
    r[18+3] = (r[13]);
    r += 18; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 388: /* clo k id rand */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4];
    r[1+7] = p[5];
    r[1+8] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* k id rand loop substs out-rands out-ids in-rands in-ids */
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[0];  
    r[11+2] = (cdr((r[8])));
    r[11+3] = (cdr((r[7])));
    r[11+4] = r[9];  
    r[11+5] = (r[10]);
    r[11+6] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 7);
    goto jump;

case 389: /* clo k new-substs */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* k new-substs loop out-rands out-ids in-rands in-ids */
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[0];  
    r[7+2] = (cdr((r[6])));
    r[7+3] = (cdr((r[5])));
    r[7+4] = r[4];  
    r[7+5] = r[3];  
    r[7+6] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 7);
    goto jump;

case 390: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k out-rands out-ids */
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 391: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13];
    r[1+15] = p[14];
    r[1+16] = p[15];
    r[1+17] = p[16];
    r[1+18] = p[17];
    r[1+19] = p[18]; }
    r += 1; /* shift reg. wnd */
    /* ek r hold-lifting-invariants? bs-zero-cost-ref-transparent? loop out-rands out-ids in-rands in-ids bs-ref-transparent? bs-arg-of-application? body hold-cps-invariants? bs-removable-if-dead? loop-pass rand id loop-skip substs k */
    { const fixnum_t v10746_r = fixnum_from_obj(r[1]);
  if (((0) == (v10746_r))) {
    hreserve(hbsz(6+1), 20); /* 20 live regs */
    *--hp = (r[19]);
    *--hp = (r[18]);
    *--hp = (r[17]);
    *--hp = (r[16]);
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = obj_from_case(392);
    r[20] = (hendblk(6+1));
    r[0] = (r[13]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[20]);
    r[2] = (r[15]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((1) == (v10746_r))) {
    hreserve(hbsz(9+1), 20); /* 20 live regs */
    *--hp = (r[19]);
    *--hp = (r[15]);
    *--hp = (r[16]);
    *--hp = (r[18]);
    *--hp = (r[17]);
    *--hp = (r[14]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = obj_from_case(393);
    r[20] = (hendblk(9+1));
    r[0] = r[9];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[20]);
    r[2] = (r[15]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(14+1), 20); /* 20 live regs */
    *--hp = (r[19]);
    *--hp = (r[15]);
    *--hp = (r[16]);
    *--hp = (r[18]);
    *--hp = (r[17]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = (r[11]);
    *--hp = (r[14]);
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(395);
    r[20] = (hendblk(14+1));
  if (((2) == (v10746_r))) {
  if ((!bool_from_obj(r[12]))) {
  if ((isvector((r[15])))) {
  if (((vectorlen((r[15]))) == (4))) {
    r[21] = (vectorref((r[15]), (0)));
    r[21] = obj_from_bool((r[21]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[21] = obj_from_bool(0);
  }
  } else {
    r[21] = obj_from_bool(0);
  }
  if (bool_from_obj(r[21])) {
    r[21] = (vectorref((r[15]), (3)));
    r[22] = (vectorref((r[15]), (1)));
  if (((r[22]) == (mksymbol(internsym("no-effect"))))) {
    { fixnum_t v10747_tmp;
    { /* length */
    int n; obj l = (r[21]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v10747_tmp = (n); }
    r[23] = obj_from_bool((v10747_tmp) <= (2)); }
  if (bool_from_obj(r[23])) {
    r[0] = (r[20]);
    r[1] = (r[21]);
    r[2] = r[3];  
    goto s_loop_v7814;
  } else {
    r[23+0] = obj_from_ktrap();
    r[23+1] = obj_from_bool(0);
    r[23+2] = r[2];  
    r[23+3] = r[3];  
    r[23+4] = (r[14]);
    r[23+5] = (r[11]);
    r[23+6] = r[4];  
    r[23+7] = r[5];  
    r[23+8] = r[6];  
    r[23+9] = r[7];  
    r[23+10] = r[8];  
    r[23+11] = (r[17]);
    r[23+12] = (r[18]);
    r[23+13] = (r[16]);
    r[23+14] = (r[15]);
    r[23+15] = (r[19]);
    r += 23; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7826;
  }
  } else {
    r[23+0] = obj_from_ktrap();
    r[23+1] = obj_from_bool(0);
    r[23+2] = r[2];  
    r[23+3] = r[3];  
    r[23+4] = (r[14]);
    r[23+5] = (r[11]);
    r[23+6] = r[4];  
    r[23+7] = r[5];  
    r[23+8] = r[6];  
    r[23+9] = r[7];  
    r[23+10] = r[8];  
    r[23+11] = (r[17]);
    r[23+12] = (r[18]);
    r[23+13] = (r[16]);
    r[23+14] = (r[15]);
    r[23+15] = (r[19]);
    r += 23; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7826;
  }
  } else {
    r[21+0] = obj_from_ktrap();
    r[21+1] = obj_from_bool(0);
    r[21+2] = r[2];  
    r[21+3] = r[3];  
    r[21+4] = (r[14]);
    r[21+5] = (r[11]);
    r[21+6] = r[4];  
    r[21+7] = r[5];  
    r[21+8] = r[6];  
    r[21+9] = r[7];  
    r[21+10] = r[8];  
    r[21+11] = (r[17]);
    r[21+12] = (r[18]);
    r[21+13] = (r[16]);
    r[21+14] = (r[15]);
    r[21+15] = (r[19]);
    r += 21; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7826;
  }
  } else {
    r[21+0] = obj_from_ktrap();
    r[21+1] = obj_from_bool(0);
    r[21+2] = r[2];  
    r[21+3] = r[3];  
    r[21+4] = (r[14]);
    r[21+5] = (r[11]);
    r[21+6] = r[4];  
    r[21+7] = r[5];  
    r[21+8] = r[6];  
    r[21+9] = r[7];  
    r[21+10] = r[8];  
    r[21+11] = (r[17]);
    r[21+12] = (r[18]);
    r[21+13] = (r[16]);
    r[21+14] = (r[15]);
    r[21+15] = (r[19]);
    r += 21; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7826;
  }
  } else {
    r[21+0] = obj_from_ktrap();
    r[21+1] = obj_from_bool(0);
    r[21+2] = r[2];  
    r[21+3] = r[3];  
    r[21+4] = (r[14]);
    r[21+5] = (r[11]);
    r[21+6] = r[4];  
    r[21+7] = r[5];  
    r[21+8] = r[6];  
    r[21+9] = r[7];  
    r[21+10] = r[8];  
    r[21+11] = (r[17]);
    r[21+12] = (r[18]);
    r[21+13] = (r[16]);
    r[21+14] = (r[15]);
    r[21+15] = (r[19]);
    r += 21; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7826;
  }
  }
  } } 

case 392: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop-pass rand id loop-skip substs k */
  if (bool_from_obj(r[1])) {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 393: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs-arg-of-application? body hold-cps-invariants? loop-pass loop-skip substs id rand k */
    hreserve(hbsz(6+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(394);
    r[11] = (hendblk(6+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    r[2] = r[5];  
    r[3] = r[6];  
    r[4] = r[7];  
    r[5] = r[8];  
    r[6] = r[9];  
    r[7] = (r[10]);
    goto s_l_v7880;
  } else {
  if ((!bool_from_obj(r[4]))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[8];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    r[2] = r[5];  
    r[3] = r[6];  
    r[4] = r[7];  
    r[5] = r[8];  
    r[6] = r[9];  
    r[7] = (r[10]);
    goto s_l_v7880;
  }
  }

case 394: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
s_l_v7880: /* ek r loop-pass loop-skip substs id rand k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[4];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[5];  
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 395: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13];
    r[1+15] = p[14]; }
    r += 1; /* shift reg. wnd */
s_l_v7826: /* ek r hold-lifting-invariants? bs-zero-cost-ref-transparent? loop-pass body loop out-rands out-ids in-rands in-ids loop-skip substs id rand k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 16); /* 16 live regs */
    *--hp = (mknull());
    *--hp = (r[14]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[16] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 17); /* 17 live regs */
    *--hp = (r[16]);
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[16] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 17); /* 17 live regs */
    *--hp = (r[12]);
    *--hp = (r[16]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[16] = (hendblk(3)); }
    r[0] = (r[11]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[15]);
    r[2] = (r[16]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(13+1), 16); /* 16 live regs */
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(396);
    r[16] = (hendblk(13+1));
  if (bool_from_obj(r[2])) {
    r[0] = (r[16]);
    r[1] = (r[14]);
    goto gs_curry_2Dexp_3F_234038;
  } else {
  if ((isvector((r[14])))) {
  if (((vectorlen((r[14]))) == (3))) {
    r[17] = (vectorref((r[14]), (0)));
    r[17] = obj_from_bool((r[17]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[17] = obj_from_bool(0);
  }
  } else {
    r[17] = obj_from_bool(0);
  }
  if (bool_from_obj(r[17])) {
    r[17] = (vectorref((r[14]), (2)));
  if ((isvector((r[17])))) {
  if (((vectorlen((r[17]))) == (4))) {
    r[18] = (vectorref((r[17]), (0)));
    r[18] = obj_from_bool((r[18]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[18] = obj_from_bool(0);
  }
  } else {
    r[18] = obj_from_bool(0);
  }
  if (bool_from_obj(r[18])) {
    r[18] = (vectorref((r[17]), (3)));
    r[0] = (r[16]);
    r[1] = (r[18]);
    r[2] = r[3];  
    goto s_loop_v7844;
  } else {
  if ((isvector((r[17])))) {
  if (((vectorlen((r[17]))) == (3))) {
    r[18] = (vectorref((r[17]), (0)));
    r[18] = obj_from_bool((r[18]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[18] = obj_from_bool(0);
  }
  } else {
    r[18] = obj_from_bool(0);
  }
  if (bool_from_obj(r[18])) {
    r[18] = (vectorref((r[17]), (2)));
    r[19] = (vectorref((r[17]), (1)));
    hreserve(hbsz(3+1), 20); /* 20 live regs */
    *--hp = (r[16]);
    *--hp = (r[18]);
    *--hp = r[3];  
    *--hp = obj_from_case(401);
    r[20] = (hendblk(3+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[20]);
    r[2] = (r[19]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    r[9] = (r[10]);
    r[10] = (r[11]);
    r[11] = (r[12]);
    r[12] = (r[13]);
    r[13] = (r[14]);
    r[14] = (r[15]);
    goto s_l_v7856;
  }
  }
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    r[9] = (r[10]);
    r[10] = (r[11]);
    r[11] = (r[12]);
    r[12] = (r[13]);
    r[13] = (r[14]);
    r[14] = (r[15]);
    goto s_l_v7856;
  }
  }
  }

case 396: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13]; }
    r += 1; /* shift reg. wnd */
s_l_v7856: /* ek r bs-zero-cost-ref-transparent? loop-pass body loop out-rands out-ids in-rands in-ids loop-skip substs id rand k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(10+1), 15); /* 15 live regs */
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(397);
    r[15] = (hendblk(10+1));
    r[0] = (cx_var_2Donly_2Dapplied_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[15]);
    r[2] = (r[12]);
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(6+1), 15); /* 15 live regs */
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[3];  
    *--hp = obj_from_case(398);
    r[15] = (hendblk(6+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[15]);
    r[2] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 397: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop out-rands out-ids in-rands in-ids loop-skip substs id rand k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (mknull());
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = r[8];  
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (mknull());
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[13] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[12]);
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = r[8];  
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = r[4];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[3];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    r[15+0] = r[2];  
    pc = objptr_from_obj(r[15+0])[0];
    r[15+1] = (r[11]);
    r[15+2] = (cdr((r[6])));
    r[15+3] = (cdr((r[5])));
    r[15+4] = (r[13]);
    r[15+5] = (r[14]);
    r[15+6] = (r[12]);
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 7);
    goto jump;
  }

case 398: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop-pass loop-skip substs id rand k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[4];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[5];  
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 399: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7844: /* k id bs-zero-cost-ref-transparent? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(399);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(400);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 400: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 401: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs-zero-cost-ref-transparent? rands k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7829;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 402: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7829: /* k id bs-zero-cost-ref-transparent? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(402);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(403);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 403: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 404: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7814: /* k id bs-zero-cost-ref-transparent? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(404);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(405);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 405: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop_v7792: /* k id bs */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(406);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7792;
  }

case 406: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(407);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 407: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 408: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k prim effect */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v7769: /* k id bs */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(409);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7769;
  }

case 409: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(410);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 410: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 411: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs-head rator bs k */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(412);
    r[6] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 412: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs k r */
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[5])))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 413: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs lams k ids */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(416);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7736;

s_loop_v7736: /* k id bs */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(414);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7736;
  }

case 414: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(415);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 415: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 416: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r ids */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("fix-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 417: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k ids */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 418: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("letcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 419: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs cont-exp k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(420);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 420: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("withcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 421: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(422);
    r[5] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[3];  
    r[7+3] = (cx__23651);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 422: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(423);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23647);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 423: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 424: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* k exp hold-lifting-invariants? bs-zero-cost-ref-transparent? bs-ref-transparent? bs-arg-of-application? hold-cps-invariants? bs-removable-if-dead? beta-subst substs */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[10] = (vectorref((r[1]), (0)));
    r[10] = obj_from_bool((r[10]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[10] = obj_from_bool(0);
  }
  } else {
    r[10] = obj_from_bool(0);
  }
  if (bool_from_obj(r[10])) {
    r[10] = (vectorref((r[1]), (1)));
    { /* assq */
    obj x = (r[10]), l = (r[9]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[11] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[11])) {
    r[12] = (cdr((r[11])));
    r[12] = (cdr((r[12])));
    r[12] = obj_from_bool(isnull((r[12])));
  if (bool_from_obj(r[12])) {
    r[12] = (cdr((r[11])));
    r[10] = (car((r[12])));
  } else {
    r[12] = (cdr((r[11])));
    r[12] = (cdr((r[12])));
    r[10] = (car((r[12])));
  }
  } else {
    r[10] = r[1];  
  }
    r[11+0] = r[0];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = obj_from_ktrap();
    r[11+2] = (r[10]);
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    /* r[0] */    
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_bs;
  }

case 425: /* clo k id body */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id body */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(449);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = obj_from_case(426);
    r[4] = (hendblk(1+1));
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (4))) {
    r[5] = (vectorref((r[2]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[2]), (3)));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(429);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[5];  
    r[2] = r[3];  
    goto s_loop_v7653;
  } else {
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (3))) {
    r[5] = (vectorref((r[2]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[2]), (1)));
    r[6] = (vectorref((r[2]), (2)));
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_case(432);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = r[5];  
    goto s_bs_2Dref_2Dtransparent_3F;
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

case 426: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k exp id */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[3] = obj_from_bool((r[2]) == (r[3]));
  } else {
    r[3] = obj_from_bool(0);
  }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 427: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7653: /* k id bs-ref-transparent? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(427);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(428);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 428: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 429: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id-one-of? rands k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7638;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 430: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7638: /* k id id-one-of? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(430);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(431);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 431: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[5+0] = r[4];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 432: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs-ref-transparent? rator id-one-of? rands k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(435);
    r[7] = (hendblk(4+1));
    r[0] = r[7];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_loop_v7621;
  } else {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 433: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7621: /* k id bs-ref-transparent? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(433);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(434);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 434: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 435: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r rator id-one-of? rands k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(436);
    r[6] = (hendblk(3+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 436: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id-one-of? rands k */
  if (bool_from_obj(r[1])) {
    r[5+0] = r[4];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7602;
  }

case 437: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7602: /* k id id-one-of? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(437);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(438);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 438: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[5+0] = r[4];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 439: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_bs_2Dremovable_2Dif_2Ddead_3F: /* k exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(439);
    r[2] = (hendblk(0+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[1]), (2)));
    r[5] = (vectorref((r[1]), (3)));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[6];  
    /* r[2] */    
    goto s_loop_v7575;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[1]), (3)));
    { /* memq */
    obj x = (r[3]), l = (cx__234516);
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[5] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[5])) {
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v7564;
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
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[4]), (2)));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(446);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7553;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (2)));
    /* r[0] */    
    r[1] = r[3];  
    goto s_bs_2Dremovable_2Dif_2Ddead_3F;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[1]), (2)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    goto s_loop_v7536;
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
  }
  }
  }

case 440: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7575: /* k id bs-removable-if-dead? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(440);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(441);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 441: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 442: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7564: /* k id bs-removable-if-dead? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(442);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(443);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 443: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 444: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7553: /* k id bs-removable-if-dead? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(444);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(445);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 445: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 446: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs-removable-if-dead? body k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 447: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7536: /* k id bs-removable-if-dead? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(447);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(448);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 448: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 449: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_bs_2Dref_2Dtransparent_3F: /* k exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(449);
    r[2] = (hendblk(0+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    { bool_t v10745_tmp;
    r[4] = (cdr((r[3])));
    r[4] = (cdr((r[4])));
    r[4] = (car((r[4])));
    v10745_tmp = (fixnum_from_obj(r[4]) < (0));
    r[4] = obj_from_bool(!(v10745_tmp)); }
    r[3] = (bool_from_obj(r[4]) ? (r[4]) : (cxs_global_2Did_2Dconstant_3F_234696((r[3]))));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[1]), (2)));
    r[5] = (vectorref((r[1]), (3)));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[6];  
    /* r[2] */    
    goto s_loop_v7515;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[1]), (3)));
  if (((r[3]) == (mksymbol(internsym("no-effect"))))) {
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v7504;
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
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23215((r[1])))) {
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[4]), (2)));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(456);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7493;
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
  }

case 450: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7515: /* k id bs-ref-transparent? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(450);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(451);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 451: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 452: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7504: /* k id bs-ref-transparent? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(452);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(453);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 453: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 454: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7493: /* k id bs-ref-transparent? */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(454);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(455);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 455: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 456: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bs-ref-transparent? body k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

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
