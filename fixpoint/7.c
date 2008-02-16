/* 7.sf */
#ifdef PROFILE
#define host host_module_7
#endif
#define MODULE module_7
#define LOAD() module_6(); module_3(); module_0(); 
extern void module_0(void); /* 0.sf */
extern void module_3(void); /* 3.sf */
extern void module_6(void); /* 6.sf */

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
#define mkimm(o, t) (obj)((((o) & 0xffffff) << 8) | ((t) << 1) | 1)
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
extern int iseqv(obj x, obj y);
extern obj ismemv(obj x, obj l);
extern obj isassv(obj x, obj l);
extern int isequal(obj x, obj y);
extern obj ismember(obj x, obj l);
extern obj isassoc(obj x, obj l);

/* cx globals */
extern obj cx__2Acurrent_2Derror_2Dport_2A; /* *current-error-port* */
extern obj cx_c_2Derror_2A; /* c-error* */
extern obj cx_c_2Dformat_2A; /* c-format* */
extern obj cx_c_2Dformat_2Dprim_2A; /* c-format-prim* */
extern obj cx_c_2Dformat_2Dprimexp_2A; /* c-format-primexp* */
extern obj cx_c_2Dmangle; /* c-mangle */
extern obj cx_c_2Dundecorate_2Dalvar; /* c-undecorate-alvar */
extern obj cx_cleanup_2Dc_2Dcode_21; /* cleanup-c-code! */
extern obj cx_exp_2D_3Efree_2Dvars; /* exp->free-vars */
extern obj cx_fixnum_2D_3Estring; /* fixnum->string */
extern obj cx_flonum_2D_3Estring; /* flonum->string */
extern obj cx_fprintf_2A; /* fprintf* */
extern obj cx_id_2D_3Euname; /* id->uname */
extern obj cx_labelapp_2Dexp_3F; /* labelapp-exp? */
extern obj cx_memoizate_2Dexp_2Dattribute; /* memoizate-exp-attribute */
extern obj cx_posq; /* posq */
extern obj cx_prim_2Dcexp_3F; /* prim-cexp? */
extern obj cx_primexp_2Dctype; /* primexp-ctype */
extern obj cx_reduce_2Dleft; /* reduce-left */
extern obj cx_reset; /* reset */
extern obj cx_string_2Dstarts_2Dwith_3F; /* string-starts-with? */
extern obj cx_the_2Dhalt_2Dprim; /* the-halt-prim */
extern obj cx_timestamp; /* timestamp */
extern obj cx_var_2Dassigned_2Din_2Dexp_3F; /* var-assigned-in-exp? */
extern obj cx_var_2Dreferenced_2Din_2Dexp_3F; /* var-referenced-in-exp? */
extern obj cx_write_2F3; /* write/3 */
obj cx_bgc_2Dexp; /* bgc-exp */
obj cx_bgc_2Dexp_2D_3Eexp2; /* bgc-exp->exp2 */
obj cx_bgc_2Dexp_2D_3Eid; /* bgc-exp->id */
obj cx_bgc_2Dexp_2D_3Eids; /* bgc-exp->ids */
obj cx_bgc_2Dexp_2D_3Elbid; /* bgc-exp->lbid */
obj cx_bgc_2Dexp_3F; /* bgc-exp? */
obj cx_code_2Dgenerate; /* code-generate */
obj cx_merge_2Dby; /* merge-by */
obj cx_merge_2Dby_21; /* merge-by! */
obj cx_rassq; /* rassq */
obj cx_sort_2Dby; /* sort-by */
obj cx_sort_2Dby_21; /* sort-by! */
obj cx_split_2Dby_21; /* split-by! */
static obj cx__231232; /* constant #1232 */
static obj cx__231253; /* constant #1253 */
static obj cx__231266; /* constant #1266 */
static obj cx__231275; /* constant #1275 */
static obj cx__231310; /* constant #1310 */
static obj cx__231322; /* constant #1322 */
static obj cx__231342; /* constant #1342 */
static obj cx__231412; /* constant #1412 */
static obj cx__231421; /* constant #1421 */
static obj cx__231485; /* constant #1485 */
static obj cx__231495; /* constant #1495 */
static obj cx__231498; /* constant #1498 */
static obj cx__231503; /* constant #1503 */
static obj cx__231514; /* constant #1514 */
static obj cx__231521; /* constant #1521 */
static obj cx__231530; /* constant #1530 */
static obj cx__231535; /* constant #1535 */
static obj cx__231553; /* constant #1553 */
static obj cx__231769; /* constant #1769 */
static obj cx__232052; /* constant #2052 */
static obj cx__232053; /* constant #2053 */
static obj cx__232056; /* constant #2056 */
static obj cx__232059; /* constant #2059 */
static obj cx__232062; /* constant #2062 */
static obj cx__232065; /* constant #2065 */
static obj cx__232068; /* constant #2068 */
static obj cx__232080; /* constant #2080 */
static obj cx__232083; /* constant #2083 */
static obj cx__232086; /* constant #2086 */
static obj cx__232089; /* constant #2089 */
static obj cx__232110; /* constant #2110 */
static obj cx__232113; /* constant #2113 */
static obj cx__232120; /* constant #2120 */
static obj cx__232130; /* constant #2130 */
static obj cx__232133; /* constant #2133 */
static obj cx__232197; /* constant #2197 */
static obj cx__232203; /* constant #2203 */
static obj cx__232206; /* constant #2206 */
static obj cx__232209; /* constant #2209 */
static obj cx__232501; /* constant #2501 */
static obj cx__232508; /* constant #2508 */
static obj cx__232517; /* constant #2517 */
static obj cx__232523; /* constant #2523 */
static obj cx__232537; /* constant #2537 */
static obj cx__232563; /* constant #2563 */
static obj cx__232584; /* constant #2584 */
static obj cx__232589; /* constant #2589 */
static obj cx__232594; /* constant #2594 */
static obj cx__232597; /* constant #2597 */
static obj cx__232865; /* constant #2865 */
static obj cx__232882; /* constant #2882 */
static obj cx__232896; /* constant #2896 */
static obj cx__232917; /* constant #2917 */
static obj cx__232922; /* constant #2922 */
static obj cx__232953; /* constant #2953 */
static obj cx__232961; /* constant #2961 */
static obj cx__232977; /* constant #2977 */
static obj cx__233021; /* constant #3021 */
static obj cx__233030; /* constant #3030 */
static obj cx__233040; /* constant #3040 */
static obj cx__233041; /* constant #3041 */
static obj cx__233049; /* constant #3049 */
static obj cx__233054; /* constant #3054 */
static obj cx__233063; /* constant #3063 */
static obj cx__233076; /* constant #3076 */
static obj cx__233077; /* constant #3077 */
static obj cx__233080; /* constant #3080 */
static obj cx__233088; /* constant #3088 */
static obj cx__233187; /* constant #3187 */
static obj cx__233188; /* constant #3188 */
static obj cx__233210; /* constant #3210 */
static obj cx__233224; /* constant #3224 */
static obj cx__233234; /* constant #3234 */
static obj cx__233235; /* constant #3235 */
static obj cx__233273; /* constant #3273 */
static obj cx__23466; /* constant #466 */
static obj cx__23472; /* constant #472 */
static obj cx__23475; /* constant #475 */
static obj cx__23489; /* constant #489 */
static obj cx__23494; /* constant #494 */
static obj cx__23497; /* constant #497 */
static obj cx__23525; /* constant #525 */
static obj cx__23533; /* constant #533 */
static obj cx__23535; /* constant #535 */
static obj cx__23538; /* constant #538 */
static obj cx__23541; /* constant #541 */
static obj cx__23558; /* constant #558 */
static obj cx__23561; /* constant #561 */
static obj cx__23601; /* constant #601 */
static obj cx__23609; /* constant #609 */
static obj cx__23617; /* constant #617 */
static obj cx__23640; /* constant #640 */
static obj cx__23691; /* constant #691 */
static obj cx__23713; /* constant #713 */
static obj cx__23718; /* constant #718 */
static obj cx__23729; /* constant #729 */
static obj cx__23746; /* constant #746 */
static obj cx__23749; /* constant #749 */
static obj cx__23769; /* constant #769 */
static obj cx__23780; /* constant #780 */
static obj cx__23789; /* constant #789 */
static obj cx__23792; /* constant #792 */
static obj cx__23823; /* constant #823 */
static obj cx__23831; /* constant #831 */
static obj cx__23834; /* constant #834 */
static obj cx__23839; /* constant #839 */
static obj cx__23851; /* constant #851 */
static obj cx__23856; /* constant #856 */
static obj cx__23861; /* constant #861 */
static obj cx__23866; /* constant #866 */
static obj cx__23895; /* constant #895 */
static obj cx__23929; /* constant #929 */
static obj cx__23948; /* constant #948 */
static obj cx__23971; /* constant #971 */
static obj cx__23975; /* constant #975 */
static obj cx__23977; /* constant #977 */
static obj cx__23981; /* constant #981 */
static obj cx_begin_2Dexp_3F_23301; /* constant begin-exp?#301 */
static obj cx_curry_2Dexp_2381; /* constant curry-exp#81 */
static obj cx_curry_2Dexp_3F_23213; /* constant curry-exp?#213 */
static obj cx_halt_2Dexp_3F_231934; /* constant halt-exp?#1934 */
static obj cx_iota_23869; /* constant iota#869 */
static obj cx_keep_23510; /* constant keep#510 */

/* helper functions */
/* gvarassign-exp?#274 */
static obj cxs_gvarassign_2Dexp_3F_23274(obj v276_exp)
{ 
  if (bool_from_obj((isvector((v276_exp))) ? (((vectorlen((v276_exp))) == (3)) ? obj_from_bool((vectorref((v276_exp), (0))) == (mksymbol(internsym("varassign-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v282_id = (vectorref((v276_exp), (1)));
    return obj_from_bool(fixnum_from_obj(car((cdr((cdr((v282_id))))))) < (0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* let-exp?#331 */
static obj cxs_let_2Dexp_3F_23331(obj v333_exp)
{ 
  if (bool_from_obj((isvector((v333_exp))) ? (((vectorlen((v333_exp))) == (3)) ? obj_from_bool((vectorref((v333_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v337_rator = (vectorref((v333_exp), (1)));
  if (bool_from_obj((isvector((v337_rator))) ? (((vectorlen((v337_rator))) == (3)) ? obj_from_bool((vectorref((v337_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v3619_tmp;
    obj v3618_tmp;
    { /* length */
    int n; obj l = (vectorref((v333_exp), (2)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v3619_tmp = obj_from_fixnum(n); };
    { /* length */
    int n; obj l = (vectorref((v337_rator), (1)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v3618_tmp = obj_from_fixnum(n); };
    return obj_from_bool(fixnum_from_obj(v3618_tmp) == fixnum_from_obj(v3619_tmp));
  }
  } else {
    return obj_from_bool(0);
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* global-id-private-constant?#618 */
static obj cxs_global_2Did_2Dprivate_2Dconstant_3F_23618(obj v620_id)
{ 
    return ((fixnum_from_obj(car((cdr((cdr((v620_id))))))) < (0)) ? obj_from_bool((car((cdr((cdr((cdr((v620_id))))))))) == (mksymbol(internsym("private")))) : obj_from_bool(0));
}

/* global-id-constant?#1038 */
static obj cxs_global_2Did_2Dconstant_3F_231038(obj v1040_id)
{ 
    return ((fixnum_from_obj(car((cdr((cdr((v1040_id))))))) < (0)) ? (car((cdr((cdr((cdr((v1040_id))))))))) : obj_from_bool(0));
}

/* label-id?#1554 */
static obj cxs_label_2Did_3F_231554(obj v1556_id)
{ 
    return ((!(fixnum_from_obj(car((cdr((cdr((v1556_id))))))) < (0))) ? obj_from_bool((car((cdr((cdr((cdr((v1556_id))))))))) == obj_from_bool(1)) : obj_from_bool(0));
}

/* degenerate-let-exp->body#2367 */
static obj cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_232367(obj v2369_exp)
{ 
    return ((isnull((vectorref((v2369_exp), (2))))) ? (vectorref((vectorref((v2369_exp), (1))), (2))) : (car((vectorref((v2369_exp), (2))))));
}

/* null-let-exp?#2393 */
static obj cxs_null_2Dlet_2Dexp_3F_232393(obj v2395_exp)
{ 
  if (bool_from_obj((isvector((v2395_exp))) ? (((vectorlen((v2395_exp))) == (3)) ? obj_from_bool((vectorref((v2395_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if ((isnull((vectorref((v2395_exp), (2)))))) {
  { /* let */
    obj v2399_rator = (vectorref((v2395_exp), (1)));
    return (bool_from_obj((isvector((v2399_rator))) ? (((vectorlen((v2399_rator))) == (3)) ? obj_from_bool((vectorref((v2399_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool(isnull((vectorref((v2399_rator), (1))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-lambda-exp?#2439 */
static obj cxs_identity_2Dlambda_2Dexp_3F_232439(obj v2441_exp)
{ 
  if (bool_from_obj((isvector((v2441_exp))) ? (((vectorlen((v2441_exp))) == (3)) ? obj_from_bool((vectorref((v2441_exp), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v2448_body = (vectorref((v2441_exp), (2)));
    obj v2449_ids = (vectorref((v2441_exp), (1)));
    return ((ispair((v2449_ids))) ? ((isnull((cdr((v2449_ids))))) ? (bool_from_obj((isvector((v2448_body))) ? (((vectorlen((v2448_body))) == (2)) ? obj_from_bool((vectorref((v2448_body), (0))) == (mksymbol(internsym("var-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool((car((v2449_ids))) == (vectorref((v2448_body), (1)))) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-let-exp?#2426 */
static obj cxs_identity_2Dlet_2Dexp_3F_232426(obj v2428_exp)
{ 
  if (bool_from_obj((isvector((v2428_exp))) ? (((vectorlen((v2428_exp))) == (3)) ? obj_from_bool((vectorref((v2428_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if (bool_from_obj(cxs_identity_2Dlambda_2Dexp_3F_232439((vectorref((v2428_exp), (1)))))) {
  { /* let */
    obj v2432_rands = (vectorref((v2428_exp), (2)));
    return ((ispair((v2432_rands))) ? obj_from_bool(isnull((cdr((v2432_rands))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* degenerate-let-exp?#2390 */
static obj cxs_degenerate_2Dlet_2Dexp_3F_232390(obj v2392_exp)
{ 
  { /* let */
    obj v2425_x = (cxs_null_2Dlet_2Dexp_3F_232393((v2392_exp)));
    return (bool_from_obj(v2425_x) ? (v2425_x) : (cxs_identity_2Dlet_2Dexp_3F_232426((v2392_exp))));
  }
}

/* cvar-id?#2940 */
static obj cxs_cvar_2Did_3F_232940(obj v2942_id)
{ 
    return ((!(fixnum_from_obj(car((cdr((cdr((v2942_id))))))) < (0))) ? obj_from_bool(isstring((car((cdr((cdr((cdr((v2942_id))))))))))) : obj_from_bool(0));
}

/* rassq */
static obj cxs_rassq(obj v2_x, obj v1_al)
{ 
  s_rassq:
  if ((isnull((v1_al)))) {
    return obj_from_bool(0);
  } else {
  if (((v2_x) == (cdr((car((v1_al))))))) {
    return (car((v1_al)));
  } else {
  { /* let */
    obj v3617_tmp = (cdr((v1_al)));
    obj v3616_tmp = (v2_x);
    /* tail call */
    v2_x = (v3616_tmp);
    v1_al = (v3617_tmp);
    goto s_rassq;
  }
  }
  }
}

/* split-by! */
static obj cxs_split_2Dby_21(obj v28_lst)
{ 
  if ((!ispair((v28_lst)))) {
    return (v28_lst);
  } else {
  { /* letrec */
    obj v32_hd;
    obj v31_tl;
  { /* let */
    obj v3615_tmp = (cdr((v28_lst)));
    obj v3614_tmp = (v28_lst);
    /* tail call */
    v32_hd = (v3614_tmp);
    v31_tl = (v3615_tmp);
    goto s_loop;
  }
  s_loop:
  if (bool_from_obj((isnull((v31_tl))) ? obj_from_bool(isnull((v31_tl))) : obj_from_bool(isnull((cdr((v31_tl))))))) {
  { /* let */
    obj v38_x = (cdr((v32_hd)));
    (void) obj_from_void(cdr((v32_hd)) = (mknull()));
    return (v38_x);
  }
  } else {
  { /* let */
    obj v3613_tmp = (cdr((cdr((v31_tl)))));
    obj v3612_tmp = (cdr((v32_hd)));
    /* tail call */
    v32_hd = (v3612_tmp);
    v31_tl = (v3613_tmp);
    goto s_loop;
  }
  }
  }
  }
}

/* gc roots */
static obj *globv[] = {
  &cx__231232,
  &cx__231253,
  &cx__231266,
  &cx__231275,
  &cx__231310,
  &cx__231322,
  &cx__231342,
  &cx__231412,
  &cx__231421,
  &cx__231485,
  &cx__231495,
  &cx__231498,
  &cx__231503,
  &cx__231514,
  &cx__231521,
  &cx__231530,
  &cx__231535,
  &cx__231553,
  &cx__231769,
  &cx__232052,
  &cx__232053,
  &cx__232056,
  &cx__232059,
  &cx__232062,
  &cx__232065,
  &cx__232068,
  &cx__232080,
  &cx__232083,
  &cx__232086,
  &cx__232089,
  &cx__232110,
  &cx__232113,
  &cx__232120,
  &cx__232130,
  &cx__232133,
  &cx__232197,
  &cx__232203,
  &cx__232206,
  &cx__232209,
  &cx__232501,
  &cx__232508,
  &cx__232517,
  &cx__232523,
  &cx__232537,
  &cx__232563,
  &cx__232584,
  &cx__232589,
  &cx__232594,
  &cx__232597,
  &cx__232865,
  &cx__232882,
  &cx__232896,
  &cx__232917,
  &cx__232922,
  &cx__232953,
  &cx__232961,
  &cx__232977,
  &cx__233021,
  &cx__233030,
  &cx__233040,
  &cx__233041,
  &cx__233049,
  &cx__233054,
  &cx__233063,
  &cx__233076,
  &cx__233077,
  &cx__233080,
  &cx__233088,
  &cx__233187,
  &cx__233188,
  &cx__233210,
  &cx__233224,
  &cx__233234,
  &cx__233235,
  &cx__233273,
  &cx__23466,
  &cx__23472,
  &cx__23475,
  &cx__23489,
  &cx__23494,
  &cx__23497,
  &cx__23525,
  &cx__23533,
  &cx__23535,
  &cx__23538,
  &cx__23541,
  &cx__23558,
  &cx__23561,
  &cx__23601,
  &cx__23609,
  &cx__23617,
  &cx__23640,
  &cx__23691,
  &cx__23713,
  &cx__23718,
  &cx__23729,
  &cx__23746,
  &cx__23749,
  &cx__23769,
  &cx__23780,
  &cx__23789,
  &cx__23792,
  &cx__23823,
  &cx__23831,
  &cx__23834,
  &cx__23839,
  &cx__23851,
  &cx__23856,
  &cx__23861,
  &cx__23866,
  &cx__23895,
  &cx__23929,
  &cx__23948,
  &cx__23971,
  &cx__23975,
  &cx__23977,
  &cx__23981,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[300] = {
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
};

/* host procedure */
#define MAX_LIVEREGS 43
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
    { static obj c[] = { obj_from_case(1) }; cx_curry_2Dexp_2381 = (obj)c; }
    { static obj c[] = { obj_from_case(4) }; cx_curry_2Dexp_3F_23213 = (obj)c; }
    { static obj c[] = { obj_from_case(6) }; cx_begin_2Dexp_3F_23301 = (obj)c; }
    { static char s[] = { 10, 125, 59, 10, 0 };
    cx__23466 = (hpushstr(0, newstring(s))); }
    cx__23472 = (hpushstr(0, newstring("  (obj)host,")));
    { static char s[] = { 10, 32, 32, 40, 111, 98, 106, 41, 104, 111, 115, 116, 44, 0 };
    cx__23475 = (hpushstr(0, newstring(s))); }
    cx__23489 = (hpushstr(0, newstring("] = {")));
    cx__23494 = (hpushstr(0, newstring("static obj cases[")));
    { static char s[] = { 115, 116, 97, 116, 105, 99, 32, 111, 98, 106, 32, 104, 111, 115, 116, 40, 111, 98, 106, 41, 59, 10, 0 };
    cx__23497 = (hpushstr(0, newstring(s))); }
    { static obj c[] = { obj_from_case(8) }; cx_keep_23510 = (obj)c; }
    cx__23525 = (hpushstr(0, newstring("cx_")));
    { static char s[] = { 32, 32, 38, 36, 97, 44, 10, 0 };
    cx__23533 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 125, 59, 10, 10, 0 };
    cx__23535 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 115, 105, 122, 101, 111, 102, 40, 103, 108, 111, 98, 118, 41, 47, 115, 105, 122, 101, 111, 102, 40, 111, 98, 106, 32, 42, 41, 44, 32, 103, 108, 111, 98, 118, 44, 32, 78, 85, 76, 76, 10, 0 };
    cx__23538 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 115, 116, 97, 116, 105, 99, 32, 99, 120, 114, 111, 111, 116, 95, 116, 32, 114, 111, 111, 116, 32, 61, 32, 123, 10, 0 };
    cx__23541 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 115, 116, 97, 116, 105, 99, 32, 111, 98, 106, 32, 42, 103, 108, 111, 98, 118, 91, 93, 32, 61, 32, 123, 10, 0 };
    cx__23558 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 115, 116, 97, 116, 105, 99, 32, 99, 120, 114, 111, 111, 116, 95, 116, 32, 114, 111, 111, 116, 32, 61, 32, 123, 32, 48, 44, 32, 78, 85, 76, 76, 44, 32, 78, 85, 76, 76, 32, 125, 59, 10, 10, 0 };
    cx__23561 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 101, 120, 116, 101, 114, 110, 32, 111, 98, 106, 32, 36, 97, 59, 32, 47, 42, 32, 36, 97, 32, 42, 47, 10, 0 };
    cx__23601 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 111, 98, 106, 32, 36, 97, 59, 32, 47, 42, 32, 36, 97, 32, 42, 47, 10, 0 };
    cx__23609 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 115, 116, 97, 116, 105, 99, 32, 111, 98, 106, 32, 36, 97, 59, 32, 47, 42, 32, 99, 111, 110, 115, 116, 97, 110, 116, 32, 36, 97, 32, 42, 47, 10, 0 };
    cx__23617 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 0 };
    cx__23640 = (hpushstr(0, newstring(s))); }
    cx__23691 = (hpushstr(0, newstring(": ")));
    { static char s[] = { 10, 10, 0 };
    cx__23713 = (hpushstr(0, newstring(s))); }
    cx__23718 = (hpushstr(0, newstring("*/")));
    cx__23729 = (hpushstr(0, newstring(" ")));
    cx__23746 = (hpushstr(0, newstring("/* ")));
    cx__23749 = (hpushstr(0, newstring("    ")));
    { static char s[] = { 32, 32, 32, 32, 114, 32, 43, 61, 32, 49, 59, 32, 47, 42, 32, 115, 104, 105, 102, 116, 32, 114, 101, 103, 46, 32, 119, 110, 100, 32, 42, 47, 10, 0 };
    cx__23769 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 41, 59, 10, 0 };
    cx__23780 = (hpushstr(0, newstring(s))); }
    cx__23789 = (hpushstr(0, newstring("    assert(rc == ")));
    { static char s[] = { 42, 47, 10, 0 };
    cx__23792 = (hpushstr(0, newstring(s))); }
    cx__23823 = (hpushstr(0, newstring("clo")));
    cx__23831 = (hpushstr(0, newstring(" /* ")));
    cx__23834 = (hpushstr(0, newstring(":")));
    cx__23839 = (hpushstr(0, newstring("case ")));
    cx__23851 = (hpushstr(0, newstring("];")));
    cx__23856 = (hpushstr(0, newstring("] = p[")));
    { static char s[] = { 10, 32, 32, 32, 32, 114, 91, 49, 43, 0 };
    cx__23861 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 125, 10, 0 };
    cx__23866 = (hpushstr(0, newstring(s))); }
    { static obj c[] = { obj_from_case(11) }; cx_iota_23869 = (obj)c; }
    cx__23895 = (hpushstr(0, newstring("    { obj* p = objptr_from_obj(r[0]);")));
    cx__23929 = (hpushstr(0, newstring(": /* load module */")));
    cx__23948 = (hpushstr(0, newstring("no curry for")));
    { static char s[] = { 46, 10, 0 };
    cx__23971 = (hpushstr(0, newstring(s))); }
    cx__23975 = (hpushstr(0, newstring("no clause matches ~s")));
    cx__23977 = (hpushstr(0, newstring("Error: ")));
    cx__23981 = (hpushstr(0, newstring("Error in ~a: ")));
    cx__231232 = (hpushstr(0, newstring("goto args mismatch??")));
    cx__231253 = (hpushstr(0, newstring("curry args mismatch??")));
    cx__231266 = (hpushstr(0, newstring("curry ids mismatch??")));
    cx__231275 = (hpushstr(0, newstring("gs_")));
    cx__231310 = (hpushstr(0, newstring("can't find a label for")));
    cx__231322 = (hpushstr(0, newstring("s_")));
    cx__231342 = (hpushstr(0, newstring("_v")));
    { static char s[] = { 36, 97, 114, 103, 59, 0 };
    cx__231412 = (hpushstr(0, newstring(s))); }
    cx__231421 = (hpushstr(0, newstring("(void)(0);")));
    cx__231485 = (hpushstr(0, newstring("obj")));
    { static char s[] = { 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 41, 0 };
    cx__231495 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 32, 58, 32, 0 };
    cx__231498 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 40, 98, 111, 111, 108, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 32, 63, 32, 0 };
    cx__231503 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 40, 98, 111, 111, 108, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 32, 124, 124, 32, 0 };
    cx__231514 = (hpushstr(0, newstring(s))); }
    cx__231521 = (hpushstr(0, newstring("bool")));
    { static char s[] = { 40, 98, 111, 111, 108, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 32, 38, 38, 32, 0 };
    cx__231530 = (hpushstr(0, newstring(s))); }
    cx__231535 = (hpushstr(0, newstring("bool(0)")));
    cx__231553 = (hpushstr(0, newstring("escaping label???")));
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
    cx__231769 = (hendblk(3)); }
    { static obj c[] = { obj_from_case(13) }; cx_halt_2Dexp_3F_231934 = (obj)c; }
    cx__232052 = (hpushstr(0, newstring("    goto jump;")));
    { static char s[] = { 32, 32, 32, 32, 97, 115, 115, 101, 114, 116, 40, 114, 99, 32, 61, 32, 36, 97, 114, 103, 35, 41, 59, 10, 0 };
    cx__232053 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 114, 114, 101, 115, 101, 114, 118, 101, 40, 77, 65, 88, 95, 76, 73, 86, 69, 82, 69, 71, 83, 41, 59, 10, 0 };
    cx__232056 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 36, 125, 114, 32, 43, 61, 32, 36, 108, 105, 118, 101, 59, 32, 47, 42, 32, 115, 104, 105, 102, 116, 32, 114, 101, 103, 32, 119, 110, 100, 32, 42, 47, 10, 0 };
    cx__232059 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 36, 123, 114, 91, 36, 108, 105, 118, 101, 43, 36, 97, 114, 103, 35, 93, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 10, 0 };
    cx__232062 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 112, 99, 32, 61, 32, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 114, 91, 36, 108, 105, 118, 101, 43, 48, 93, 41, 91, 48, 93, 59, 10, 0 };
    cx__232065 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 114, 91, 36, 108, 105, 118, 101, 43, 48, 93, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 10, 0 };
    cx__232068 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 36, 125, 114, 114, 101, 115, 101, 114, 118, 101, 40, 77, 65, 88, 95, 76, 73, 86, 69, 82, 69, 71, 83, 41, 59, 10, 0 };
    cx__232080 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 36, 123, 114, 91, 36, 97, 114, 103, 35, 93, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 10, 0 };
    cx__232083 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 112, 99, 32, 61, 32, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 114, 91, 48, 93, 41, 91, 48, 93, 59, 10, 0 };
    cx__232086 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 114, 91, 48, 93, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 10, 0 };
    cx__232089 = (hpushstr(0, newstring(s))); }
    cx__232110 = (hpushstr(0, newstring(";")));
    cx__232113 = (hpushstr(0, newstring("    goto ")));
    { static char s[] = { 36, 123, 114, 91, 36, 108, 105, 118, 101, 43, 36, 97, 114, 103, 35, 93, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 10, 0 };
    cx__232120 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 36, 125, 103, 111, 116, 111, 32, 0 };
    cx__232130 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 36, 123, 114, 91, 36, 97, 114, 103, 35, 93, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 10, 0 };
    cx__232133 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 36, 114, 101, 116, 117, 114, 110, 32, 111, 98, 106, 40, 104, 101, 110, 100, 98, 108, 107, 40, 36, 97, 114, 103, 99, 43, 49, 41, 41, 59, 0 };
    cx__232197 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 36, 125, 42, 45, 45, 104, 112, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 99, 97, 115, 101, 40, 0 };
    cx__232203 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 32, 32, 32, 36, 123, 42, 45, 45, 104, 112, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 10, 0 };
    cx__232206 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 104, 114, 101, 115, 101, 114, 118, 101, 40, 104, 98, 115, 122, 40, 36, 97, 114, 103, 99, 43, 49, 41, 44, 32, 36, 108, 105, 118, 101, 41, 59, 32, 47, 42, 32, 36, 108, 105, 118, 101, 32, 108, 105, 118, 101, 32, 114, 101, 103, 115, 32, 42, 47, 10, 0 };
    cx__232209 = (hpushstr(0, newstring(s))); }
    cx__232501 = (hpushstr(0, newstring("}")));
    cx__232508 = (hpushstr(0, newstring("} else {")));
    cx__232517 = (hpushstr(0, newstring("])) {")));
    cx__232523 = (hpushstr(0, newstring("if (bool_from_obj(r[")));
    { static char s[] = { 105, 102, 32, 40, 98, 111, 111, 108, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 41, 32, 123, 0 };
    cx__232537 = (hpushstr(0, newstring(s))); }
    cx__232563 = (hpushstr(0, newstring("void(0)")));
    cx__232584 = (hpushstr(0, newstring(" = (obj)c; }")));
    cx__232589 = (hpushstr(0, newstring(") }; ")));
    cx__232594 = (hpushstr(0, newstring("static obj c[] = { obj_from_case(")));
    cx__232597 = (hpushstr(0, newstring("{ ")));
    cx__232865 = (hpushstr(0, newstring(" }")));
    cx__232882 = (hpushstr(0, newstring("_t ")));
    cx__232896 = (hpushstr(0, newstring(" } ")));
    cx__232917 = (hpushstr(0, newstring("{ const ")));
    cx__232922 = (hpushstr(0, newstring(" = ")));
    cx__232953 = (hpushstr(0, newstring("")));
    cx__232961 = (hpushstr(0, newstring("_")));
    cx__232977 = (hpushstr(0, newstring("v")));
    cx__233021 = (hpushstr(0, newstring("invalid lval cg target")));
    { static char s[] = { 114, 91, 36, 97, 93, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 0 };
    cx__233030 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 36, 97, 32, 61, 32, 36, 97, 95, 102, 114, 111, 109, 95, 0 };
    cx__233040 = (hpushstr(0, newstring(s))); }
    cx__233041 = (hpushstr(0, newstring(" = obj_from_")));
    cx__233049 = (hpushstr(0, newstring("void_from_")));
    cx__233054 = (hpushstr(0, newstring("invalid rval cg target")));
    { static char s[] = { 111, 98, 106, 40, 114, 91, 36, 97, 93, 41, 0 };
    cx__233063 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 36, 97, 40, 36, 97, 44, 32, 36, 97, 41, 0 };
    cx__233076 = (hpushstr(0, newstring(s))); }
    cx__233077 = (hpushstr(0, newstring(")")));
    cx__233080 = (hpushstr(0, newstring("obj(")));
    cx__233088 = (hpushstr(0, newstring("void")));
    cx__233187 = (hpushstr(0, newstring("a")));
    cx__233188 = (hpushstr(0, newstring("z")));
    cx__233210 = (hpushstr(0, newstring("/* live regs: ")));
    cx__233224 = (hpushstr(0, newstring(".")));
    { static char s[] = { 10, 32, 32, 0 };
    cx__233234 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 32, 32, 32, 32, 0 };
    cx__233235 = (hpushstr(0, newstring(s))); }
    cx__233273 = (hpushstr(0, newstring("#define MAX_LIVEREGS ")));
    { static obj c[] = { obj_from_case(14) }; cx_rassq = (obj)c; }
    { static obj c[] = { obj_from_case(15) }; cx_merge_2Dby_21 = (obj)c; }
    { static obj c[] = { obj_from_case(19) }; cx_split_2Dby_21 = (obj)c; }
    { static obj c[] = { obj_from_case(20) }; cx_sort_2Dby_21 = (obj)c; }
    { static obj c[] = { obj_from_case(23) }; cx_sort_2Dby = (obj)c; }
    { static obj c[] = { obj_from_case(24) }; cx_merge_2Dby = (obj)c; }
    { static obj c[] = { obj_from_case(25) }; cx_bgc_2Dexp = (obj)c; }
    { static obj c[] = { obj_from_case(28) }; cx_bgc_2Dexp_3F = (obj)c; }
    { static obj c[] = { obj_from_case(31) }; cx_bgc_2Dexp_2D_3Eid = (obj)c; }
    { static obj c[] = { obj_from_case(32) }; cx_bgc_2Dexp_2D_3Elbid = (obj)c; }
    { static obj c[] = { obj_from_case(33) }; cx_bgc_2Dexp_2D_3Eids = (obj)c; }
    { static obj c[] = { obj_from_case(34) }; cx_bgc_2Dexp_2D_3Eexp2 = (obj)c; }
    { static obj c[] = { obj_from_case(35) }; cx_code_2Dgenerate = (obj)c; }
    r[0] = obj_from_void(0);
    r[1+0] = r[0];
    pc = 0; /* exit from module init */
    r[1+1] = r[0];  
    r += 1; /* shift reg wnd */
    assert(rc = 2);
    goto jump;

case 1: /* curry-exp#81 k id ids rands */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_curry_2Dexp_2381: /* k id ids rands */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(3);
    r[4] = (hendblk(4+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop;

s_loop: /* k id */
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
    *--hp = obj_from_case(2);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop;
  }

case 2: /* clo ek r */
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

case 3: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k ids id rands */
    { fixnum_t v7825_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7825_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v7825_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[1];   t = r[5];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[6] = (o); } }
    { /* vector */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(2+1)); }
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
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 4: /* curry-exp?#213 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_curry_2Dexp_3F_23213: /* k exp */
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
    *--hp = obj_from_case(5);
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

case 5: /* clo ek r */
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
    goto s_loop_v7754;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop_v7754: /* k ids rands */
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
    goto s_loop_v7754;
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

case 6: /* begin-exp?#301 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_begin_2Dexp_3F_23301: /* k exp */
  if (bool_from_obj(cxs_let_2Dexp_3F_23331((r[1])))) {
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
    *--hp = obj_from_case(7);
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

case 7: /* clo ek r */
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

case 8: /* keep#510 k f lst */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_keep_23510: /* k f lst */
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
    *--hp = obj_from_case(8);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(9);
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

case 9: /* clo ek r */
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
    *--hp = obj_from_case(10);
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

case 10: /* clo ek r */
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

case 11: /* iota#869 k n m */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_iota_23869: 
s_iota: /* k n m */
  if ((fixnum_from_obj(r[1]) < fixnum_from_obj(r[2]))) {
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(12);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = obj_from_fixnum(fixnum_from_obj(r[1]) + (1));
    /* r[2] */    
    goto s_iota;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 12: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k n */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
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

case 13: /* halt-exp?#1934 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_halt_2Dexp_3F_231934: /* k exp */
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
    r[2] = obj_from_bool((r[2]) == (cx_the_2Dhalt_2Dprim));
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

case 14: /* rassq k x al */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k x al */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_rassq((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 15: /* merge-by! k less? lst1 lst2 */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_merge_2Dby_21: /* k less? lst1 lst2 */
  if ((!ispair((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((!ispair((r[3])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(16);
    r[4] = (hendblk(4+1));
    r[5+0] = r[1];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[3])));
    r[5+3] = (car((r[2])));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }

case 16: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r lst1 less? k lst2 */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(17);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[3];  
    /* r[2] */    
    r[3] = (cdr((r[5])));
    goto gs_merge_2Dby_21;
  } else {
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(18);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[3];  
    r[2] = (cdr((r[2])));
    r[3] = r[5];  
    goto gs_merge_2Dby_21;
  }

case 17: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k lst2 */
    (void)(cdr((r[3])) = (r[1]));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 18: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k lst1 */
    (void)(cdr((r[3])) = (r[1]));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 19: /* split-by! k lst */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k lst */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_split_2Dby_21((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 20: /* sort-by! k less? lst */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_sort_2Dby_21: /* k less? lst */
  if ((isnull((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[2])));
    r[3] = obj_from_bool(isnull((r[3])));
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cxs_split_2Dby_21((r[2])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(21);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    /* r[1] */    
    /* r[2] */    
    goto gs_sort_2Dby_21;
  }
  }

case 21: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r lst2 less? k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(22);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto gs_sort_2Dby_21;

case 22: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r less? k */
    r[5+0] = r[4];  
    r[5+1] = r[3];  
    r[5+2] = r[2];  
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_merge_2Dby_21;

case 23: /* sort-by k less? lst */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_sort_2Dby: /* k less? lst */
    { fixnum_t v7824_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7824_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v7824_tmp);
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    l = r[2];   t = (mknull()); /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[3] = (o); } }
    /* r[0] */    
    /* r[1] */    
    r[2] = r[3];  
    goto gs_sort_2Dby_21;

case 24: /* merge-by k less? lst1 lst2 */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k less? lst1 lst2 */
    { fixnum_t v7823_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7823_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v7823_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[2];   t = (mknull()); /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[4] = (o); } }
    { fixnum_t v7822_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7822_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v7822_tmp);
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    l = r[3];   t = (mknull()); /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[5] = (o); } }
    /* r[0] */    
    /* r[1] */    
    r[2] = r[4];  
    r[3] = r[5];  
    goto gs_merge_2Dby_21;

case 25: /* bgc-exp k id lbid ids exp2 */
    assert(rc == 6);
    r += 1; /* shift reg. wnd */
    /* k id lbid ids exp2 */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = obj_from_case(26);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[2];  
    r[2] = r[3];  
    r[3] = (mknull());
    goto gs_curry_2Dexp_2381;

case 26: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp2 id */
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(3+1)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(27);
    r[6] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 27: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r exp2 */
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

case 28: /* bgc-exp? k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_bgc_2Dexp_3F: /* k exp */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(29);
    r[2] = (hendblk(2+1));
    r[0] = r[2];  
    /* r[1] */    
    goto gs_begin_2Dexp_3F_23301;

case 29: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp */
  if (bool_from_obj(r[1])) {
    r[4] = (vectorref((r[3]), (2)));
    r[4] = (car((r[4])));
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_23274((r[4])))) {
    r[5] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(30);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[5];  
    goto gs_curry_2Dexp_3F_23213;
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

case 30: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp */
  if (bool_from_obj(r[1])) {
    { fixnum_t v7821_tmp;
    r[4] = (vectorref((r[3]), (2)));
    r[4] = (vectorref((r[4]), (2)));
    r[5] = (vectorref((r[3]), (1)));
    { /* length */
    int n; obj l = r[5];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7821_tmp = (n); }
    { /* list-tail */
    obj l = r[4];   int c = (v7821_tmp);
    while (c-- > 0) l = cdr(l);
    r[4] = (l); } }
    r[4] = obj_from_bool(isnull((r[4])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
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

case 31: /* bgc-exp->id k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    r[2] = (vectorref((r[1]), (2)));
    r[2] = (car((r[2])));
    r[2] = (vectorref((r[2]), (1)));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 32: /* bgc-exp->lbid k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    r[2] = (vectorref((r[1]), (2)));
    r[2] = (car((r[2])));
    r[2] = (vectorref((r[2]), (2)));
    r[3] = (vectorref((r[2]), (2)));
    r[3] = (vectorref((r[3]), (1)));
    r[2] = (vectorref((r[3]), (1)));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 33: /* bgc-exp->ids k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    r[2] = (vectorref((r[1]), (2)));
    r[2] = (car((r[2])));
    r[2] = (vectorref((r[2]), (2)));
    r[2] = (vectorref((r[2]), (1)));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 34: /* bgc-exp->exp2 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    r[2] = (vectorref((r[1]), (1)));
    r[2] = (vectorref((r[2]), (2)));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 35: /* code-generate k input-fix-exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k input-fix-exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(36);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(1), 3); /* 3 live regs */
    *--hp = obj_from_void(0);
    r[3] = (hendblk(1));
    hreserve(hbsz(1), 4); /* 4 live regs */
    *--hp = obj_from_void(0);
    r[4] = (hendblk(1));
    hreserve(hbsz(1), 5); /* 5 live regs */
    *--hp = obj_from_void(0);
    r[5] = (hendblk(1));
    hreserve(hbsz(1), 6); /* 6 live regs */
    *--hp = obj_from_void(0);
    r[6] = (hendblk(1));
    hreserve(hbsz(1), 7); /* 7 live regs */
    *--hp = obj_from_void(0);
    r[7] = (hendblk(1));
    hreserve(hbsz(1), 8); /* 8 live regs */
    *--hp = obj_from_void(0);
    r[8] = (hendblk(1));
    hreserve(hbsz(1), 9); /* 9 live regs */
    *--hp = obj_from_void(0);
    r[9] = (hendblk(1));
    hreserve(hbsz(1), 10); /* 10 live regs */
    *--hp = obj_from_void(0);
    r[10] = (hendblk(1));
    hreserve(hbsz(1), 11); /* 11 live regs */
    *--hp = obj_from_void(0);
    r[11] = (hendblk(1));
    hreserve(hbsz(1), 12); /* 12 live regs */
    *--hp = obj_from_void(0);
    r[12] = (hendblk(1));
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(43);
    r[13] = (hendblk(2+1));
    hreserve(hbsz(1+1), 14); /* 14 live regs */
    *--hp = (r[11]);
    *--hp = obj_from_case(44);
    r[14] = (hendblk(1+1));
    hreserve(hbsz(1+1), 15); /* 15 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(42);
    r[15] = (hendblk(1+1));
    hreserve(hbsz(0+1), 16); /* 16 live regs */
    *--hp = obj_from_case(37);
    r[16] = (hendblk(0+1));
    hreserve(hbsz(1+1), 17); /* 17 live regs */
    *--hp = (r[11]);
    *--hp = obj_from_case(46);
    r[17] = (hendblk(1+1));
    hreserve(hbsz(7+1), 18); /* 18 live regs */
    *--hp = (r[15]);
    *--hp = (r[16]);
    *--hp = (r[17]);
    *--hp = r[3];  
    *--hp = (r[13]);
    *--hp = r[9];  
    *--hp = r[2];  
    *--hp = obj_from_case(47);
    r[15] = (hendblk(7+1));
    hreserve(hbsz(16+1), 16); /* 16 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = (r[12]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = r[0];  
    *--hp = (r[15]);
    *--hp = r[2];  
    *--hp = obj_from_case(211);
    r[14] = (hendblk(16+1));
    r[15+0] = (cx_exp_2D_3Efree_2Dvars);
    pc = objptr_from_obj(r[15+0])[0];
    r[15+1] = (r[14]);
    r[15+2] = r[1];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 36: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k id */
    r[2] = (cdr((r[1])));
    r[2] = (car((r[2])));
    r[2] = (hpushstr(3, newstring(symbolname(getsymbol((r[2]))))));
    r[3+0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[0];  
    r[3+2] = r[2];  
    r[3+3] = (cx__231275);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 37: /* clo k renv */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k renv */
  if (bool_from_obj(mknull())) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(38);
    r[2] = (hendblk(1+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(39);
    r[3] = (hendblk(0+1));
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__23718);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = (cx_reduce_2Dleft);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[2];  
    r[5+2] = r[3];  
    r[5+3] = r[4];  
    r[5+4] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }

case 38: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 3); /* 3 live regs */
    p = r[1];   /* gc-safe */
    *--hp = p; *--hp = (cx__233210);
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (cx__233235);
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[3] = (p); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 39: /* clo k id lst */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id lst */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(40);
    r[3] = (hendblk(2+1));
  if (((is_fixnum_obj(r[1])) || (is_flonum_obj(r[1])))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__23729);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (cx__233224);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = obj_from_ktrap();
    r[5+1] = r[4];  
    r[5+2] = r[0];  
    r[5+3] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7523;
  } else {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(41);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 40: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v7523: /* ek r k lst */
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

case 41: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__23729);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
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

case 42: /* clo k id renv */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id renv max-live */
    { fixnum_t v7820_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7820_tmp = (n); }
    r[4] = obj_from_fixnum((1) + (v7820_tmp)); }
    r[5] = (objptr_from_obj(r[3])[0]);
    r[4] = ((fixnum_from_obj(r[4]) > fixnum_from_obj(r[5])) ? (r[4]) : (r[5]));
    (void)(objptr_from_obj(r[3])[0] = (r[4]));
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 43: /* clo k id ref? */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k id ref? label-alist used-labels */
  if (bool_from_obj(r[2])) {
    r[5] = (objptr_from_obj(r[4])[0]);
    { /* memq */
    obj x = (r[1]), l = r[5];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[6] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[6])) {
    /* r[5] */    
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
  }
    (void)(objptr_from_obj(r[4])[0] = (r[5]));
  } else {
  }
    r[5] = (objptr_from_obj(r[3])[0]);
    { /* assq */
    obj x = (r[1]), l = (r[5]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[5] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[5])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cdr((r[5])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[0];  
    r[7+2] = (cx__231310);
    r[7+3] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 44: /* clo k id ids rands */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id ids rands curry-alist */
    r[5] = (objptr_from_obj(r[4])[0]);
    { /* assq */
    obj x = (r[1]), l = (r[5]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[5] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[5])) {
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(45);
    r[6] = (hendblk(5+1));
    { bool_t v7819_tmp;
    { fixnum_t v7818_tmp;
    { fixnum_t v7817_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7818_tmp = (n); }
    r[7] = (cdr((r[5])));
    r[7] = (car((r[7])));
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7817_tmp = (n); }
    v7819_tmp = ((v7818_tmp) == (v7817_tmp)); } }
    r[7] = obj_from_bool(!(v7819_tmp)); }
  if (bool_from_obj(r[7])) {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[6];  
    r[8+2] = (cx__231266);
    r[8+3] = r[7];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[7+0] = obj_from_ktrap();
    r[7+1] = obj_from_void(0);
    r[7+2] = r[1];  
    r[7+3] = r[2];  
    r[7+4] = r[0];  
    r[7+5] = r[5];  
    r[7+6] = r[3];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7483;
  }
  } else {
    r[6] = (objptr_from_obj(r[4])[0]);
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 7); /* 7 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[7] = (p); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[6] = (objptr_from_obj(r[4])[0] = (r[6]));
    r[7+0] = r[0];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 45: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
s_l_v7483: /* ek  id ids k tmp rands */
    { bool_t v7816_tmp;
    { fixnum_t v7815_tmp;
    { fixnum_t v7814_tmp;
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7815_tmp = (n); }
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7814_tmp = (n); }
    v7816_tmp = ((v7815_tmp) == (v7814_tmp)); } }
    r[7] = obj_from_bool(!(v7816_tmp)); }
  if (bool_from_obj(r[7])) {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[4];  
    r[8+2] = (cx__231253);
    r[8+3] = r[7];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 46: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id curry-alist */
    r[3] = (objptr_from_obj(r[2])[0]);
    r[4+0] = r[0];  
    r[4+1] = r[3];  
    r[4+2] = obj_from_fixnum(1);
    r[4+3] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v7469;

s_loop_v7469: /* k alst i id */
  if ((isnull((r[1])))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = (cx__23948);
    r[5+3] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[4] = (car((r[1])));
    r[4] = (car((r[4])));
    r[4] = obj_from_bool((r[4]) == (r[3]));
  if (bool_from_obj(r[4])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[4];  
    r[2] = obj_from_fixnum(fixnum_from_obj(r[2]) + (1));
    /* r[3] */    
    goto s_loop_v7469;
  }
  }

case 47: /* clo k exp register-env */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4];
    r[1+7] = p[5];
    r[1+8] = p[6];
    r[1+9] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* k exp register-env make-gclabel global-constant-labels lookup-label max-live lookup-curry-case-no nl/regs-comment alloc-reg */
    hreserve(hbsz(0+1), 10); /* 10 live regs */
    *--hp = obj_from_case(48);
    r[10] = (hendblk(0+1));
    hreserve(hbsz(0+1), 11); /* 11 live regs */
    *--hp = obj_from_case(51);
    r[11] = (hendblk(0+1));
    hreserve(hbsz(1), 12); /* 12 live regs */
    *--hp = obj_from_void(0);
    r[12] = (hendblk(1));
    hreserve(hbsz(1), 13); /* 13 live regs */
    *--hp = obj_from_void(0);
    r[13] = (hendblk(1));
    hreserve(hbsz(1), 14); /* 14 live regs */
    *--hp = obj_from_void(0);
    r[14] = (hendblk(1));
    hreserve(hbsz(2+1), 15); /* 15 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = obj_from_case(52);
    r[15] = (hendblk(2+1));
    hreserve(hbsz(2+1), 16); /* 16 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = obj_from_case(56);
    r[16] = (hendblk(2+1));
    hreserve(hbsz(0+1), 17); /* 17 live regs */
    *--hp = obj_from_case(60);
    r[17] = (hendblk(0+1));
    hreserve(hbsz(14+1), 18); /* 18 live regs */
    *--hp = (r[15]);
    *--hp = (r[16]);
    *--hp = r[9];  
    *--hp = (r[13]);
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = (r[17]);
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = (r[14]);
    *--hp = obj_from_case(113);
    r[18] = (hendblk(14+1));
    hreserve(hbsz(7+1), 19); /* 19 live regs */
    *--hp = (r[12]);
    *--hp = (r[13]);
    *--hp = (r[14]);
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = (r[18]);
    *--hp = obj_from_case(84);
    r[18] = (hendblk(7+1));
    hreserve(hbsz(1+1), 19); /* 19 live regs */
    *--hp = (r[14]);
    *--hp = obj_from_case(102);
    r[19] = (hendblk(1+1));
    r[20+0] = (cx_memoizate_2Dexp_2Dattribute);
    pc = objptr_from_obj(r[20+0])[0];
    r[20+1] = (r[18]);
    r[20+2] = (r[19]);
    r += 20; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 48: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k id */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(49);
    r[2] = (hendblk(2+1));
    r[3] = (cdr((r[1])));
    r[3] = (car((r[3])));
    r[4+0] = (cx_c_2Dundecorate_2Dalvar);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[2];  
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 49: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(50);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (cdr((r[5])));
    r[5] = (car((r[5])));
    r[5] = obj_from_bool(is_fixnum_obj(r[5]));
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[2])));
    r[5] = (cdr((r[5])));
    r[5] = (car((r[5])));
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[5];  
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[5] = (cdr((r[2])));
    r[5] = (cdr((r[5])));
    r[5] = (car((r[5])));
    r[0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 50: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__232961)));
    r[4] = (hpushstr(4, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232977)), stringdata((r[4])));
    r[5] = (hpushstr(5, d)); }
    r[0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    /* r[2] */    
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 51: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k id */
    r[2] = (cdr((r[1])));
    r[2] = (car((r[2])));
    r[2] = (hpushstr(3, newstring(symbolname(getsymbol((r[2]))))));
    r[3+0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[0];  
    r[3+2] = r[2];  
    r[3+3] = (cx__23525);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 52: /* clo k tgt */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k tgt cvar-id->c-name gvar-id->c-name */
  if ((!bool_from_obj(r[1]))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__233088);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((is_fixnum_obj(r[1]))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = (cx__233063);
    r[5+3] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((ispair((r[1])))) {
    r[4] = (car((r[1])));
    r[5] = (cdr((r[1])));
    r[6] = (cdr((r[4])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[6] = obj_from_bool(fixnum_from_obj(r[6]) < (0));
  if (bool_from_obj(r[6])) {
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(53);
    r[6] = (hendblk(1+1));
    r[7+0] = r[3];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[4];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_cvar_2Did_3F_232940((r[4])))) {
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = obj_from_case(54);
    r[6] = (hendblk(3+1));
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[4];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = obj_from_case(55);
    r[6] = (hendblk(2+1));
    r[7+0] = (cx_posq);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[4];  
    r[7+3] = r[5];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = (cx__233054);
    r[5+3] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }
  }

case 53: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__233077)));
    r[3] = (hpushstr(3, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__233080)), stringdata((r[3])));
    r[4] = (hpushstr(4, d)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 54: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id renv */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[6] = obj_from_fixnum(n); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7] = (cdr((r[3])));
    r[7] = (cdr((r[7])));
    r[7] = (cdr((r[7])));
    r[7] = (car((r[7])));
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = (cx__233076);
    r[3] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 55: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k renv */
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[4] = obj_from_fixnum(n); }
    r[4] = obj_from_fixnum(fixnum_from_obj(r[4]) - (fixnum_from_obj(r[1]) + (1)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = (cx__233063);
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 56: /* clo k tgt */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k tgt cvar-id->c-name gvar-id->c-name */
  if ((!bool_from_obj(r[1]))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__233049);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((is_fixnum_obj(r[1]))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = (cx__233030);
    r[5+3] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((ispair((r[1])))) {
    r[4] = (car((r[1])));
    r[5] = (cdr((r[1])));
    r[6] = (cdr((r[4])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[6] = obj_from_bool(fixnum_from_obj(r[6]) < (0));
  if (bool_from_obj(r[6])) {
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(57);
    r[6] = (hendblk(1+1));
    r[7+0] = r[3];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[4];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_cvar_2Did_3F_232940((r[4])))) {
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = obj_from_case(58);
    r[6] = (hendblk(2+1));
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[4];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = obj_from_case(59);
    r[6] = (hendblk(2+1));
    r[7+0] = (cx_posq);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[4];  
    r[7+3] = r[5];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = (cx__233021);
    r[5+3] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }
  }

case 57: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__233041)));
    r[3] = (hpushstr(3, d)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 58: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (cdr((r[3])));
    r[4] = (cdr((r[4])));
    r[4] = (cdr((r[4])));
    r[4] = (car((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = (cx__233040);
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 59: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k renv */
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[4] = obj_from_fixnum(n); }
    r[4] = obj_from_fixnum(fixnum_from_obj(r[4]) - (fixnum_from_obj(r[1]) + (1)));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = (cx__233030);
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 60: /* clo k renv rands */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k renv rands */
    { fixnum_t v7813_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7813_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v7813_tmp);
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    l = r[1];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[3] = (o); } }
    /* r[0] */    
    r[1] = r[2];  
    r[2] = r[3];  
    r[3] = (mknull());
    goto s_loop_v7249;

case 61: /* clo k rands regs spoiled */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
s_loop_v7249: /* k rands regs spoiled */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isnull((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (car((r[2])));
    r[4] = obj_from_bool((is_fixnum_obj(r[4])) || (is_flonum_obj(r[4])));
  if (bool_from_obj(r[4])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(61);
    r[4] = (hendblk(0+1));
    hreserve(hbsz(5+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(62);
    r[4] = (hendblk(5+1));
  if ((isnull((r[3])))) {
    hreserve(hbsz(0+1), 5); /* 5 live regs */
    *--hp = obj_from_case(61);
    r[5] = (hendblk(0+1));
    r[6+0] = obj_from_ktrap();
    r[6+1] = obj_from_bool(isnull((r[3])));
    r[6+2] = r[5];  
    r[6+3] = r[3];  
    r[6+4] = r[2];  
    r[6+5] = r[1];  
    r[6+6] = r[0];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7370;
  } else {
    r[5] = (car((r[1])));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(63);
    r[6] = (hendblk(1+1));
    r[0] = r[6];  
    r[1] = r[5];  
    r[2] = r[3];  
    goto s_dep_3F;
  }
  }
  }
  }

case 62: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
s_l_v7370: /* ek r loop spoiled regs rands k */
  if (bool_from_obj(r[1])) {
    r[7] = (car((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8+0] = r[2];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[6];  
    r[8+2] = (cdr((r[5])));
    r[8+3] = (cdr((r[4])));
    r[8+4] = r[7];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
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

case 63: /* clo ek r */
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

case 64: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_dep_3F: /* k exp spoiled */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(64);
    r[3] = (hendblk(1+1));
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
    { /* memq */
    obj x = (r[4]), l = r[2];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[4] = (l == mknull() ? obj_from_bool(0) : l); }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_23274((r[1])))) {
    r[4] = (vectorref((r[1]), (2)));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_dep_3F;
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
    r[4] = (vectorref((r[1]), (3)));
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (1)));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[7];  
    r[2] = r[3];  
    goto s_loop_v7348;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23331((r[1])))) {
    r[4] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[4]), (2)));
    r[5] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(69);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[5];  
    r[2] = r[3];  
    goto s_loop_v7337;
  } else {
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(70);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    /* r[1] */    
    goto gs_curry_2Dexp_3F_23213;
  }
  }
  }
  }

case 65: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7348: /* k id dep? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(65);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(66);
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

case 66: /* clo ek r */
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

case 67: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7337: /* k id dep? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(67);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(68);
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

case 68: /* clo ek r */
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

case 69: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r dep? body k */
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

case 70: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r dep? k exp */
  if (bool_from_obj(r[1])) {
    { fixnum_t v7812_tmp;
    r[5] = (vectorref((r[4]), (2)));
    r[5] = (vectorref((r[5]), (2)));
    r[6] = (vectorref((r[4]), (1)));
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7812_tmp = (n); }
    { /* list-tail */
    obj l = r[5];   int c = (v7812_tmp);
    while (c-- > 0) l = cdr(l);
    r[5] = (l); } }
    r[0] = r[3];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_loop_v7316;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(73);
    r[5] = (hendblk(3+1));
    r[0] = (cx_labelapp_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 71: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7316: /* k id dep? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(71);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(72);
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

case 72: /* clo ek r */
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

case 73: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r dep? k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[0] = r[3];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_loop_v7301;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (3))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (2)));
    r[6] = (vectorref((r[4]), (1)));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(76);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (4))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (3)));
    r[0] = r[3];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_loop_v7273;
  } else {
    r[5] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[5]))));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(81);
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
    r[3] = (cx__23981);
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__23977);
    r[3] = obj_from_bool(1);
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }

case 74: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7301: /* k id dep? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(74);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(75);
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

case 75: /* clo ek r */
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

case 76: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r dep? rands k */
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
    goto s_loop_v7286;
  }

case 77: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7286: /* k id dep? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(77);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(78);
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

case 78: /* clo ek r */
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

case 79: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7273: /* k id dep? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(79);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(80);
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

case 80: /* clo ek r */
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

case 81: /* clo ek  */
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
    *--hp = obj_from_case(82);
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
    r[7+3] = (cx__23975);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 82: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(83);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23971);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 83: /* clo ek  */
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

case 84: /* clo ek r */
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
    /* ek r cg-to exp register-env k cg-cexp? cg-gcsafe-cexp? cg-returns? */
    hreserve(hbsz(8+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(85);
    r[9] = (hendblk(8+1));
    hreserve(hbsz(1+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = obj_from_case(91);
    r[10] = (hendblk(1+1));
    r[0] = (cx_memoizate_2Dexp_2Dattribute);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
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
    /* ek r cg-to exp register-env k cg-cexp? r cg-gcsafe-cexp? cg-returns? */
    hreserve(hbsz(9+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(86);
    r[10] = (hendblk(9+1));
    hreserve(hbsz(1+1), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = obj_from_case(87);
    r[11] = (hendblk(1+1));
    r[0] = (cx_memoizate_2Dexp_2Dattribute);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 86: /* clo ek r */
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
    /* ek r cg-to exp register-env k cg-cexp? r cg-gcsafe-cexp? r cg-returns? */
    (void)(objptr_from_obj(r[10])[0] = (r[1]));
    (void)(objptr_from_obj(r[8])[0] = (r[9]));
    (void)(objptr_from_obj(r[6])[0] = (r[7]));
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[11] = obj_from_fixnum(n); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (r[11]);
    /* r[3] */    
    /* r[4] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 87: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k exp cg-returns? */
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
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_23274((r[1])))) {
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
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(88);
    r[5] = (hendblk(3+1));
    r[6+0] = (objptr_from_obj(r[2])[0]);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = r[3];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23331((r[1])))) {
    r[3] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[3]), (2)));
    r[4+0] = (objptr_from_obj(r[2])[0]);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[0];  
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(89);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    /* r[1] */    
    goto gs_curry_2Dexp_3F_23213;
  }
  }
  }
  }

case 88: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-returns? else-exp k */
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
    r[0] = (objptr_from_obj(r[2])[0]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 89: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(90);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_halt_2Dexp_3F_231934;
  }

case 90: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (4))) {
    r[4] = (vectorref((r[2]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = obj_from_bool(1);
  } else {
    r[4] = obj_from_bool(0);
  }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 91: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k exp cg-gcsafe-cexp? */
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
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_23274((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
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
    goto s_loop_v7160;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23331((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(94);
    r[3] = (hendblk(3+1));
    r[0] = r[3];  
    /* r[1] */    
    goto gs_curry_2Dexp_3F_23213;
  }
  }
  }
  }

case 92: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7160: /* k id cg-gcsafe-cexp? */
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
    *--hp = obj_from_case(92);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(93);
    r[3] = (hendblk(3+1));
    r[4+0] = (objptr_from_obj(r[2])[0]);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 93: /* clo ek r */
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

case 94: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-gcsafe-cexp? exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(95);
    r[5] = (hendblk(3+1));
    r[0] = (cx_labelapp_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 95: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-gcsafe-cexp? exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (3))) {
    r[5] = (vectorref((r[3]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (4))) {
    r[5] = (vectorref((r[3]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[3]), (1)));
    r[6] = (vectorref((r[3]), (2)));
    r[7] = (vectorref((r[3]), (3)));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = r[2];  
    *--hp = obj_from_case(96);
    r[8] = (hendblk(4+1));
    r[0] = (cx_prim_2Dcexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[5]))));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_case(99);
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
    r[3] = (cx__23981);
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__23977);
    r[3] = obj_from_bool(1);
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }

case 96: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-gcsafe-cexp? rands k effect */
  if (bool_from_obj(r[1])) {
    { /* memq */
    obj x = (r[5]), l = (cx__231769);
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[6] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[6])) {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7137;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 97: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7137: /* k id cg-gcsafe-cexp? */
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
    *--hp = obj_from_case(97);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(98);
    r[3] = (hendblk(3+1));
    r[4+0] = (objptr_from_obj(r[2])[0]);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 98: /* clo ek r */
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

case 99: /* clo ek  */
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
    *--hp = obj_from_case(100);
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
    r[7+3] = (cx__23975);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 100: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(101);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23971);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 101: /* clo ek  */
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

case 102: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k exp cg-cexp? */
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
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_23274((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
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
    goto s_loop_v7094;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23331((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(105);
    r[3] = (hendblk(3+1));
    r[0] = r[3];  
    /* r[1] */    
    goto gs_curry_2Dexp_3F_23213;
  }
  }
  }
  }

case 103: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7094: /* k id cg-cexp? */
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
    *--hp = obj_from_case(103);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(104);
    r[3] = (hendblk(3+1));
    r[4+0] = (objptr_from_obj(r[2])[0]);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 104: /* clo ek r */
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

case 105: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp? exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(106);
    r[5] = (hendblk(3+1));
    r[0] = (cx_labelapp_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 106: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp? exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (3))) {
    r[5] = (vectorref((r[3]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (4))) {
    r[5] = (vectorref((r[3]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[3]), (2)));
    r[6] = (vectorref((r[3]), (3)));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(107);
    r[7] = (hendblk(3+1));
    r[0] = (cx_prim_2Dcexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[5]))));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_case(110);
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
    r[3] = (cx__23981);
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__23977);
    r[3] = obj_from_bool(1);
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }

case 107: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp? rands k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7071;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 108: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7071: /* k id cg-cexp? */
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
    *--hp = obj_from_case(108);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(109);
    r[3] = (hendblk(3+1));
    r[4+0] = (objptr_from_obj(r[2])[0]);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 109: /* clo ek r */
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

case 110: /* clo ek  */
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
    *--hp = obj_from_case(111);
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
    r[7+3] = (cx__23975);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 111: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(112);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23971);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 112: /* clo ek  */
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

case 113: /* clo k tgt exp renv */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6];
    r[1+10] = p[7];
    r[1+11] = p[8];
    r[1+12] = p[9];
    r[1+13] = p[10];
    r[1+14] = p[11];
    r[1+15] = p[12];
    r[1+16] = p[13];
    r[1+17] = p[14]; }
    r += 1; /* shift reg. wnd */
s_cg_2Dto: /* k tgt exp renv cg-cexp? gvar-id->c-name cvar-id->c-name make-gclabel global-constant-labels cg-fast-goto? lookup-label lookup-curry-case-no max-live nl/regs-comment cg-gcsafe-cexp? alloc-reg cg-tgt-lval cg-tgt-rval */
    hreserve(hbsz(2+1), 18); /* 18 live regs */
    *--hp = (r[17]);
    *--hp = (r[16]);
    *--hp = obj_from_case(118);
    r[18] = (hendblk(2+1));
    hreserve(hbsz(14+1), 19); /* 19 live regs */
    *--hp = (r[17]);
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = (r[16]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(136);
    r[19] = (hendblk(14+1));
    hreserve(hbsz(14+1), 20); /* 20 live regs */
    *--hp = (r[17]);
    *--hp = (r[16]);
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
    *--hp = obj_from_case(162);
    r[20] = (hendblk(14+1));
    hreserve(hbsz(7+1), 21); /* 21 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = (r[18]);
    *--hp = (r[19]);
    *--hp = (r[20]);
    *--hp = obj_from_case(114);
    r[18] = (hendblk(7+1));
    r[19+0] = (objptr_from_obj(r[4])[0]);
    pc = objptr_from_obj(r[19+0])[0];
    r[19+1] = (r[18]);
    r[19+2] = r[2];  
    r += 19; /* shift reg wnd */
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
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-complex-to cg-application-to cg-cexp-to renv exp tgt k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[7];  
    r[3] = r[6];  
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(115);
    r[9] = (hendblk(6+1));
  if (bool_from_obj(cxs_let_2Dexp_3F_23331((r[6])))) {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    /* r[2] */    
    /* r[3] */    
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    goto s_l_v7004;
  } else {
    hreserve(hbsz(2+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[6];  
    *--hp = obj_from_case(116);
    r[10] = (hendblk(2+1));
    r[0] = (r[10]);
    r[1] = r[6];  
    goto gs_curry_2Dexp_3F_23213;
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
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
s_l_v7004: /* ek r cg-complex-to cg-application-to renv exp tgt k */
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    r[3] = r[5];  
    /* r[4] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    r[3] = r[5];  
    /* r[4] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }

case 116: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(117);
    r[4] = (hendblk(2+1));
    r[0] = (cx_labelapp_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 117: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (3))) {
    r[4] = (vectorref((r[2]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = obj_from_bool(1);
  } else {
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (4))) {
    r[4] = (vectorref((r[2]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = obj_from_bool(1);
  } else {
    r[4] = obj_from_bool(0);
  }
  }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 118: /* clo k tgt exp renv */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k tgt exp renv cg-tgt-lval cg-tgt-rval */
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = obj_from_case(123);
    r[6] = (hendblk(1+1));
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = obj_from_case(119);
    r[6] = (hendblk(4+1));
    r[7+0] = r[4];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 119: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp renv exp k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__231412)));
    r[6] = (hpushstr(6, d)); }
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(120);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 120: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(121);
    r[5] = (hendblk(1+1));
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 121: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(122);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_cleanup_2Dc_2Dcode_21);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 122: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
  if ((isequal((r[1]), (cx__231421)))) {
    r[3] = (mknull());
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx__233235);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
  }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 123: /* clo k exp renv */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_cg_2Dcexp: /* k exp renv cg-tgt-rval */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(123);
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
    r[5] = (vectorref((r[1]), (1)));
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(124);
    r[6] = (hendblk(4+1));
  if (bool_from_obj(cxs_label_2Did_3F_231554((r[5])))) {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__231553);
    r[3] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[7+0] = obj_from_ktrap();
    r[7+1] = obj_from_void(0);
    r[7+2] = r[3];  
    r[7+3] = r[2];  
    r[7+4] = r[5];  
    r[7+5] = r[0];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6896;
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
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = obj_from_case(125);
    r[8] = (hendblk(5+1));
    r[0] = r[8];  
    r[1] = r[7];  
    /* r[2] */    
    /* r[3] */    
    goto s_cg_2Dcexp;
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
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(132);
    r[7] = (hendblk(3+1));
    r[8+0] = r[7];  
    r[8+1] = r[6];  
    r[8+2] = r[4];  
    r[8+3] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v6848;
  } else {
    r[5] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[5]))));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_case(133);
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
    r[3] = (cx__23981);
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__23977);
    r[3] = obj_from_bool(1);
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }

case 124: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
s_l_v6896: /* ek  cg-tgt-rval renv id k */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 125: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp cg-cexp renv test-exp k */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(126);
    r[7] = (hendblk(5+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    /* r[2] */    
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 126: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp renv test-exp k r */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_case(127);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 127: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k r */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(128);
    r[5] = (hendblk(4+1));
    r[0] = (cx_primexp_2Dctype);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 128: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r k r */
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(129);
    r[6] = (hendblk(5+1));
    r[0] = (cx_primexp_2Dctype);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 129: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r k r r */
    r[7] = ((bool_from_obj(r[1]) && (bool_from_obj(r[6]) && (strcmp(stringchars((r[1])), stringchars((r[6]))) == 0))) ? (r[1]) : (cx__231485));
  if (((strcmp(stringchars((r[7])), stringchars((cx__231521))) == 0) && (strcmp(stringchars((r[5])), stringchars((cx__231535))) == 0))) {
    { /* string-append */
    int *d = stringcat(stringdata((r[7])), stringdata((cx__231495)));
    r[8] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__231530)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[7])), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[4];  
    r[10+2] = r[8];  
    r[10+3] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if (((strcmp(stringchars((r[7])), stringchars((cx__231521))) == 0) && (strcmp(stringchars((r[2])), stringchars((r[3]))) == 0))) {
    { /* string-append */
    int *d = stringcat(stringdata((r[7])), stringdata((cx__231495)));
    r[8] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__231514)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[7])), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[4];  
    r[10+2] = r[8];  
    r[10+3] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    { /* string-append */
    int *d = stringcat(stringdata((r[7])), stringdata((cx__231495)));
    r[8] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__231498)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[7])), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__231503)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[7])), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[4];  
    r[10+2] = r[8];  
    r[10+3] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }

s_loop_v6848: /* k id cg-cexp renv */
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
    *--hp = obj_from_case(130);
    r[5] = (hendblk(4+1));
    r[0] = r[5];  
    r[1] = r[4];  
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v6848;
  }

case 130: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp renv id k */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(131);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (car((r[4])));
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 131: /* clo ek r */
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

case 132: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r prim renv k */
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[5] = obj_from_fixnum(n); }
    r[6+0] = (cx_c_2Dformat_2Dprimexp_2A);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = r[5];  
    r[6+3] = r[2];  
    r[6+4] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 133: /* clo ek  */
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
    *--hp = obj_from_case(134);
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
    r[7+3] = (cx__23975);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 134: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(135);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23971);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 135: /* clo ek  */
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

case 136: /* clo k tgt exp renv */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6];
    r[1+10] = p[7];
    r[1+11] = p[8];
    r[1+12] = p[9];
    r[1+13] = p[10];
    r[1+14] = p[11];
    r[1+15] = p[12];
    r[1+16] = p[13];
    r[1+17] = p[14]; }
    r += 1; /* shift reg. wnd */
    /* k tgt exp renv cg-cexp? gvar-id->c-name cvar-id->c-name make-gclabel global-constant-labels cg-fast-goto? lookup-label lookup-curry-case-no max-live cg-tgt-lval nl/regs-comment cg-gcsafe-cexp? alloc-reg cg-tgt-rval */
    hreserve(hbsz(14+1), 18); /* 18 live regs */
    *--hp = (r[17]);
    *--hp = (r[16]);
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
    *--hp = obj_from_case(151);
    r[18] = (hendblk(14+1));
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (4))) {
    r[19] = (vectorref((r[2]), (0)));
    r[19] = obj_from_bool((r[19]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[19] = obj_from_bool(0);
  }
  } else {
    r[19] = obj_from_bool(0);
  }
  if (bool_from_obj(r[19])) {
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
    r[10] = (r[10]);
    r[11] = (r[11]);
    r[12] = (r[12]);
    r[13] = (r[13]);
    r[14] = (r[14]);
    r[15] = (r[15]);
    r[16] = (r[16]);
    r[17] = (r[17]);
    goto s_cg_2Dprimapp_2Dto;
  } else {
    hreserve(hbsz(10+1), 19); /* 19 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (r[18]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(137);
    r[19] = (hendblk(10+1));
    r[0] = (r[19]);
    r[1] = r[2];  
    goto gs_curry_2Dexp_3F_23213;
  }

case 137: /* clo ek r */
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
    /* ek r make-gclabel global-constant-labels cg-fast-goto? lookup-label lookup-curry-case-no cg-primapp-to renv tgt k exp */
  if (bool_from_obj(r[1])) {
    r[12] = (vectorref((r[11]), (2)));
    r[12] = (vectorref((r[12]), (1)));
    r[12] = (vectorref((r[12]), (1)));
    { fixnum_t v7811_tmp;
    r[13] = (vectorref((r[11]), (2)));
    r[13] = (vectorref((r[13]), (2)));
    r[14] = (vectorref((r[11]), (1)));
    { /* length */
    int n; obj l = (r[14]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7811_tmp = (n); }
    { /* list-tail */
    obj l = (r[13]); int c = (v7811_tmp);
    while (c-- > 0) l = cdr(l);
    r[13] = (l); } }
    hreserve(hbsz(5+1), 14); /* 14 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = (r[13]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(138);
    r[14] = (hendblk(5+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[14]);
    r[2] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(9+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(140);
    r[12] = (hendblk(9+1));
    r[0] = (cx_labelapp_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 138: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-primapp-to renv rands tgt k */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(139);
    r[7] = (hendblk(5+1));
  if ((is_fixnum_obj(r[1]))) {
    r[8+0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[1];  
    r[8+3] = obj_from_fixnum(10);
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[8+0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 139: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-primapp-to renv rands tgt k */
    { /* string-append */
    int *d = stringcat(stringdata((cx__23780)), stringdata((cx__232197)));
    r[7] = (hpushstr(7, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232203)), stringdata((r[7])));
    r[8] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232206)), stringdata((r[8])));
    r[9] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232209)), stringdata((r[9])));
    r[10] = (hpushstr(10, d)); }
    { fixnum_t v7810_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7810_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v7810_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = r[4];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[11] = (o); } }
    { /* vector */
    hreserve(hbsz(4+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = (mksymbol(internsym("*-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[11] = (hendblk(4+1)); }
    r[12+0] = r[2];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = r[6];  
    r[12+2] = r[5];  
    r[12+3] = (r[11]);
    r[12+4] = r[3];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 140: /* clo ek r */
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
    /* ek r make-gclabel global-constant-labels cg-fast-goto? lookup-label cg-primapp-to renv tgt k exp */
  if (bool_from_obj(r[1])) {
    r[11] = (vectorref((r[10]), (1)));
    r[11] = (vectorref((r[11]), (1)));
    r[12] = (vectorref((r[10]), (2)));
    hreserve(hbsz(7+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[12]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = (r[11]);
    *--hp = r[5];  
    *--hp = obj_from_case(141);
    r[13] = (hendblk(7+1));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = r[7];  
    r[3] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
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
  if ((isvector((r[11])))) {
  if (((vectorlen((r[11]))) == (2))) {
    r[12] = (vectorref((r[11]), (0)));
    r[11] = obj_from_bool((r[12]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[10]), (1)));
    r[11] = (vectorref((r[11]), (1)));
    r[12] = (objptr_from_obj(r[3])[0]);
    r[11] = (cxs_rassq((r[11]), (r[12])));
  } else {
    r[11] = obj_from_bool(0);
  }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[11])) {
    r[11] = (vectorref((r[10]), (1)));
    r[11] = (vectorref((r[11]), (1)));
    r[12] = (vectorref((r[10]), (2)));
    hreserve(hbsz(7+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[12]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = (r[11]);
    *--hp = r[2];  
    *--hp = obj_from_case(144);
    r[13] = (hendblk(7+1));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = r[7];  
    r[3] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
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
    hreserve(hbsz(6+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[11]);
    *--hp = (r[12]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(147);
    r[13] = (hendblk(6+1));
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = r[7];  
    r[3] = (r[14]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[11] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[11]))));
    hreserve(hbsz(3+1), 12); /* 12 live regs */
    *--hp = r[9];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = obj_from_case(148);
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
    r[3] = (cx__23981);
    r[4] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (cx__23977);
    r[3] = obj_from_bool(1);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }

case 141: /* clo ek r */
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
    /* ek r lookup-label id cg-primapp-to renv rands tgt k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(142);
    r[9] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[3];  
    r[3] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(143);
    r[9] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[3];  
    r[3] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 142: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-primapp-to renv rands tgt k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__232110)));
    r[7] = (hpushstr(7, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232130)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232133)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* vector */
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("*?!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(4+1)); }
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[6];  
    r[9+2] = r[5];  
    r[9+3] = r[8];  
    r[9+4] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 143: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-primapp-to renv rands tgt k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__232110)));
    r[7] = (hpushstr(7, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232113)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232056)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232059)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232120)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* vector */
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("*?!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(4+1)); }
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[6];  
    r[9+2] = r[5];  
    r[9+3] = r[8];  
    r[9+4] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 144: /* clo ek r */
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
    /* ek r make-gclabel id cg-primapp-to renv rands tgt k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(145);
    r[9] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(146);
    r[9] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 145: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-primapp-to renv rands tgt k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__232110)));
    r[7] = (hpushstr(7, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232130)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232133)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* vector */
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("*?!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(4+1)); }
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[6];  
    r[9+2] = r[5];  
    r[9+3] = r[8];  
    r[9+4] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 146: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-primapp-to renv rands tgt k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__232110)));
    r[7] = (hpushstr(7, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232113)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232056)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232059)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232120)), stringdata((r[7])));
    r[7] = (hpushstr(8, d)); }
    { /* vector */
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("*?!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(4+1)); }
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[6];  
    r[9+2] = r[5];  
    r[9+3] = r[8];  
    r[9+4] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 147: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-primapp-to renv rands rator tgt k */
  if (bool_from_obj(r[1])) {
    { /* string-append */
    int *d = stringcat(stringdata((cx__232053)), stringdata((cx__232052)));
    r[8] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232080)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232083)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232086)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232089)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("*?!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[9] = (hendblk(4+1)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[7];  
    r[10+2] = r[6];  
    r[10+3] = r[9];  
    r[10+4] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    { /* string-append */
    int *d = stringcat(stringdata((cx__232053)), stringdata((cx__232052)));
    r[8] = (hpushstr(8, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232056)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232059)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232062)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232065)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232068)), stringdata((r[8])));
    r[8] = (hpushstr(9, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("*?!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[9] = (hendblk(4+1)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[7];  
    r[10+2] = r[6];  
    r[10+3] = r[9];  
    r[10+4] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }

case 148: /* clo ek  */
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
    *--hp = obj_from_case(149);
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
    r[7+3] = (cx__23975);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 149: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(150);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23971);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 150: /* clo ek  */
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

case 151: /* clo k tgt exp renv */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6];
    r[1+10] = p[7];
    r[1+11] = p[8];
    r[1+12] = p[9];
    r[1+13] = p[10];
    r[1+14] = p[11];
    r[1+15] = p[12];
    r[1+16] = p[13];
    r[1+17] = p[14]; }
    r += 1; /* shift reg. wnd */
s_cg_2Dprimapp_2Dto: /* k tgt exp renv cg-cexp? gvar-id->c-name cvar-id->c-name make-gclabel global-constant-labels cg-fast-goto? lookup-label lookup-curry-case-no max-live cg-tgt-lval nl/regs-comment cg-gcsafe-cexp? alloc-reg cg-tgt-rval */
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (4))) {
    r[18] = (vectorref((r[2]), (0)));
    r[18] = obj_from_bool((r[18]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[18] = obj_from_bool(0);
  }
  } else {
    r[18] = obj_from_bool(0);
  }
  if (bool_from_obj(r[18])) {
    r[18] = (vectorref((r[2]), (2)));
    r[19] = (vectorref((r[2]), (3)));
    { fixnum_t v7809_tmp;
    { fixnum_t v7808_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7809_tmp = (n); }
    { /* length */
    int n; obj l = (r[19]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7808_tmp = (n); }
    r[20] = obj_from_fixnum((v7809_tmp) + (v7808_tmp)); } }
    r[21] = (objptr_from_obj(r[12])[0]);
    r[20] = ((fixnum_from_obj(r[20]) > fixnum_from_obj(r[21])) ? (r[20]) : (r[21]));
    (void)(objptr_from_obj(r[12])[0] = (r[20]));
    hreserve(hbsz(4+1), 20); /* 20 live regs */
    *--hp = (r[14]);
    *--hp = (r[18]);
    *--hp = r[1];  
    *--hp = (r[13]);
    *--hp = obj_from_case(152);
    r[20] = (hendblk(4+1));
    r[21+0] = r[0];  
    r[21+1] = (r[19]);
    r[21+2] = r[3];  
    r[21+3] = (r[20]);
    r[21+4] = r[4];  
    r[21+5] = r[5];  
    r[21+6] = r[6];  
    r[21+7] = r[7];  
    r[21+8] = r[8];  
    r[21+9] = r[9];  
    r[21+10] = (r[10]);
    r[21+11] = (r[11]);
    r[21+12] = (r[12]);
    r[21+13] = (r[13]);
    r[21+14] = (r[14]);
    r[21+15] = (r[15]);
    r[21+16] = (r[16]);
    r[21+17] = (r[17]);
    r += 21; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_cg_2Dprim_2Dargs;
  } else {
    r[18] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[18]))));
    hreserve(hbsz(3+1), 19); /* 19 live regs */
    *--hp = r[0];  
    *--hp = (r[18]);
    *--hp = r[2];  
    *--hp = obj_from_case(159);
    r[19] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 20); /* 20 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[20] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[19]);
    r[2] = (r[18]);
    r[3] = (cx__23981);
    r[4] = (r[20]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[19]);
    r[2] = (cx__23977);
    r[3] = obj_from_bool(1);
    r[4] = (r[18]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }

case 152: /* clo k code args new-renv */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* k code args new-renv cg-tgt-lval tgt prim nl/regs-comment */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(153);
    r[8] = (hendblk(6+1));
    r[9+0] = r[4];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[5];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 153: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r args prim nl/regs-comment k code new-renv */
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[8] = obj_from_fixnum(n); }
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = obj_from_case(154);
    r[9] = (hendblk(4+1));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(157);
    r[9] = (hendblk(5+1));
    r[0] = (cx_prim_2Dcexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 154: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r nl/regs-comment new-renv k code */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(155);
    r[6] = (hendblk(4+1));
    r[7+0] = (cx_cleanup_2Dc_2Dcode_21);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 155: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r nl/regs-comment new-renv k code */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__233235);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(156);
    r[8] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 156: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k code r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 157: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r args prim ltext r k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(158);
    r[7] = (hendblk(2+1));
    r[8+0] = (cx_c_2Dformat_2Dprimexp_2A);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[4];  
    r[8+3] = r[3];  
    r[8+4] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[7+0] = (cx_c_2Dformat_2Dprim_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[4];  
    r[7+3] = r[5];  
    r[7+4] = r[3];  
    r[7+5] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 158: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[2])), stringdata((cx__231412)));
    r[5] = (hpushstr(5, d)); }
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[5];  
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 159: /* clo ek  */
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
    *--hp = obj_from_case(160);
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
    r[7+3] = (cx__23975);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 160: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(161);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23971);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 161: /* clo ek  */
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

case 162: /* clo k tgt exp renv */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6];
    r[1+10] = p[7];
    r[1+11] = p[8];
    r[1+12] = p[9];
    r[1+13] = p[10];
    r[1+14] = p[11];
    r[1+15] = p[12];
    r[1+16] = p[13];
    r[1+17] = p[14]; }
    r += 1; /* shift reg. wnd */
    /* k tgt exp renv cg-cexp? gvar-id->c-name cvar-id->c-name make-gclabel global-constant-labels cg-fast-goto? lookup-label lookup-curry-case-no max-live nl/regs-comment cg-gcsafe-cexp? alloc-reg cg-tgt-lval cg-tgt-rval */
    hreserve(hbsz(14+1), 18); /* 18 live regs */
    *--hp = (r[17]);
    *--hp = (r[16]);
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
    *--hp = obj_from_case(113);
    r[18] = (hendblk(14+1));
    hreserve(hbsz(1+1), 19); /* 19 live regs */
    *--hp = (r[17]);
    *--hp = obj_from_case(123);
    r[19] = (hendblk(1+1));
    hreserve(hbsz(14+1), 20); /* 20 live regs */
    *--hp = r[6];  
    *--hp = (r[15]);
    *--hp = (r[13]);
    *--hp = (r[17]);
    *--hp = (r[16]);
    *--hp = (r[14]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(190);
    r[20] = (hendblk(14+1));
    hreserve(hbsz(12+1), 21); /* 21 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = (r[13]);
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = (r[18]);
    *--hp = (r[11]);
    *--hp = (r[19]);
    *--hp = (r[15]);
    *--hp = r[4];  
    *--hp = (r[20]);
    *--hp = obj_from_case(163);
    r[18] = (hendblk(12+1));
    r[0] = (r[18]);
    r[1] = r[2];  
    goto gs_bgc_2Dexp_3F;

case 163: /* clo ek r */
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
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-let-list cg-cexp? alloc-reg cg-cexp lookup-curry-case-no cg-to tgt gvar-id->c-name nl/regs-comment renv k exp */
  if (bool_from_obj(r[1])) {
    r[14] = (vectorref((r[13]), (2)));
    r[14] = (car((r[14])));
    r[14] = (vectorref((r[14]), (1)));
    r[15] = (vectorref((r[13]), (2)));
    r[15] = (car((r[15])));
    r[15] = (vectorref((r[15]), (2)));
    r[16] = (vectorref((r[15]), (2)));
    r[16] = (vectorref((r[16]), (1)));
    r[15] = (vectorref((r[16]), (1)));
    r[16] = (vectorref((r[13]), (1)));
    r[16] = (vectorref((r[16]), (2)));
    hreserve(hbsz(8+1), 17); /* 17 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = (r[14]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[16]);
    *--hp = r[7];  
    *--hp = obj_from_case(164);
    r[17] = (hendblk(8+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[17]);
    r[2] = (r[15]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_23274((r[13])))) {
    r[14] = (vectorref((r[13]), (1)));
    r[15] = (vectorref((r[13]), (2)));
    hreserve(hbsz(6+1), 16); /* 16 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = (r[14]);
    *--hp = (r[15]);
    *--hp = r[7];  
    *--hp = obj_from_case(168);
    r[16] = (hendblk(6+1));
    { /* vector */
    hreserve(hbsz(4+1), 17); /* 17 live regs */
    *--hp = (mknull());
    *--hp = (cx__232563);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[17] = (hendblk(4+1)); }
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[16]);
    r[2] = r[8];  
    r[3] = (r[17]);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((isvector((r[13])))) {
  if (((vectorlen((r[13]))) == (4))) {
    r[14] = (vectorref((r[13]), (0)));
    r[14] = obj_from_bool((r[14]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[14] = obj_from_bool(0);
  }
  } else {
    r[14] = obj_from_bool(0);
  }
  if (bool_from_obj(r[14])) {
    r[14] = (vectorref((r[13]), (1)));
    r[15] = (vectorref((r[13]), (2)));
    r[16] = (vectorref((r[13]), (3)));
    hreserve(hbsz(10+1), 17); /* 17 live regs */
    *--hp = (r[12]);
    *--hp = (r[14]);
    *--hp = (r[11]);
    *--hp = r[5];  
    *--hp = (r[10]);
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[8];  
    *--hp = (r[15]);
    *--hp = obj_from_case(171);
    r[17] = (hendblk(10+1));
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[17]);
    r[2] = r[8];  
    r[3] = (r[16]);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_232390((r[13])))) {
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = r[8];  
    r[3] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_232367((r[13])));
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[2];  
    *--hp = obj_from_case(181);
    r[14] = (hendblk(7+1));
    r[0] = (r[14]);
    r[1] = (r[13]);
    goto gs_begin_2Dexp_3F_23301;
  }
  }
  }
  }

case 164: /* clo ek r */
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
    /* ek r cg-to exp2 tgt gvar-id->c-name id nl/regs-comment renv k */
    hreserve(hbsz(6+1), 10); /* 10 live regs */
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(165);
    r[10] = (hendblk(6+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[4];  
    /* r[3] */    
    r[4] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 165: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r gvar-id->c-name id nl/regs-comment renv k r */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__232584);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(166);
    r[10] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 166: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r nl/regs-comment renv k r r */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx__232589);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__232594);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__232597);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (cx__233235);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[4];  
    *--hp = obj_from_case(167);
    r[13] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 167: /* clo ek r */
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

case 168: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-to exp id nl/regs-comment renv k */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(169);
    r[9] = (hendblk(4+1));
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    /* r[3] */    
    r[4] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 169: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r nl/regs-comment renv k r */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_case(170);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 170: /* clo ek r */
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

case 171: /* clo ek r */
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
    /* ek r then-exp tgt cg-cexp? alloc-reg cg-to nl/regs-comment cg-cexp renv test-exp k */
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (mknull());
    *--hp = (cx__232501);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (cx__233234);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (cx__232508);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (cx__233234);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    hreserve(hbsz(9+1), 15); /* 15 live regs */
    *--hp = (r[14]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(172);
    r[15] = (hendblk(9+1));
    r[16+0] = r[6];  
    pc = objptr_from_obj(r[16+0])[0];
    r[16+1] = (r[15]);
    r[16+2] = r[3];  
    r[16+3] = r[2];  
    r[16+4] = r[9];  
    r += 16; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 172: /* clo ek r */
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
    /* ek r cg-cexp? alloc-reg cg-to nl/regs-comment cg-cexp renv test-exp k r */
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    hreserve(hbsz(2+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = obj_from_case(173);
    r[12] = (hendblk(2+1));
    hreserve(hbsz(7+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(174);
    r[12] = (hendblk(7+1));
    r[0] = (objptr_from_obj(r[2])[0]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 173: /* clo ek r */
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

case 174: /* clo ek r */
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
    /* ek r alloc-reg cg-to nl/regs-comment cg-cexp renv test-exp k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(1+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = obj_from_case(175);
    r[9] = (hendblk(1+1));
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[7];  
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_case(178);
    r[9] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = obj_from_fixnum(0);
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 175: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(176);
    r[4] = (hendblk(1+1));
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cx__232537);
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 176: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(177);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_cleanup_2Dc_2Dcode_21);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 177: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx__233234);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 178: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-to test-exp nl/regs-comment k renv */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(179);
    r[7] = (hendblk(3+1));
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[8] = obj_from_fixnum(n); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[8];  
    /* r[3] */    
    r[4] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 179: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r nl/regs-comment k renv */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(180);
    r[5] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 180: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r renv */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = (cx__232517);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[6] = obj_from_fixnum(n); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__232523);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__233234);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mknull());
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
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 181: /* clo ek r */
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
    /* ek r cg-let-list tgt cg-to nl/regs-comment renv k exp */
  if (bool_from_obj(r[1])) {
    r[9] = (vectorref((r[8]), (2)));
    r[9] = (car((r[9])));
    r[10] = (vectorref((r[8]), (1)));
    r[10] = (vectorref((r[10]), (2)));
    hreserve(hbsz(5+1), 11); /* 11 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = obj_from_case(182);
    r[11] = (hendblk(5+1));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[3];  
    r[3] = (r[10]);
    r[4] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23331((r[8])))) {
    r[9] = (vectorref((r[8]), (1)));
    r[9] = (vectorref((r[9]), (2)));
    r[10] = (vectorref((r[8]), (1)));
    r[10] = (vectorref((r[10]), (1)));
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = r[3];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = obj_from_case(185);
    r[11] = (hendblk(3+1));
    r[12+0] = r[2];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = r[7];  
    r[12+2] = (vectorref((r[8]), (2)));
    r[12+3] = (r[10]);
    r[12+4] = r[6];  
    r[12+5] = (r[11]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    r[9] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[9]))));
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = obj_from_case(187);
    r[10] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[9];  
    r[3] = (cx__23981);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__23977);
    r[3] = obj_from_bool(1);
    r[4] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }

case 182: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-to exp1 nl/regs-comment renv k */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(183);
    r[8] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = obj_from_bool(0);
    /* r[3] */    
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 183: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r nl/regs-comment renv k r */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_case(184);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 184: /* clo ek r */
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

case 185: /* clo k code new-renv */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* k code new-renv cg-to body tgt */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(186);
    r[6] = (hendblk(2+1));
    r[7+0] = r[3];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[5];  
    r[7+3] = r[4];  
    r[7+4] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 186: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k code */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 187: /* clo ek  */
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
    *--hp = obj_from_case(188);
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
    r[7+3] = (cx__23975);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 188: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(189);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23971);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 189: /* clo ek  */
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

case 190: /* clo k exps vars renv cont */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4];
    r[1+9] = p[5];
    r[1+10] = p[6];
    r[1+11] = p[7];
    r[1+12] = p[8];
    r[1+13] = p[9];
    r[1+14] = p[10];
    r[1+15] = p[11];
    r[1+16] = p[12];
    r[1+17] = p[13];
    r[1+18] = p[14]; }
    r += 1; /* shift reg. wnd */
    /* k exps vars renv cont cg-cexp? gvar-id->c-name make-gclabel global-constant-labels cg-fast-goto? lookup-label lookup-curry-case-no max-live cg-gcsafe-cexp? cg-tgt-lval cg-tgt-rval nl/regs-comment alloc-reg cvar-id->c-name */
    hreserve(hbsz(14+1), 19); /* 19 live regs */
    *--hp = (r[18]);
    *--hp = (r[17]);
    *--hp = (r[16]);
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
    *--hp = obj_from_case(190);
    r[19] = (hendblk(14+1));
    hreserve(hbsz(14+1), 20); /* 20 live regs */
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[17]);
    *--hp = (r[13]);
    *--hp = (r[16]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (r[18]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(113);
    r[20] = (hendblk(14+1));
  if ((isnull((r[1])))) {
    r[21+0] = r[4];  
    pc = objptr_from_obj(r[21+0])[0];
    r[21+1] = r[0];  
    r[21+2] = (cx__232953);
    r[21+3] = r[3];  
    r += 21; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[21] = (car((r[2])));
    r[21] = (cxs_cvar_2Did_3F_232940((r[21])));
  if (bool_from_obj(r[21])) {
    r[21] = (car((r[2])));
    hreserve(hbsz(9+1), 22); /* 22 live regs */
    *--hp = (r[21]);
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[19]);
    *--hp = (r[18]);
    *--hp = (r[20]);
    *--hp = obj_from_case(191);
    r[22] = (hendblk(9+1));
    r[23+0] = (r[18]);
    pc = objptr_from_obj(r[23+0])[0];
    r[23+1] = (r[22]);
    r[23+2] = (r[21]);
    r += 23; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[21] = obj_from_fixnum(n); }
    r[22] = (car((r[1])));
    hreserve(hbsz(8+1), 23); /* 23 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = (r[19]);
    *--hp = r[3];  
    *--hp = (r[17]);
    *--hp = (r[16]);
    *--hp = obj_from_case(198);
    r[23] = (hendblk(8+1));
    r[24+0] = (r[23]);
    r[24+1] = (r[21]);
    r[24+2] = (r[22]);
    r[24+3] = r[3];  
    r[24+4] = r[5];  
    r[24+5] = r[6];  
    r[24+6] = (r[18]);
    r[24+7] = r[7];  
    r[24+8] = r[8];  
    r[24+9] = r[9];  
    r[24+10] = (r[10]);
    r[24+11] = (r[11]);
    r[24+12] = (r[12]);
    r[24+13] = (r[16]);
    r[24+14] = (r[13]);
    r[24+15] = (r[17]);
    r[24+16] = (r[14]);
    r[24+17] = (r[15]);
    r += 24; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_cg_2Dto;
  }
  }

case 191: /* clo ek r */
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
    /* ek r cg-to cvar-id->c-name cg-let-list cont renv vars exps k id */
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(192);
    r[11] = (hendblk(9+1));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = r[6];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (r[12]);
    r[3] = (car((r[8])));
    r[4] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 192: /* clo ek r */
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
    /* ek r r cvar-id->c-name cg-let-list cont renv vars exps k id */
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = r[1];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(193);
    r[11] = (hendblk(9+1));
  if ((ispair((r[1])))) {
    r[12] = (car((r[1])));
    r[12] = obj_from_bool(isequal((r[12]), (cx__233235)));
  if (bool_from_obj(r[12])) {
    r[12] = (cdr((r[1])));
    r[12] = obj_from_bool(ispair((r[12])));
  if (bool_from_obj(r[12])) {
    r[12] = (cdr((r[1])));
    r[12] = (car((r[12])));
    r[12] = obj_from_bool(isstring((r[12])));
  if (bool_from_obj(r[12])) {
    r[12] = (cdr((r[1])));
    r[12] = (car((r[12])));
    { /* string-append */
    int *d = stringcat(stringdata((r[2])), stringdata((cx__232922)));
    r[13] = (hpushstr(13, d)); }
    r[14+0] = (cx_string_2Dstarts_2Dwith_3F);
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[11]);
    r[14+2] = (r[12]);
    r[14+3] = (r[13]);
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = obj_from_bool(0);
    r[12+2] = r[3];  
    r[12+3] = r[4];  
    r[12+4] = r[5];  
    r[12+5] = r[6];  
    r[12+6] = r[7];  
    r[12+7] = r[8];  
    r[12+8] = r[9];  
    r[12+9] = (r[10]);
    r[12+10] = r[1];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6345;
  }
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = obj_from_bool(0);
    r[12+2] = r[3];  
    r[12+3] = r[4];  
    r[12+4] = r[5];  
    r[12+5] = r[6];  
    r[12+6] = r[7];  
    r[12+7] = r[8];  
    r[12+8] = r[9];  
    r[12+9] = (r[10]);
    r[12+10] = r[1];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6345;
  }
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = obj_from_bool(0);
    r[12+2] = r[3];  
    r[12+3] = r[4];  
    r[12+4] = r[5];  
    r[12+5] = r[6];  
    r[12+6] = r[7];  
    r[12+7] = r[8];  
    r[12+8] = r[9];  
    r[12+9] = (r[10]);
    r[12+10] = r[1];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6345;
  }
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = obj_from_bool(0);
    r[12+2] = r[3];  
    r[12+3] = r[4];  
    r[12+4] = r[5];  
    r[12+5] = r[6];  
    r[12+6] = r[7];  
    r[12+7] = r[8];  
    r[12+8] = r[9];  
    r[12+9] = (r[10]);
    r[12+10] = r[1];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6345;
  }

case 193: /* clo ek r */
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
s_l_v6345: /* ek r cvar-id->c-name cg-let-list cont renv vars exps k id r */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = obj_from_case(194);
    r[11] = (hendblk(3+1));
    r[12+0] = r[3];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = (cdr((r[7])));
    r[12+3] = (cdr((r[6])));
    r[12+4] = r[5];  
    r[12+5] = r[4];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[2];  
    *--hp = obj_from_case(195);
    r[11] = (hendblk(3+1));
    hreserve(hbsz(2+1), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = r[4];  
    *--hp = obj_from_case(197);
    r[12] = (hendblk(2+1));
    r[13+0] = r[3];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = (r[11]);
    r[13+2] = (cdr((r[7])));
    r[13+3] = (cdr((r[6])));
    r[13+4] = r[5];  
    r[13+5] = (r[12]);
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 194: /* clo ek r */
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
    *--hp = (cx__232896);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6] = (cdr((r[4])));
    r[6] = (car((r[6])));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__232882);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8] = (cdr((r[3])));
    r[8] = (cdr((r[8])));
    r[8] = (cdr((r[8])));
    r[8] = (car((r[8])));
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__232917);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__233235);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 195: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name k id */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = (cx__232865);
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
    *--hp = (cx__232110);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(196);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[4];  
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
    /* ek r k id r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__232882);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7] = (cdr((r[3])));
    r[7] = (cdr((r[7])));
    r[7] = (cdr((r[7])));
    r[7] = (car((r[7])));
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx__232597);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__233235);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 197: /* clo k code renv */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k code renv cont r */
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
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[0];  
    r[6+2] = r[5];  
    r[6+3] = r[2];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 198: /* clo ek r */
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
    /* ek r nl/regs-comment alloc-reg renv cg-let-list cont vars exps k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(199);
    r[10] = (hendblk(8+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 199: /* clo ek r */
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
    /* ek r alloc-reg renv cg-let-list cont r vars exps k */
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(200);
    r[10] = (hendblk(7+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (car((r[7])));
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 200: /* clo ek r */
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
    /* ek r cg-let-list cont r r vars exps k */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(201);
    r[9] = (hendblk(3+1));
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = (cdr((r[7])));
    r[10+3] = (cdr((r[6])));
    r[10+4] = r[1];  
    r[10+5] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 201: /* clo k code renv */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* k code renv cont r r */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
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
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = r[3];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[0];  
    r[7+2] = r[6];  
    r[7+3] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 202: /* clo k exps renv cont */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6];
    r[1+10] = p[7];
    r[1+11] = p[8];
    r[1+12] = p[9];
    r[1+13] = p[10];
    r[1+14] = p[11];
    r[1+15] = p[12];
    r[1+16] = p[13];
    r[1+17] = p[14]; }
    r += 1; /* shift reg. wnd */
s_cg_2Dprim_2Dargs: /* k exps renv cont cg-cexp? gvar-id->c-name cvar-id->c-name make-gclabel global-constant-labels cg-fast-goto? lookup-label lookup-curry-case-no max-live cg-tgt-lval nl/regs-comment cg-gcsafe-cexp? alloc-reg cg-tgt-rval */
  if ((isnull((r[1])))) {
    r[18+0] = r[3];  
    pc = objptr_from_obj(r[18+0])[0];
    r[18+1] = r[0];  
    r[18+2] = (cx__232953);
    r[18+3] = (mknull());
    r[18+4] = r[2];  
    r += 18; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(14+1), 18); /* 18 live regs */
    *--hp = (r[17]);
    *--hp = (r[16]);
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
    *--hp = obj_from_case(202);
    r[18] = (hendblk(14+1));
    hreserve(hbsz(1+1), 19); /* 19 live regs */
    *--hp = (r[17]);
    *--hp = obj_from_case(123);
    r[19] = (hendblk(1+1));
    hreserve(hbsz(14+1), 20); /* 20 live regs */
    *--hp = (r[17]);
    *--hp = (r[13]);
    *--hp = (r[16]);
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(113);
    r[20] = (hendblk(14+1));
    hreserve(hbsz(10+1), 21); /* 21 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = (r[18]);
    *--hp = (r[19]);
    *--hp = (r[20]);
    *--hp = (r[17]);
    *--hp = (r[16]);
    *--hp = (r[15]);
    *--hp = obj_from_case(203);
    r[18] = (hendblk(10+1));
    r[19+0] = (r[14]);
    pc = objptr_from_obj(r[19+0])[0];
    r[19+1] = (r[18]);
    r[19+2] = r[2];  
    r += 19; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 203: /* clo ek r */
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
    /* ek r cg-gcsafe-cexp? alloc-reg cg-tgt-rval cg-to cg-cexp cg-prim-args cont renv exps k */
    hreserve(hbsz(10+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(204);
    r[12] = (hendblk(10+1));
    r[0] = (objptr_from_obj(r[2])[0]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (car((r[10])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 204: /* clo ek r */
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
    /* ek r alloc-reg cg-tgt-rval cg-to cg-cexp cg-prim-args cont r renv exps k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(6+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(205);
    r[12] = (hendblk(6+1));
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (car((r[10])));
    r[3] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 12); /* 12 live regs */
    *--hp = r[9];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(207);
    r[12] = (hendblk(8+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = obj_from_fixnum(0);
    r[3] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 205: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-prim-args cont r renv exps k */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(206);
    r[8] = (hendblk(3+1));
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[7];  
    r[9+2] = (cdr((r[6])));
    r[9+3] = r[5];  
    r[9+4] = r[8];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 206: /* clo k code atexts renv */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* k code atexts renv cont r r */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[9+0] = r[4];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[0];  
    r[9+2] = r[7];  
    r[9+3] = r[8];  
    r[9+4] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 207: /* clo ek r */
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
    /* ek r cg-tgt-rval cg-to cg-prim-args cont r exps k renv */
    { /* length */
    int n; obj l = r[9];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[10] = obj_from_fixnum(n); }
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[3];  
    *--hp = obj_from_case(208);
    r[11] = (hendblk(9+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 208: /* clo ek r */
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
    /* ek r cg-to renv tgt cg-prim-args cont r r exps k */
    hreserve(hbsz(7+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(209);
    r[11] = (hendblk(7+1));
    r[12+0] = r[2];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = r[4];  
    r[12+3] = (car((r[9])));
    r[12+4] = r[3];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 209: /* clo ek r */
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
    /* ek r cg-prim-args cont r r r exps k */
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(210);
    r[9] = (hendblk(4+1));
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = (cdr((r[7])));
    r[10+3] = r[6];  
    r[10+4] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 210: /* clo k code atexts renv */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* k code atexts renv cont r r r */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
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
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[4];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[0];  
    r[10+2] = r[8];  
    r[10+3] = r[9];  
    r[10+4] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 211: /* clo ek r */
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
    r[1+17] = p[16]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-gclabel code-gen-to-nextreg k lookup-label add-curry! goto-alist curry-alist used-global-labels global-constant-labels global-labels static-roots used-labels label-alist global-vars max-live input-fix-exp */
    hreserve(hbsz(16+1), 18); /* 18 live regs */
    *--hp = (r[16]);
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (r[17]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(215);
    r[18] = (hendblk(16+1));
    hreserve(hbsz(1+1), 19); /* 19 live regs */
    *--hp = (r[17]);
    *--hp = obj_from_case(212);
    r[19] = (hendblk(1+1));
    hreserve(hbsz(1+1), 20); /* 20 live regs */
    *--hp = (r[19]);
    *--hp = obj_from_case(297);
    r[19] = (hendblk(1+1));
    r[20+0] = (r[18]);
    r[20+1] = (r[19]);
    r[20+2] = r[1];  
    r += 20; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_sort_2Dby;

case 212: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id input-fix-exp */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(213);
    r[3] = (hendblk(2+1));
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(214);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_var_2Dassigned_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 213: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (cdr((r[3])));
    r[4] = (car((r[4])));
    r[4] = (hpushstr(5, newstring(symbolname(getsymbol((r[4]))))));
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((r[4])));
    r[4] = (hpushstr(5, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((bool_from_obj(cxs_global_2Did_2Dprivate_2Dconstant_3F_23618((r[3]))) ? (cx__233188) : (cx__233187))), stringdata((r[4])));
    r[5] = (hpushstr(5, d)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 214: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__233188);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__233187);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 215: /* clo ek r */
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
    r[1+17] = p[16]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-gclabel code-gen-to-nextreg k lookup-label add-curry! input-fix-exp goto-alist curry-alist used-global-labels global-constant-labels global-labels static-roots used-labels label-alist global-vars max-live */
    (void)(objptr_from_obj(r[17])[0] = obj_from_fixnum(0));
    (void)(objptr_from_obj(r[16])[0] = (r[1]));
    (void)(objptr_from_obj(r[15])[0] = (mknull()));
    (void)(objptr_from_obj(r[14])[0] = (mknull()));
    (void)(objptr_from_obj(r[13])[0] = (mknull()));
    (void)(objptr_from_obj(r[12])[0] = (mknull()));
    (void)(objptr_from_obj(r[11])[0] = (mknull()));
    (void)(objptr_from_obj(r[10])[0] = (mknull()));
    (void)(objptr_from_obj(r[9])[0] = (mknull()));
    (void)(objptr_from_obj(r[8])[0] = (mknull()));
  if ((isvector((r[7])))) {
  if (((vectorlen((r[7]))) == (4))) {
    r[18] = (vectorref((r[7]), (0)));
    r[18] = obj_from_bool((r[18]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[18] = obj_from_bool(0);
  }
  } else {
    r[18] = obj_from_bool(0);
  }
  if (bool_from_obj(r[18])) {
    r[18] = (vectorref((r[7]), (1)));
    r[19] = (vectorref((r[7]), (2)));
    r[20] = (vectorref((r[7]), (3)));
    hreserve(hbsz(19+1), 21); /* 21 live regs */
    *--hp = (r[15]);
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = (r[17]);
    *--hp = r[4];  
    *--hp = (r[16]);
    *--hp = r[7];  
    *--hp = (r[18]);
    *--hp = (r[19]);
    *--hp = r[3];  
    *--hp = (r[14]);
    *--hp = r[2];  
    *--hp = (r[20]);
    *--hp = obj_from_case(222);
    r[21] = (hendblk(19+1));
    r[0] = (r[21]);
    r[1] = (r[18]);
    r[2] = (r[15]);
    goto s_loop_v6150;
  } else {
    r[18] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[18]))));
    hreserve(hbsz(3+1), 19); /* 19 live regs */
    *--hp = r[4];  
    *--hp = (r[18]);
    *--hp = r[7];  
    *--hp = obj_from_case(294);
    r[19] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 20); /* 20 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[20] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[19]);
    r[2] = (r[18]);
    r[3] = (cx__23981);
    r[4] = (r[20]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[19]);
    r[2] = (cx__23977);
    r[3] = obj_from_bool(1);
    r[4] = (r[18]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }

case 216: /* clo k ids */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v6150: /* k ids label-alist */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[1])));
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(216);
    r[4] = (hendblk(1+1));
    hreserve(hbsz(5+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(217);
    r[4] = (hendblk(5+1));
    r[5] = (cdr((r[3])));
    r[5] = (car((r[5])));
    r[6+0] = (cx_c_2Dundecorate_2Dalvar);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 217: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r id loop k label-alist ids */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(218);
    r[7] = (hendblk(5+1));
    r[8+0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[1];  
    r[8+3] = (cx__231322);
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 218: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r id loop k label-alist ids */
    r[7] = (objptr_from_obj(r[5])[0]);
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(219);
    r[8] = (hendblk(4+1));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[2];  
    *--hp = obj_from_case(220);
    r[8] = (hendblk(3+1));
    r[9+0] = r[8];  
    r[9+1] = r[7];  
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v6167;

case 219: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop k label-alist ids */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = (car((r[5])));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (objptr_from_obj(r[4])[0]);
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    (void)(objptr_from_obj(r[4])[0] = (r[7]));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (cdr((r[5])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v6167: /* k id r */
  if ((!(isnull((r[1]))))) {
    r[3] = (car((r[1])));
    r[4] = (cdr((r[3])));
    r[3] = obj_from_bool(strcmp(stringchars((r[2])), stringchars((r[4]))) == 0);
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v6167;
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

case 220: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(221);
    r[5] = (hendblk(2+1));
    r[6] = (cdr((r[2])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[6] = obj_from_bool(is_fixnum_obj(r[6]));
  if (bool_from_obj(r[6])) {
    r[6] = (cdr((r[2])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[6] = (cdr((r[2])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 221: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* string-append */
    int *d = stringcat(stringdata((cx__231342)), stringdata((r[1])));
    r[4] = (hpushstr(4, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[3])), stringdata((r[4])));
    r[5] = (hpushstr(5, d)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 222: /* clo ek  */
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
    r[1+19] = p[18];
    r[1+20] = p[19]; }
    r += 1; /* shift reg. wnd */
    /* ek  body make-gclabel used-labels code-gen-to-nextreg lams ids input-fix-exp global-vars k max-live curry-alist used-global-labels lookup-label goto-alist add-curry! global-constant-labels global-labels static-roots label-alist */
    r[21] = (objptr_from_obj(r[20])[0]);
    { fixnum_t v7807_tmp;
    { /* length */
    int n; obj l = (r[21]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7807_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v7807_tmp);
    hreserve(hbsz(3)*c, 22); /* 22 live regs */
    l = (r[21]); /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[21] = (o); } }
    (void)(objptr_from_obj(r[20])[0] = (r[21]));
    hreserve(hbsz(7+1), 21); /* 21 live regs */
    *--hp = (r[19]);
    *--hp = (r[18]);
    *--hp = (r[17]);
    *--hp = (r[16]);
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = obj_from_case(269);
    r[21] = (hendblk(7+1));
    hreserve(hbsz(17+1), 22); /* 22 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[19]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = (r[17]);
    *--hp = (r[18]);
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = (r[21]);
    *--hp = obj_from_case(225);
    r[22] = (hendblk(17+1));
    r[0] = (r[22]);
    r[1] = r[6];  
    r[2] = (r[21]);
    goto s_loop_v6133;

case 223: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v6133: /* k id cc */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[1])));
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(223);
    r[4] = (hendblk(1+1));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(224);
    r[4] = (hendblk(3+1));
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (vectorref((r[3]), (2)));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 224: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 225: /* clo ek  */
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
    r[1+18] = p[17]; }
    r += 1; /* shift reg. wnd */
    /* ek  cc body make-gclabel used-global-labels lookup-label used-labels code-gen-to-nextreg global-labels global-constant-labels lams ids static-roots input-fix-exp global-vars k max-live curry-alist */
    hreserve(hbsz(16+1), 19); /* 19 live regs */
    *--hp = (r[18]);
    *--hp = (r[17]);
    *--hp = (r[16]);
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
    *--hp = obj_from_case(226);
    r[19] = (hendblk(16+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[19]);
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 226: /* clo ek  */
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
    r[1+17] = p[16]; }
    r += 1; /* shift reg. wnd */
    /* ek  body make-gclabel used-global-labels lookup-label used-labels code-gen-to-nextreg global-labels global-constant-labels lams ids static-roots input-fix-exp global-vars k max-live curry-alist */
    hreserve(hbsz(16+1), 18); /* 18 live regs */
    *--hp = (r[17]);
    *--hp = (r[16]);
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
    *--hp = r[2];  
    *--hp = obj_from_case(227);
    r[18] = (hendblk(16+1));
    hreserve(hbsz(1+1), 19); /* 19 live regs */
    *--hp = (r[11]);
    *--hp = obj_from_case(266);
    r[19] = (hendblk(1+1));
    r[0] = (r[18]);
    r[1] = (r[19]);
    r[2] = (objptr_from_obj(r[17])[0]);
    goto gs_sort_2Dby_21;

case 227: /* clo ek r */
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
    r[1+17] = p[16]; }
    r += 1; /* shift reg. wnd */
    /* ek r body make-gclabel used-global-labels lookup-label used-labels code-gen-to-nextreg global-labels global-constant-labels lams ids static-roots input-fix-exp global-vars k max-live curry-alist */
    (void)(objptr_from_obj(r[17])[0] = (r[1]));
    hreserve(hbsz(15+1), 18); /* 18 live regs */
    *--hp = (r[17]);
    *--hp = (r[16]);
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
    *--hp = obj_from_case(228);
    r[18] = (hendblk(15+1));
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[18]);
    /* r[2] */    
    r[3] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 228: /* clo ek r */
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
    r[1+16] = p[15]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-gclabel used-global-labels lookup-label used-labels code-gen-to-nextreg global-labels global-constant-labels lams ids static-roots input-fix-exp global-vars k max-live curry-alist */
    { /* cons */ 
    hreserve(hbsz(3), 17); /* 17 live regs */
    *--hp = (mknull());
    *--hp = (cx__23713);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[17] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 18); /* 18 live regs */
    *--hp = (r[17]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[17] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 18); /* 18 live regs */
    *--hp = (r[17]);
    *--hp = (cx__23929);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[18] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 19); /* 19 live regs */
    *--hp = (r[18]);
    *--hp = obj_from_fixnum(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[19] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 20); /* 20 live regs */
    *--hp = (r[19]);
    *--hp = (cx__23839);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[20] = (hendblk(3)); }
    hreserve(hbsz(6+1), 21); /* 21 live regs */
    *--hp = (r[16]);
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = obj_from_case(247);
    r[21] = (hendblk(6+1));
    r[22+0] = (r[21]);
    r[22+1] = (r[10]);
    r[22+2] = r[9];  
    r[22+3] = (mknull());
    r[22+4] = r[2];  
    r[22+5] = r[3];  
    r[22+6] = r[4];  
    r[22+7] = r[5];  
    r[22+8] = r[6];  
    r[22+9] = r[7];  
    r[22+10] = (r[16]);
    r[22+11] = r[8];  
    r[22+12] = (r[20]);
    r += 22; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5982;

case 229: /* clo k ids lams code */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6];
    r[1+10] = p[7];
    r[1+11] = p[8];
    r[1+12] = p[9]; }
    r += 1; /* shift reg. wnd */
s_loop_v5982: /* k ids lams code make-gclabel used-global-labels lookup-label used-labels code-gen-to-nextreg global-labels curry-alist global-constant-labels r */
  if ((isnull((r[1])))) {
    { fixnum_t v7806_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7806_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v7806_tmp);
    hreserve(hbsz(3)*c, 13); /* 13 live regs */
    l = r[3];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[13] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (mknull());
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[13] = (car((r[2])));
    r[14] = (car((r[1])));
    r[15] = (objptr_from_obj(r[11])[0]);
    { /* assq */
    obj x = (r[14]), l = (r[15]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[15] = (l == mknull() ? obj_from_bool(0) : p); }
    hreserve(hbsz(9+1), 16); /* 16 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(229);
    r[16] = (hendblk(9+1));
    hreserve(hbsz(12+1), 17); /* 17 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = (r[16]);
    *--hp = (r[10]);
    *--hp = (r[14]);
    *--hp = r[9];  
    *--hp = (r[13]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(230);
    r[16] = (hendblk(12+1));
  if (bool_from_obj(r[15])) {
    r[17] = (cdr((r[15])));
    r[18] = (objptr_from_obj(r[5])[0]);
    { /* memq */
    obj x = (r[17]), l = (r[18]);
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[17] = (l == mknull() ? obj_from_bool(0) : l); }
  } else {
    r[17] = obj_from_bool(0);
  }
  if (bool_from_obj(r[17])) {
    hreserve(hbsz(1+1), 17); /* 17 live regs */
    *--hp = (r[16]);
    *--hp = obj_from_case(246);
    r[17] = (hendblk(1+1));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[17]);
    r[2] = (cdr((r[15])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(9+1), 17); /* 17 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(229);
    r[17] = (hendblk(9+1));
    r[18+0] = obj_from_ktrap();
    r[18+1] = (mknull());
    r[18+2] = r[6];  
    r[18+3] = r[7];  
    r[18+4] = r[8];  
    r[18+5] = (r[13]);
    r[18+6] = r[9];  
    r[18+7] = (r[14]);
    r[18+8] = (r[10]);
    r[18+9] = (r[17]);
    r[18+10] = r[3];  
    r[18+11] = r[2];  
    r[18+12] = r[1];  
    r[18+13] = r[0];  
    r += 18; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5990;
  }
  }

case 230: /* clo ek r */
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
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
s_l_v5990: /* ek r lookup-label used-labels code-gen-to-nextreg lam global-labels id curry-alist loop code lams ids k */
    hreserve(hbsz(10+1), 14); /* 14 live regs */
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
    *--hp = obj_from_case(231);
    r[14] = (hendblk(10+1));
    r[15] = (objptr_from_obj(r[3])[0]);
    { /* memq */
    obj x = (r[7]), l = (r[15]);
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[15] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[15])) {
    hreserve(hbsz(2+1), 15); /* 15 live regs */
    *--hp = r[1];  
    *--hp = (r[14]);
    *--hp = obj_from_case(245);
    r[15] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[15]);
    r[2] = r[7];  
    r[3] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    r[2] = r[4];  
    r[3] = r[5];  
    r[4] = r[6];  
    r[5] = r[7];  
    r[6] = r[8];  
    r[7] = r[9];  
    r[8] = (r[10]);
    r[9] = (r[11]);
    r[10] = (r[12]);
    r[11] = (r[13]);
    goto s_l_v5998;
  }

case 231: /* clo ek r */
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
s_l_v5998: /* ek r code-gen-to-nextreg lam global-labels id curry-alist loop code lams ids k */
    hreserve(hbsz(10+1), 12); /* 12 live regs */
    *--hp = r[1];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(232);
    r[12] = (hendblk(10+1));
    r[13] = (vectorref((r[3]), (1)));
    { fixnum_t v7805_tmp;
    { /* length */
    int n; obj l = (r[13]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7805_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v7805_tmp);
    hreserve(hbsz(3)*c, 14); /* 14 live regs */
    l = (r[13]); /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[13] = (o); } }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (vectorref((r[3]), (2)));
    r[3] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 232: /* clo ek r */
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
    /* ek r lam global-labels id curry-alist loop code lams ids k r */
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (mknull());
    *--hp = (cx__23713);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (cx__23718);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14] = (vectorref((r[2]), (1)));
    hreserve(hbsz(10+1), 15); /* 15 live regs */
    *--hp = (r[13]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(235);
    r[15] = (hendblk(10+1));
    r[0] = (r[15]);
    r[1] = (r[14]);
    goto s_loop_v6080;

s_loop_v6080: /* k id */
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
    *--hp = obj_from_case(233);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v6080;
  }

case 233: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(234);
    r[5] = (hendblk(2+1));
    r[6] = (cdr((r[4])));
    r[6] = (car((r[6])));
    r[0] = (cx_c_2Dundecorate_2Dalvar);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 234: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__23729);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 235: /* clo ek r */
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
    /* ek r global-labels id curry-alist loop code lams ids k r r */
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (cx__23746);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = ((isnull((r[10]))) ? (cx__23749) : (r[10]));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    r[15] = (objptr_from_obj(r[4])[0]);
    hreserve(hbsz(6+1), 16); /* 16 live regs */
    *--hp = (r[14]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(244);
    r[16] = (hendblk(6+1));
    r[17+0] = (r[16]);
    r[17+1] = (r[15]);
    r[17+2] = obj_from_fixnum(1);
    r[17+3] = r[2];  
    r[17+4] = r[3];  
    r += 17; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v6009;

s_loop_v6009: /* k alst i global-labels id */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (car((r[1])));
    r[5] = (car((r[5])));
    r[5] = obj_from_bool((r[5]) == (r[4]));
  if (bool_from_obj(r[5])) {
    r[5] = (car((r[1])));
    r[6] = (cdr((r[5])));
    r[6] = (car((r[6])));
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    hreserve(hbsz(1), 8); /* 8 live regs */
    *--hp = obj_from_void(0);
    r[8] = (hendblk(1));
    hreserve(hbsz(1), 9); /* 9 live regs */
    *--hp = obj_from_void(0);
    r[9] = (hendblk(1));
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[10] = obj_from_fixnum(n); }
    { fixnum_t v7804_tmp;
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7804_tmp = (n); }
    r[11] = obj_from_fixnum((v7804_tmp) + (1)); }
    (void)(objptr_from_obj(r[9])[0] = (r[11]));
    (void)(objptr_from_obj(r[8])[0] = (r[10]));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(236);
    r[10] = (hendblk(5+1));
    r[11] = (objptr_from_obj(r[8])[0]);
    r[11] = obj_from_bool(fixnum_from_obj(r[11]) > (0));
  if (bool_from_obj(r[11])) {
    hreserve(hbsz(2+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[8];  
    *--hp = obj_from_case(240);
    r[11] = (hendblk(2+1));
    r[12] = (objptr_from_obj(r[9])[0]);
    r[13] = (objptr_from_obj(r[8])[0]);
    r[12] = obj_from_fixnum(fixnum_from_obj(r[12]) + fixnum_from_obj(r[13]));
    r[0] = (r[11]);
    r[1] = (objptr_from_obj(r[9])[0]);
    r[2] = (r[12]);
    goto gs_iota_23869;
  } else {
    r[11+0] = obj_from_ktrap();
    r[11+1] = (mknull());
    r[11+2] = r[0];  
    r[11+3] = r[2];  
    r[11+4] = r[4];  
    r[11+5] = r[3];  
    r[11+6] = r[5];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6042;
  }
  } else {
    r[5] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[5];  
    r[2] = obj_from_fixnum(fixnum_from_obj(r[2]) + (1));
    /* r[3] */    
    /* r[4] */    
    goto s_loop_v6009;
  }
  }

case 236: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
s_l_v6042: /* ek r k i id global-labels id&ids&rands */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = (cx__23769);
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
    *--hp = (cx__23780);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { fixnum_t v7803_tmp;
    r[9] = (cdr((r[6])));
    r[9] = (car((r[9])));
    { /* length */
    int n; obj l = r[9];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7803_tmp = (n); }
    r[9] = obj_from_fixnum((1) + (v7803_tmp)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__23789);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__23792);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[12] = (cdr((r[6])));
    r[12] = (car((r[12])));
    hreserve(hbsz(5+1), 13); /* 13 live regs */
    *--hp = (r[11]);
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(239);
    r[13] = (hendblk(5+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    goto s_loop_v6049;

s_loop_v6049: /* k id */
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
    *--hp = obj_from_case(237);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v6049;
  }

case 237: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(238);
    r[5] = (hendblk(2+1));
    r[6] = (cdr((r[4])));
    r[6] = (car((r[6])));
    r[0] = (cx_c_2Dundecorate_2Dalvar);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 238: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__23729);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 239: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r k i id global-labels r */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx__23729);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[9] = (objptr_from_obj(r[5])[0]);
    { /* assq */
    obj x = (r[4]), l = (r[9]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[9] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[9])) {
    r[10] = (cdr((r[9])));
    r[11] = (cdr((r[10])));
    r[9] = (car((r[11])));
  } else {
    r[9] = (cx__23823);
  }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__23831);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = (cx__23834);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (cx__23839);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 240: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r ncopy k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(241);
    r[4] = (hendblk(2+1));
    r[5] = (objptr_from_obj(r[2])[0]);
    r[5] = obj_from_fixnum((1) + fixnum_from_obj(r[5]));
    r[0] = r[4];  
    r[1] = obj_from_fixnum(1);
    r[2] = r[5];  
    goto gs_iota_23869;

case 241: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(243);
    r[4] = (hendblk(1+1));
    r[5+0] = r[4];  
    r[5+1] = r[2];  
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v6020;

s_loop_v6020: /* k id id */
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
    *--hp = obj_from_case(242);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v6020;
  }

case 242: /* clo ek r */
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
    *--hp = (mknull());
    *--hp = (cx__23851);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx__23856);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = obj_from_fixnum(fixnum_from_obj(r[6]) - (1));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx__23861);
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

case 243: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__23866);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx__23895);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 244: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop code lams ids k r */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[3];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[6];  
    r[10+2] = (cdr((r[5])));
    r[10+3] = (cdr((r[4])));
    r[10+4] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 245: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__23691);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = ((isnull((r[3]))) ? (mknull()) : (cx__23640));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 246: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__23691);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
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

case 247: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r static-roots input-fix-exp global-vars k max-live curry-alist */
    r[8] = (objptr_from_obj(r[4])[0]);
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(254);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = r[8];  
    r[2] = r[3];  
    goto s_loop_v5937;

s_loop_v5937: /* k id input-fix-exp */
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
    *--hp = obj_from_case(248);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v5937;
  }

case 248: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r input-fix-exp k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(1), 6); /* 6 live regs */
    *--hp = obj_from_void(0);
    r[6] = (hendblk(1));
    r[7] = (cdr((r[5])));
    r[7] = (car((r[7])));
    r[7] = (hpushstr(8, newstring(symbolname(getsymbol((r[7]))))));
    (void)(objptr_from_obj(r[6])[0] = (r[7]));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(249);
    r[7] = (hendblk(2+1));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(250);
    r[7] = (hendblk(3+1));
    r[8+0] = (cx_var_2Dassigned_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[5];  
    r[8+3] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 249: /* clo ek r */
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

case 250: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r name k id */
  if (bool_from_obj(r[1])) {
  if (bool_from_obj(cxs_global_2Did_2Dprivate_2Dconstant_3F_23618((r[4])))) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(251);
    r[5] = (hendblk(2+1));
    r[0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (objptr_from_obj(r[2])[0]);
    r[3] = (cx__23525);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(252);
    r[5] = (hendblk(2+1));
    r[0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (objptr_from_obj(r[2])[0]);
    r[3] = (cx__23525);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  } else {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(253);
    r[5] = (hendblk(2+1));
    r[0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (objptr_from_obj(r[2])[0]);
    r[3] = (cx__23525);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 251: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (cdr((r[3])));
    r[4] = (car((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = (cx__23617);
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 252: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (cdr((r[3])));
    r[4] = (car((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = (cx__23609);
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 253: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (cdr((r[3])));
    r[4] = (car((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = (cx__23601);
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 254: /* clo ek r */
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
    /* ek r static-roots input-fix-exp global-vars k max-live r curry-alist */
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__23640);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    hreserve(hbsz(1), 10); /* 10 live regs */
    *--hp = obj_from_void(0);
    r[10] = (hendblk(1));
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[9];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(255);
    r[11] = (hendblk(8+1));
    hreserve(hbsz(2+1), 12); /* 12 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(265);
    r[12] = (hendblk(2+1));
    r[0] = (r[11]);
    r[1] = (r[12]);
    r[2] = (objptr_from_obj(r[4])[0]);
    goto gs_keep_23510;

case 255: /* clo ek r */
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
    /* ek r input-fix-exp global-vars k max-live r r curry-alist live-roots */
    (void)(objptr_from_obj(r[9])[0] = (r[1]));
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(256);
    r[10] = (hendblk(7+1));
    r[11] = (objptr_from_obj(r[9])[0]);
    r[11] = obj_from_bool(isnull((r[11])));
  if (bool_from_obj(r[11])) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = (cx__23561);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = obj_from_ktrap();
    r[1] = (r[11]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    goto s_l_v5894;
  } else {
    r[11] = (objptr_from_obj(r[9])[0]);
    hreserve(hbsz(1+1), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = obj_from_case(264);
    r[12] = (hendblk(1+1));
    r[0] = (r[12]);
    r[1] = (r[11]);
    goto s_loop_v5870;
  }

case 256: /* clo ek r */
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
s_l_v5894: /* ek r input-fix-exp global-vars k max-live r r curry-alist */
    hreserve(hbsz(1), 9); /* 9 live regs */
    *--hp = obj_from_void(0);
    r[9] = (hendblk(1));
    { fixnum_t v7802_tmp;
    r[10] = (objptr_from_obj(r[8])[0]);
    { /* length */
    int n; obj l = (r[10]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7802_tmp = (n); }
    r[10] = obj_from_fixnum((1) + (v7802_tmp)); }
    (void)(objptr_from_obj(r[9])[0] = (r[10]));
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(257);
    r[10] = (hendblk(8+1));
    r[0] = (r[10]);
    r[1] = obj_from_fixnum(0);
    r[2] = (mknull());
    r[3] = r[9];  
    goto s_loop_v5918;

s_loop_v5918: /* k i code ncases */
    { const fixnum_t v7800_i = fixnum_from_obj(r[1]);
    r[4] = (objptr_from_obj(r[3])[0]);
    r[4] = obj_from_bool((v7800_i) == fixnum_from_obj(r[4]));
  if (bool_from_obj(r[4])) {
    { fixnum_t v7801_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7801_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v7801_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[4] = (o); } }
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((((v7800_i) % (5)) == 0)) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = (cx__23475);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = obj_from_fixnum((v7800_i) + (1));
    r[2] = r[4];  
    /* r[3] */    
    goto s_loop_v5918;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = (cx__23472);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = obj_from_fixnum((v7800_i) + (1));
    r[2] = r[4];  
    /* r[3] */    
    goto s_loop_v5918;
  }
  } } 

case 257: /* clo ek r */
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
    /* ek r input-fix-exp global-vars k max-live r r r ncases */
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__23466);
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
    *--hp = (cx__23489);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (objptr_from_obj(r[9])[0]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (cx__23494);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (cx__23497);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    r[15] = (objptr_from_obj(r[3])[0]);
    hreserve(hbsz(6+1), 16); /* 16 live regs */
    *--hp = r[8];  
    *--hp = (r[14]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(260);
    r[16] = (hendblk(6+1));
    r[0] = (r[16]);
    r[1] = (r[15]);
    /* r[2] */    
    goto s_loop_v5903;

case 258: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v5903: /* k id input-fix-exp */
  if ((!(isnull((r[1]))))) {
    r[3] = (car((r[1])));
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(258);
    r[4] = (hendblk(1+1));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(259);
    r[4] = (hendblk(3+1));
    r[5] = (cdr((r[3])));
    r[5] = (car((r[5])));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("main"))));
  if (bool_from_obj(r[5])) {
    r[5+0] = (cx_var_2Dassigned_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r[5+3] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(258);
    r[5] = (hendblk(1+1));
    r[6+0] = obj_from_ktrap();
    r[6+1] = obj_from_bool(0);
    r[6+2] = r[5];  
    r[6+3] = r[1];  
    r[6+4] = r[0];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5905;
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

case 259: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v5905: /* ek r loop id k */
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

case 260: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r k max-live r r r r */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[7];  
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
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__23640);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10] = (objptr_from_obj(r[3])[0]);
    r[10] = obj_from_fixnum((2) + fixnum_from_obj(r[10]));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__233273);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = obj_from_ktrap();
    r[9+2] = r[8];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v5870: /* k id */
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
    *--hp = obj_from_case(261);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v5870;
  }

case 261: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(262);
    r[5] = (hendblk(2+1));
    r[6] = (cdr((r[4])));
    r[6] = (car((r[6])));
    r[6] = (hpushstr(7, newstring(symbolname(getsymbol((r[6]))))));
    r[0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    r[3] = (cx__23525);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 262: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(263);
    r[5] = (hendblk(2+1));
    r[0] = (cx_c_2Dformat_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cx__23533);
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 263: /* clo ek r */
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

case 264: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__23535);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx__23538);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx__23541);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx__23535);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx__23558);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 265: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k id input-fix-exp static-roots */
    r[4] = (objptr_from_obj(r[3])[0]);
    { /* assq */
    obj x = (r[1]), l = (r[4]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[4] = (l == mknull() ? obj_from_bool(0) : p); }
    r[4] = obj_from_bool(!bool_from_obj(r[4]));
  if (bool_from_obj(r[4])) {
    r[4+0] = (cx_var_2Dassigned_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[0];  
    r[4+2] = r[1];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
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

case 266: /* clo k c1 c2 */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k c1 c2 ids */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(267);
    r[4] = (hendblk(3+1));
    r[5+0] = (cx_posq);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[1])));
    r[5+3] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 267: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids c2 k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(268);
    r[5] = (hendblk(2+1));
    r[6+0] = (cx_posq);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[3])));
    r[6+3] = r[2];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 268: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_bool(fixnum_from_obj(r[3]) < fixnum_from_obj(r[1]));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 269: /* clo k exp */
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
    /* k exp used-global-labels lookup-label goto-alist add-curry! global-constant-labels global-labels static-roots */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(269);
    r[9] = (hendblk(7+1));
    hreserve(hbsz(10+1), 10); /* 10 live regs */
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[0];  
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(270);
    r[9] = (hendblk(10+1));
    r[0] = r[9];  
    /* r[1] */    
    goto gs_bgc_2Dexp_3F;

case 270: /* clo ek r */
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
    /* ek r used-global-labels lookup-label goto-alist add-curry! cc k global-constant-labels global-labels static-roots exp */
  if (bool_from_obj(r[1])) {
    r[12] = (vectorref((r[11]), (2)));
    r[12] = (car((r[12])));
    r[12] = (vectorref((r[12]), (1)));
    r[13] = (vectorref((r[11]), (2)));
    r[13] = (car((r[13])));
    r[13] = (vectorref((r[13]), (2)));
    r[14] = (vectorref((r[13]), (2)));
    r[14] = (vectorref((r[14]), (1)));
    r[13] = (vectorref((r[14]), (1)));
    r[14] = (vectorref((r[11]), (2)));
    r[14] = (car((r[14])));
    r[14] = (vectorref((r[14]), (2)));
    r[14] = (vectorref((r[14]), (1)));
    r[15] = (vectorref((r[11]), (1)));
    r[15] = (vectorref((r[15]), (2)));
    hreserve(hbsz(8+1), 16); /* 16 live regs */
    *--hp = (r[10]);
    *--hp = (r[12]);
    *--hp = (r[13]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (r[15]);
    *--hp = r[6];  
    *--hp = obj_from_case(271);
    r[16] = (hendblk(8+1));
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[16]);
    r[2] = (r[13]);
    r[3] = (r[14]);
    r[4] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((isvector((r[11])))) {
  if (((vectorlen((r[11]))) == (2))) {
    r[12] = (vectorref((r[11]), (0)));
    r[12] = obj_from_bool((r[12]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[12] = obj_from_bool(0);
  }
  } else {
    r[12] = obj_from_bool(0);
  }
  if (bool_from_obj(r[12])) {
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mksymbol(internsym("ok")));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_gvarassign_2Dexp_3F_23274((r[11])))) {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = (vectorref((r[11]), (2)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[11])))) {
  if (((vectorlen((r[11]))) == (4))) {
    r[12] = (vectorref((r[11]), (0)));
    r[12] = obj_from_bool((r[12]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[12] = obj_from_bool(0);
  }
  } else {
    r[12] = obj_from_bool(0);
  }
  if (bool_from_obj(r[12])) {
    r[12] = (vectorref((r[11]), (1)));
    r[13] = (vectorref((r[11]), (2)));
    r[14] = (vectorref((r[11]), (3)));
    hreserve(hbsz(4+1), 15); /* 15 live regs */
    *--hp = r[7];  
    *--hp = (r[14]);
    *--hp = r[6];  
    *--hp = (r[13]);
    *--hp = obj_from_case(272);
    r[15] = (hendblk(4+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[15]);
    r[2] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_23331((r[11])))) {
    r[12] = (vectorref((r[11]), (2)));
    r[13] = (vectorref((r[11]), (1)));
    r[13] = (vectorref((r[13]), (2)));
    hreserve(hbsz(3+1), 14); /* 14 live regs */
    *--hp = r[7];  
    *--hp = (r[13]);
    *--hp = r[6];  
    *--hp = obj_from_case(276);
    r[14] = (hendblk(3+1));
    r[0] = (r[14]);
    r[1] = (r[12]);
    r[2] = r[6];  
    goto s_loop_v5789;
  } else {
    hreserve(hbsz(7+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(277);
    r[12] = (hendblk(7+1));
    r[0] = (r[12]);
    r[1] = (r[11]);
    goto gs_curry_2Dexp_3F_23213;
  }
  }
  }
  }
  }

case 271: /* clo ek  */
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
    /* ek  cc exp2 k global-constant-labels global-labels lbid id static-roots */
    r[10] = (objptr_from_obj(r[9])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    (void)(objptr_from_obj(r[9])[0] = (r[10]));
    r[10] = (objptr_from_obj(r[6])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    (void)(objptr_from_obj(r[6])[0] = (r[10]));
  if (bool_from_obj(cxs_global_2Did_2Dconstant_3F_231038((r[8])))) {
    r[10] = (objptr_from_obj(r[5])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    (void)(objptr_from_obj(r[5])[0] = (r[10]));
  } else {
  }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 272: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  then-exp cc else-exp k */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(273);
    r[6] = (hendblk(3+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 273: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  cc else-exp k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 274: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v5789: /* k id cc */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(274);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(275);
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

case 275: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 276: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  cc body k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 277: /* clo ek r */
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
    /* ek r used-global-labels lookup-label goto-alist add-curry! cc k exp */
  if (bool_from_obj(r[1])) {
    r[9] = (vectorref((r[8]), (2)));
    r[9] = (vectorref((r[9]), (1)));
    r[9] = (vectorref((r[9]), (1)));
    r[10] = (vectorref((r[8]), (1)));
    { fixnum_t v7799_tmp;
    r[11] = (vectorref((r[8]), (2)));
    r[11] = (vectorref((r[11]), (2)));
    r[12] = (vectorref((r[8]), (1)));
    { /* length */
    int n; obj l = (r[12]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7799_tmp = (n); }
    { /* list-tail */
    obj l = (r[11]); int c = (v7799_tmp);
    while (c-- > 0) l = cdr(l);
    r[11] = (l); } }
    hreserve(hbsz(3+1), 12); /* 12 live regs */
    *--hp = r[7];  
    *--hp = (r[11]);
    *--hp = r[6];  
    *--hp = obj_from_case(278);
    r[12] = (hendblk(3+1));
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = r[9];  
    r[3] = (r[10]);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(281);
    r[9] = (hendblk(6+1));
    r[0] = (cx_labelapp_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 278: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  cc rands k */
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v5764;

case 279: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v5764: /* k id cc */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(279);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(280);
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

case 280: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 281: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r used-global-labels lookup-label cc k goto-alist exp */
  if (bool_from_obj(r[1])) {
    r[8] = (vectorref((r[7]), (1)));
    r[8] = (vectorref((r[8]), (1)));
    r[9] = (vectorref((r[7]), (2)));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = obj_from_case(282);
    r[10] = (hendblk(5+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[8];  
    r[3] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((isvector((r[7])))) {
  if (((vectorlen((r[7]))) == (3))) {
    r[8] = (vectorref((r[7]), (0)));
    r[8] = obj_from_bool((r[8]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[8] = obj_from_bool(0);
  }
  } else {
    r[8] = obj_from_bool(0);
  }
  if (bool_from_obj(r[8])) {
    r[8] = (vectorref((r[7]), (1)));
    r[9] = (vectorref((r[7]), (2)));
  if ((isvector((r[8])))) {
  if (((vectorlen((r[8]))) == (2))) {
    r[10] = (vectorref((r[8]), (0)));
    r[10] = obj_from_bool((r[10]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[10] = obj_from_bool(0);
  }
  } else {
    r[10] = obj_from_bool(0);
  }
  if (bool_from_obj(r[10])) {
    r[10] = (vectorref((r[8]), (1)));
    r[10] = (cxs_global_2Did_2Dconstant_3F_231038((r[10])));
  } else {
    r[10] = obj_from_bool(0);
  }
  if (bool_from_obj(r[10])) {
    r[10] = (vectorref((r[8]), (1)));
    r[11] = (objptr_from_obj(r[2])[0]);
    { /* memq */
    obj x = (r[10]), l = (r[11]);
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[12] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[12])) {
    r[11] = (r[11]);
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
  }
    (void)(objptr_from_obj(r[2])[0] = (r[11]));
  } else {
  }
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = obj_from_case(286);
    r[10] = (hendblk(3+1));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[7])))) {
  if (((vectorlen((r[7]))) == (4))) {
    r[8] = (vectorref((r[7]), (0)));
    r[8] = obj_from_bool((r[8]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[8] = obj_from_bool(0);
  }
  } else {
    r[8] = obj_from_bool(0);
  }
  if (bool_from_obj(r[8])) {
    r[8] = (vectorref((r[7]), (3)));
    r[0] = r[5];  
    r[1] = r[8];  
    r[2] = r[4];  
    goto s_loop_v5707;
  } else {
    r[8] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[8]))));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(291);
    r[9] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[8];  
    r[3] = (cx__23981);
    r[4] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cx__23977);
    r[3] = obj_from_bool(1);
    r[4] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }

case 282: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek  cc rands k id goto-alist */
    r[7] = (objptr_from_obj(r[6])[0]);
    { /* assq */
    obj x = (r[5]), l = (r[7]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[7] = (l == mknull() ? obj_from_bool(0) : p); }
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(283);
    r[8] = (hendblk(3+1));
  if (bool_from_obj(r[7])) {
    { bool_t v7798_tmp;
    { fixnum_t v7797_tmp;
    { fixnum_t v7796_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7797_tmp = (n); }
    r[9] = (cdr((r[7])));
    { /* length */
    int n; obj l = r[9];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v7796_tmp = (n); }
    v7798_tmp = ((v7797_tmp) == (v7796_tmp)); } }
    r[9] = obj_from_bool(!(v7798_tmp)); }
  if (bool_from_obj(r[9])) {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = (cx__231232);
    r[3] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_l_v5738;
  }
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10] = (objptr_from_obj(r[6])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[9] = (objptr_from_obj(r[6])[0] = (r[9]));
    r[0] = obj_from_ktrap();
    r[1] = r[9];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_l_v5738;
  }

case 283: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v5738: /* ek  cc rands k */
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v5739;

case 284: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v5739: /* k id cc */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(284);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(285);
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

case 285: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 286: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  cc rands k */
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v5720;

case 287: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v5720: /* k id cc */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(287);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(288);
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

case 288: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 289: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v5707: /* k id cc */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(289);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(290);
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

case 290: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 291: /* clo ek  */
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
    *--hp = obj_from_case(292);
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
    r[7+3] = (cx__23975);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 292: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(293);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23971);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 293: /* clo ek  */
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

case 294: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  input-fix-exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(295);
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
    r[7+3] = (cx__23975);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 295: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(296);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23971);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 296: /* clo ek  */
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

case 297: /* clo k id1 id2 */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id1 id2 id-rank */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(298);
    r[4] = (hendblk(3+1));
    r[5+0] = r[3];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 298: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id-rank id2 k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(299);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 299: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_bool(strcmp(stringchars((r[3])), stringchars((r[1]))) < 0);
    r += 4; /* shift reg wnd */
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
