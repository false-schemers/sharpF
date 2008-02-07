/* 1.sf */
#define MODULE module_1
#define LOAD() module_0(); 
extern void module_0(void); /* 0.sf */

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
extern int iseqv(obj x, obj y);
extern obj ismemv(obj x, obj l);
extern obj isassv(obj x, obj l);
extern int isequal(obj x, obj y);
extern obj ismember(obj x, obj l);
extern obj isassoc(obj x, obj l);

/* cx globals */
extern obj cx__2Acurrent_2Doutput_2Dport_2A; /* *current-output-port* */
extern obj cx_reset; /* reset */
extern obj cx_string_2D_3Efixnum; /* string->fixnum */
extern obj cx_string_2D_3Eflonum; /* string->flonum */
extern obj cx_write_2F3; /* write/3 */
obj cx__2Aalarm_2A; /* *alarm* */
obj cx__2Abackspace_2A; /* *backspace* */
obj cx__2Achar_2Dname_2Dtable_2A; /* *char-name-table* */
obj cx__2Adelete_2A; /* *delete* */
obj cx__2Adispatch_2Dtable_2Dlimit_2A; /* *dispatch-table-limit* */
obj cx__2Aescape_2A; /* *escape* */
obj cx__2Alinefeed_2A; /* *linefeed* */
obj cx__2Anon_2Dsymbol_2Dconstituents_2Dabove_2D127_2A; /* *non-symbol-constituents-above-127* */
obj cx__2Anul_2A; /* *nul* */
obj cx__2Apage_2A; /* *page* */
obj cx__2Areturn_2A; /* *return* */
obj cx__2Asharp_2Dmacros_2A; /* *sharp-macros* */
obj cx__2Atab_2A; /* *tab* */
obj cx__2Avtab_2A; /* *vtab* */
obj cx__2Awhitespaces_2A; /* *whitespaces* */
obj cx_binary_2Dsearch; /* binary-search */
obj cx_char_2Dhex_2Ddigit_3F; /* char-hex-digit? */
obj cx_char_2Dscalar_2Dvalue_2Dliteral_2Ddelimiter_3F; /* char-scalar-value-literal-delimiter? */
obj cx_char_2Dsemicolon_3F; /* char-semicolon? */
obj cx_char_2Dunicode_2Dwhitespace_3F; /* char-unicode-whitespace? */
obj cx_close_2Dbracket; /* close-bracket */
obj cx_close_2Dparen; /* close-paren */
obj cx_decode_2Descape; /* decode-escape */
obj cx_decode_2Dhex_2Ddigits; /* decode-hex-digits */
obj cx_delimiter_3F; /* delimiter? */
obj cx_dot; /* dot */
obj cx_gobble_2Dline; /* gobble-line */
obj cx_make_2Dimmutable_21; /* make-immutable! */
obj cx_make_2Dreader_2Dtoken; /* make-reader-token */
obj cx_parse_2Dtoken; /* parse-token */
obj cx_proper_2Dlist_3F; /* proper-list? */
obj cx_r_2Derror_2A; /* r-error* */
obj cx_read_2Ddispatch_2Dvector; /* read-dispatch-vector */
obj cx_read_2Dterminating_3F_2Dvector; /* read-terminating?-vector */
obj cx_read_2F1; /* read/1 */
obj cx_reader_2Dtoken_2Dmarker; /* reader-token-marker */
obj cx_reader_2Dtoken_3F; /* reader-token? */
obj cx_reverse_2Dlist_2D_3Estring; /* reverse-list->string */
obj cx_set_2Dsharp_2Dmacro_21; /* set-sharp-macro! */
obj cx_set_2Dstandard_2Dread_2Dmacro_21; /* set-standard-read-macro! */
obj cx_set_2Dstandard_2Dsyntax_21; /* set-standard-syntax! */
obj cx_strange_2Dsymbol_2Dnames; /* strange-symbol-names */
obj cx_sub_2Dread; /* sub-read */
obj cx_sub_2Dread_2Dcarefully; /* sub-read-carefully */
obj cx_sub_2Dread_2Dconstituent; /* sub-read-constituent */
obj cx_sub_2Dread_2Dlist; /* sub-read-list */
obj cx_sub_2Dread_2Dlist_2Dbracket; /* sub-read-list-bracket */
obj cx_sub_2Dread_2Dlist_2Dparen; /* sub-read-list-paren */
obj cx_sub_2Dread_2Dtoken; /* sub-read-token */
static obj cx__23131; /* constant #131 */
static obj cx__23161; /* constant #161 */
static obj cx__23162; /* constant #162 */
static obj cx__2318; /* constant #18 */
static obj cx__23195; /* constant #195 */
static obj cx__23204; /* constant #204 */
static obj cx__23211; /* constant #211 */
static obj cx__2325; /* constant #25 */
static obj cx__23255; /* constant #255 */
static obj cx__2327; /* constant #27 */
static obj cx__23303; /* constant #303 */
static obj cx__23317; /* constant #317 */
static obj cx__23318; /* constant #318 */
static obj cx__23378; /* constant #378 */
static obj cx__2338; /* constant #38 */
static obj cx__23385; /* constant #385 */
static obj cx__2341; /* constant #41 */
static obj cx__23431; /* constant #431 */
static obj cx__23441; /* constant #441 */
static obj cx__23452; /* constant #452 */
static obj cx__2347; /* constant #47 */
static obj cx__23506; /* constant #506 */
static obj cx__23520; /* constant #520 */
static obj cx__23612; /* constant #612 */
static obj cx__23617; /* constant #617 */
static obj cx__23625; /* constant #625 */
static obj cx__23636; /* constant #636 */
static obj cx__2364; /* constant #64 */
static obj cx__23641; /* constant #641 */
static obj cx__2367; /* constant #67 */
static obj cx__23674; /* constant #674 */
static obj cx__23721; /* constant #721 */
static obj cx__23723; /* constant #723 */
static obj cx__23763; /* constant #763 */
static obj cx__23773; /* constant #773 */
static obj cx__23806; /* constant #806 */
static obj cx__23826; /* constant #826 */
static obj cx__2383; /* constant #83 */
static obj cx__2384; /* constant #84 */
static obj cx__2385; /* constant #85 */
static obj cx__2394; /* constant #94 */

/* helper functions */
/* char-hex-digit? */
static obj cxs_char_2Dhex_2Ddigit_3F(obj v398_c)
{ 
  { /* let */
    obj v400_scalar_2Dvalue = obj_from_fixnum((fixnum_t)char_from_obj(v398_c));
  { /* let */
    obj v401_x = ((fixnum_from_obj(v400_scalar_2Dvalue) >= (48)) ? obj_from_bool(fixnum_from_obj(v400_scalar_2Dvalue) <= (57)) : obj_from_bool(0));
  if (bool_from_obj(v401_x)) {
    return (v401_x);
  } else {
  { /* let */
    obj v402_x = ((fixnum_from_obj(v400_scalar_2Dvalue) >= (65)) ? obj_from_bool(fixnum_from_obj(v400_scalar_2Dvalue) <= (70)) : obj_from_bool(0));
    return (bool_from_obj(v402_x) ? (v402_x) : ((fixnum_from_obj(v400_scalar_2Dvalue) >= (97)) ? obj_from_bool(fixnum_from_obj(v400_scalar_2Dvalue) <= (102)) : obj_from_bool(0)));
  }
  }
  }
  }
}

/* proper-list? */
static obj cxs_proper_2Dlist_3F(obj v707_x)
{ 
  s_proper_2Dlist_3F:
  if ((isnull((v707_x)))) {
    return obj_from_bool(1);
  } else {
  if ((ispair((v707_x)))) {
  { /* let */
    obj v1070_tmp = (cdr((v707_x)));
    /* tail call */
    v707_x = (v1070_tmp);
    goto s_proper_2Dlist_3F;
  }
  } else {
    return obj_from_bool(0);
  }
  }
}

/* binary-search */
static obj cxs_binary_2Dsearch(obj v855_vec, obj v854_val)
{ 
  { /* letrec */
    obj v860_low;
    obj v859_high;
  { /* let */
    obj v1069_tmp = obj_from_fixnum(vectorlen((v855_vec)));
    obj v1068_tmp = obj_from_fixnum(0);
    /* tail call */
    v860_low = (v1068_tmp);
    v859_high = (v1069_tmp);
    goto s_loop;
  }
  s_loop:
  if ((fixnum_from_obj(v860_low) < (fixnum_from_obj(v859_high) - (1)))) {
  { /* let */
    obj v865_pos = obj_from_fixnum((fixnum_from_obj(v860_low) + fixnum_from_obj(v859_high)) / (2));
  { /* let */
    obj v868_at = (vectorref((v855_vec), fixnum_from_obj(v865_pos)));
  if ((fixnum_from_obj(v854_val) == fixnum_from_obj(v868_at))) {
    return (v865_pos);
  } else {
  if ((fixnum_from_obj(v854_val) < fixnum_from_obj(v868_at))) {
  { /* let */
    obj v1067_tmp = (v865_pos);
    obj v1066_tmp = (v860_low);
    /* tail call */
    v860_low = (v1066_tmp);
    v859_high = (v1067_tmp);
    goto s_loop;
  }
  } else {
  { /* let */
    obj v1065_tmp = (v859_high);
    obj v1064_tmp = (v865_pos);
    /* tail call */
    v860_low = (v1064_tmp);
    v859_high = (v1065_tmp);
    goto s_loop;
  }
  }
  }
  }
  }
  } else {
    return ((fixnum_from_obj(v860_low) < fixnum_from_obj(v859_high)) ? ((fixnum_from_obj(v854_val) == fixnum_from_obj(vectorref((v855_vec), fixnum_from_obj(v860_low)))) ? (v860_low) : obj_from_bool(0)) : obj_from_bool(0));
  }
  }
}

/* make-immutable! */
static obj cxs_make_2Dimmutable_21(obj v878_x)
{ 
    return (v878_x);
}

/* gc roots */
static obj *globv[] = {
  &cx__2Aalarm_2A,
  &cx__2Abackspace_2A,
  &cx__2Achar_2Dname_2Dtable_2A,
  &cx__2Adelete_2A,
  &cx__2Adispatch_2Dtable_2Dlimit_2A,
  &cx__2Aescape_2A,
  &cx__2Alinefeed_2A,
  &cx__2Anon_2Dsymbol_2Dconstituents_2Dabove_2D127_2A,
  &cx__2Anul_2A,
  &cx__2Apage_2A,
  &cx__2Areturn_2A,
  &cx__2Asharp_2Dmacros_2A,
  &cx__2Atab_2A,
  &cx__2Avtab_2A,
  &cx__2Awhitespaces_2A,
  &cx_close_2Dbracket,
  &cx_close_2Dparen,
  &cx_dot,
  &cx_read_2Ddispatch_2Dvector,
  &cx_read_2Dterminating_3F_2Dvector,
  &cx_reader_2Dtoken_2Dmarker,
  &cx_strange_2Dsymbol_2Dnames,
  &cx__23131,
  &cx__23161,
  &cx__23162,
  &cx__2318,
  &cx__23195,
  &cx__23204,
  &cx__23211,
  &cx__2325,
  &cx__23255,
  &cx__2327,
  &cx__23303,
  &cx__23317,
  &cx__23318,
  &cx__23378,
  &cx__2338,
  &cx__23385,
  &cx__2341,
  &cx__23431,
  &cx__23441,
  &cx__23452,
  &cx__2347,
  &cx__23506,
  &cx__23520,
  &cx__23612,
  &cx__23617,
  &cx__23625,
  &cx__23636,
  &cx__2364,
  &cx__23641,
  &cx__2367,
  &cx__23674,
  &cx__23721,
  &cx__23723,
  &cx__23763,
  &cx__23773,
  &cx__23806,
  &cx__23826,
  &cx__2383,
  &cx__2384,
  &cx__2385,
  &cx__2394,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[125] = {
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
#define MAX_LIVEREGS 38
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
    cx__2318 = (hpushstr(0, newstring("Reader error: ")));
    { /* vector */
    hreserve(hbsz(36+1), 0); /* 0 live regs */
    *--hp = obj_from_fixnum(187);
    *--hp = obj_from_fixnum(173);
    *--hp = obj_from_fixnum(171);
    *--hp = obj_from_fixnum(160);
    *--hp = obj_from_fixnum(159);
    *--hp = obj_from_fixnum(158);
    *--hp = obj_from_fixnum(157);
    *--hp = obj_from_fixnum(156);
    *--hp = obj_from_fixnum(155);
    *--hp = obj_from_fixnum(154);
    *--hp = obj_from_fixnum(153);
    *--hp = obj_from_fixnum(152);
    *--hp = obj_from_fixnum(151);
    *--hp = obj_from_fixnum(150);
    *--hp = obj_from_fixnum(149);
    *--hp = obj_from_fixnum(148);
    *--hp = obj_from_fixnum(147);
    *--hp = obj_from_fixnum(146);
    *--hp = obj_from_fixnum(145);
    *--hp = obj_from_fixnum(144);
    *--hp = obj_from_fixnum(143);
    *--hp = obj_from_fixnum(142);
    *--hp = obj_from_fixnum(141);
    *--hp = obj_from_fixnum(140);
    *--hp = obj_from_fixnum(139);
    *--hp = obj_from_fixnum(138);
    *--hp = obj_from_fixnum(137);
    *--hp = obj_from_fixnum(136);
    *--hp = obj_from_fixnum(135);
    *--hp = obj_from_fixnum(134);
    *--hp = obj_from_fixnum(133);
    *--hp = obj_from_fixnum(132);
    *--hp = obj_from_fixnum(131);
    *--hp = obj_from_fixnum(130);
    *--hp = obj_from_fixnum(129);
    *--hp = obj_from_fixnum(128);
    *--hp = obj_from_size(VECTOR_BTAG);
    cx__2325 = (hendblk(36+1)); }
    { /* vector */
    hreserve(hbsz(7+1), 0); /* 0 live regs */
    *--hp = obj_from_fixnum(160);
    *--hp = obj_from_fixnum(32);
    *--hp = obj_from_fixnum(13);
    *--hp = obj_from_fixnum(12);
    *--hp = obj_from_fixnum(11);
    *--hp = obj_from_fixnum(10);
    *--hp = obj_from_fixnum(9);
    *--hp = obj_from_size(VECTOR_BTAG);
    cx__2327 = (hendblk(7+1)); }
    cx__2338 = (hpushstr(0, newstring("unexpected datum")));
    cx__2341 = (hpushstr(0, newstring("discarding extraneous right bracket")));
    cx__2347 = (hpushstr(0, newstring("extraneous right parenthesis")));
    cx__2364 = (hpushstr(0, newstring("unexpected token")));
    cx__2367 = (hpushstr(0, newstring("unexpected end of file")));
    cx__2383 = (hpushstr(0, newstring("unexpected right parenthesis")));
    cx__2384 = (hpushstr(0, newstring("unexpected right bracket")));
    { static char s[] = { 117, 110, 101, 120, 112, 101, 99, 116, 101, 100, 32, 34, 32, 46, 32, 34, 0 };
    cx__2385 = (hpushstr(0, newstring(s))); }
    cx__2394 = (hpushstr(0, newstring("illegal character read")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_fixnum(13);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_fixnum(12);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_fixnum(11);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_fixnum(10);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_fixnum(9);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_fixnum(32);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23131 = (hendblk(3)); }
    cx__23161 = (hpushstr(0, newstring("NOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")));
    { static char s[] = { 33, 36, 37, 38, 42, 43, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 60, 61, 62, 63, 64, 94, 95, 126, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 0 };
    cx__23162 = (hpushstr(0, newstring(s))); }
    cx__23195 = (hpushstr(0, newstring("randomness after form after dot")));
    cx__23204 = (hpushstr(0, newstring("eof inside list -- unbalanced parentheses")));
    cx__23211 = (hpushstr(0, newstring("missing car -- ( immediately followed by .")));
    cx__23255 = (hpushstr(0, newstring("end of file after ,")));
    cx__23303 = (hpushstr(0, newstring("end of file within a string")));
    cx__23317 = (hpushstr(0, newstring("invalid escaped character in string")));
    cx__23318 = (hpushstr(0, newstring("string literal")));
    cx__23378 = (hpushstr(0, newstring("invalid hex digit in a ")));
    cx__23385 = (hpushstr(0, newstring("premature end of a scalar-value literal within a ")));
    cx__23431 = (hpushstr(0, newstring("end of file after #")));
    cx__23441 = (hpushstr(0, newstring("unknown # syntax")));
    { static char s[] = { 117, 110, 107, 110, 111, 119, 110, 32, 115, 121, 110, 116, 97, 120, 32, 35, 92, 102, 0 };
    cx__23452 = (hpushstr(0, newstring(s))); }
    cx__23506 = (hpushstr(0, newstring("end of file after &")));
    cx__23520 = (hpushstr(0, newstring("end of file in #| comment")));
    { static char s[] = { 117, 110, 107, 110, 111, 119, 110, 32, 35, 92, 32, 110, 97, 109, 101, 0 };
    cx__23612 = (hpushstr(0, newstring(s))); }
    cx__23617 = (hpushstr(0, newstring("char literal")));
    { static char s[] = { 101, 110, 100, 32, 111, 102, 32, 102, 105, 108, 101, 32, 97, 102, 116, 101, 114, 32, 35, 92, 0 };
    cx__23625 = (hpushstr(0, newstring(s))); }
    cx__23636 = (hpushstr(0, newstring("dot in #(...)")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(57);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(56);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(55);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(54);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(53);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(52);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(51);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(50);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(49);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(48);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23641 = (hendblk(3)); }
    { static char s[] = { 117, 110, 107, 110, 111, 119, 110, 32, 35, 92, 60, 100, 105, 103, 105, 116, 115, 62, 32, 109, 97, 99, 114, 111, 0 };
    cx__23674 = (hpushstr(0, newstring(s))); }
    cx__23721 = (hpushstr(0, newstring("unsupported number syntax")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(101);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(105);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(120);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(100);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(111);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(98);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23723 = (hendblk(3)); }
    cx__23763 = (hpushstr(0, newstring("symbol literal")));
    cx__23773 = (hpushstr(0, newstring("invalid escape sequence in a symbol")));
    cx__23806 = (hpushstr(0, newstring(".")));
    r[0] = (hpushstr(0, newstring("...")));
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = (mknull());
    *--hp = r[0];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    r[1] = (hpushstr(1, newstring("-")));
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    r[1] = (hpushstr(1, newstring("+")));
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23826 = (hendblk(3)); }
    { static obj c[] = { obj_from_case(1) }; cx_r_2Derror_2A = (obj)c; }
    cx__2Anon_2Dsymbol_2Dconstituents_2Dabove_2D127_2A = (cx__2325);
    cx__2Awhitespaces_2A = (cx__2327);
    { static obj c[] = { obj_from_case(7) }; cx_read_2F1 = (obj)c; }
    { static obj c[] = { obj_from_case(10) }; cx_sub_2Dread_2Dcarefully = (obj)c; }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("reader-token")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx_reader_2Dtoken_2Dmarker = (hendblk(3)); }
    { static obj c[] = { obj_from_case(13) }; cx_make_2Dreader_2Dtoken = (obj)c; }
    { static obj c[] = { obj_from_case(14) }; cx_reader_2Dtoken_3F = (obj)c; }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (cx__2383);
    *--hp = (cx_reader_2Dtoken_2Dmarker);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx_close_2Dparen = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (cx__2384);
    *--hp = (cx_reader_2Dtoken_2Dmarker);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx_close_2Dbracket = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (cx__2385);
    *--hp = (cx_reader_2Dtoken_2Dmarker);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx_dot = (hendblk(3)); }
    cx__2Adispatch_2Dtable_2Dlimit_2A = obj_from_fixnum(128);
    hreserve(hbsz(0+1), 0); /* 0 live regs */
    *--hp = obj_from_case(15);
    r[0] = (hendblk(0+1));
    { /* make-vector */
    obj o; int i = 0, c = (128);
    hreserve(hbsz(c+1), 1); /* 1 live regs */
    o = r[0];   /* gc-safe */
    while (i++ < c) *--hp = o;
    *--hp = obj_from_size(VECTOR_BTAG);
    cx_read_2Ddispatch_2Dvector = (hendblk(c+1)); }
    { /* make-vector */
    obj o; int i = 0, c = (128);
    hreserve(hbsz(c+1), 0); /* 0 live regs */
    o = obj_from_bool(1); /* gc-safe */
    while (i++ < c) *--hp = o;
    *--hp = obj_from_size(VECTOR_BTAG);
    cx_read_2Dterminating_3F_2Dvector = (hendblk(c+1)); }
    { static obj c[] = { obj_from_case(16) }; cx_set_2Dstandard_2Dsyntax_21 = (obj)c; }
    { static obj c[] = { obj_from_case(17) }; cx_sub_2Dread = (obj)c; }
    hreserve(hbsz(0+1), 0); /* 0 live regs */
    *--hp = obj_from_case(19);
    r[0] = (hendblk(0+1));
    r[1+0] = r[0];  
    r[1+1] = (cx__23131);
    r += 1; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v2817;

case 1: /* r-error* k reason args */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_r_2Derror_2A: /* k reason args */
    (void)(fputc('\n', oportdata((cx__2Acurrent_2Doutput_2Dport_2A))));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(2);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (cx__2318);
    r[4+3] = obj_from_bool(1);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 2: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  reason args k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(3);
    r[5] = (hendblk(2+1));
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    r[3] = obj_from_bool(1);
    r[4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 3: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  args k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(6);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop;

case 4: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
    (void)(fputc((32), oportdata((cx__2Acurrent_2Doutput_2Dport_2A))));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(4);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(5);
    r[3] = (hendblk(3+1));
  if ((isstring((r[2])))) {
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[2];  
    r[4+3] = obj_from_bool(1);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[2];  
    r[4+3] = obj_from_bool(0);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }

case 5: /* clo ek  */
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

case 6: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    (void)(fputc('\n', oportdata((cx__2Acurrent_2Doutput_2Dport_2A))));
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 7: /* read/1 k port */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k port */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(8);
    r[2] = (hendblk(2+1));
    r[0] = r[2];  
    /* r[1] */    
    goto gs_sub_2Dread;

case 8: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(9);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    /* r[1] */    
    goto gs_reader_2Dtoken_3F;

case 9: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k r */
  if ((!bool_from_obj(r[1]))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((r[4]) == (cx_close_2Dparen))) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[3];  
    r[1] = (cx__2347);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  } else {
  if (((r[4]) == (cx_close_2Dbracket))) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[3];  
    r[1] = (cx__2341);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6] = (cdr((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[3];  
    r[1] = (cx__2338);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  }
  }
  }

case 10: /* sub-read-carefully k port */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_sub_2Dread_2Dcarefully: /* k port */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(11);
    r[2] = (hendblk(2+1));
    r[0] = r[2];  
    /* r[1] */    
    goto gs_sub_2Dread;

case 11: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k */
  if ((iseof((r[1])))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[3];  
    r[1] = (cx__2367);
    r[2] = r[4];  
    goto gs_r_2Derror_2A;
  } else {
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(12);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    /* r[1] */    
    goto gs_reader_2Dtoken_3F;
  }

case 12: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r port k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6] = (cdr((r[2])));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__2364);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 13: /* make-reader-token k message */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k message */
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = (cx_reader_2Dtoken_2Dmarker);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 14: /* reader-token? k form */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_reader_2Dtoken_3F: /* k form */
  if ((ispair((r[1])))) {
    r[2] = (car((r[1])));
    r[2] = obj_from_bool((r[2]) == (cx_reader_2Dtoken_2Dmarker));
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

case 15: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
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
    r[4+0] = r[0];  
    r[4+1] = (cx__2394);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;

case 16: /* set-standard-syntax! k char terminating? reader */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k char terminating? reader */
    { const char_t v2958_char = char_from_obj(r[1]);
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(v2958_char))) = (r[3]));
    r[4] = obj_from_void(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(v2958_char))) = (r[2]));
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump; } 

case 17: /* sub-read k port */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_sub_2Dread: /* k port */
    { int c = fgetc(iportdata((r[1])));
    r[2] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((((fixnum_t)char_from_obj(r[2])) < (128))) {
    r[3+0] = (vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)char_from_obj(r[2]))));
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[0];  
    r[3+2] = r[2];  
    r[3+3] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((isalpha(char_from_obj(r[2])))) {
    r[3+0] = r[0];  
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_sub_2Dread_2Dconstituent;
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
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[0];  
    r[4+1] = (cx__2394);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  }
  }
  }

s_loop_v2817: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(18);
    r[3] = (hendblk(0+1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), fixnum_from_obj(r[2])) = (r[3]));
    r[2] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[2];  
    goto s_loop_v2817;
  }

case 18: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    /* r[0] */    
    r[1] = r[2];  
    goto gs_sub_2Dread;

case 19: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    { static obj c[] = { obj_from_case(20) }; cx_sub_2Dread_2Dconstituent = (obj)c; }
    { /* string-append */
    int *d = stringcat(stringdata((cx__23162)), stringdata((cx__23161)));
    r[2] = (hpushstr(2, d)); }
    { /* string->list */
    int c = (stringlen((r[2])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    s = stringchars((r[2])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[2] = (l); }
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(22);
    r[3] = (hendblk(0+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v2794;

case 20: /* sub-read-constituent k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_sub_2Dread_2Dconstituent: /* k c port */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(21);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    /* r[1] */    
    /* r[2] */    
    goto gs_sub_2Dread_2Dtoken;

case 21: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k */
    r[0] = r[3];  
    /* r[1] */    
    /* r[2] */    
    goto gs_parse_2Dtoken;

s_loop_v2794: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)char_from_obj(r[2]))) = (cx_sub_2Dread_2Dconstituent));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)char_from_obj(r[2]))) = obj_from_bool(0));
    r[2] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[2];  
    goto s_loop_v2794;
  }

case 22: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    { static obj c[] = { obj_from_case(23) }; cx_set_2Dstandard_2Dread_2Dmacro_21 = (obj)c; }
    { static obj c[] = { obj_from_case(24) }; cx_sub_2Dread_2Dlist = (obj)c; }
    { static obj c[] = { obj_from_case(31) }; cx_sub_2Dread_2Dlist_2Dparen = (obj)c; }
    { static obj c[] = { obj_from_case(32) }; cx_sub_2Dread_2Dlist_2Dbracket = (obj)c; }
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(40))) = (cx_sub_2Dread_2Dlist_2Dparen));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(40))) = obj_from_bool(1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(91))) = (cx_sub_2Dread_2Dlist_2Dbracket));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(91))) = obj_from_bool(1));
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(33);
    r[2] = (hendblk(0+1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(41))) = (r[2]));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(41))) = obj_from_bool(1));
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(34);
    r[2] = (hendblk(0+1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(93))) = (r[2]));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(93))) = obj_from_bool(1));
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(35);
    r[2] = (hendblk(0+1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(39))) = (r[2]));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(39))) = obj_from_bool(1));
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(37);
    r[2] = (hendblk(0+1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(96))) = (r[2]));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(96))) = obj_from_bool(1));
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(39);
    r[2] = (hendblk(0+1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(44))) = (r[2]));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(44))) = obj_from_bool(1));
    cx__2Anul_2A = obj_from_char((char_t)(0));
    cx__2Aalarm_2A = obj_from_char((char_t)(7));
    cx__2Abackspace_2A = obj_from_char((char_t)(8));
    cx__2Atab_2A = obj_from_char((char_t)(9));
    cx__2Alinefeed_2A = obj_from_char((char_t)(10));
    cx__2Avtab_2A = obj_from_char((char_t)(11));
    cx__2Apage_2A = obj_from_char((char_t)(12));
    cx__2Areturn_2A = obj_from_char((char_t)(13));
    cx__2Aescape_2A = obj_from_char((char_t)(27));
    cx__2Adelete_2A = obj_from_char((char_t)(127));
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(42);
    r[2] = (hendblk(0+1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(34))) = (r[2]));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(34))) = obj_from_bool(1));
    { static obj c[] = { obj_from_case(45) }; cx_decode_2Descape = (obj)c; }
    { static obj c[] = { obj_from_case(51) }; cx_char_2Dsemicolon_3F = (obj)c; }
    { static obj c[] = { obj_from_case(52) }; cx_decode_2Dhex_2Ddigits = (obj)c; }
    { static obj c[] = { obj_from_case(56) }; cx_char_2Dhex_2Ddigit_3F = (obj)c; }
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(57);
    r[2] = (hendblk(0+1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(59))) = (r[2]));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(59))) = obj_from_bool(1));
    { static obj c[] = { obj_from_case(59) }; cx_gobble_2Dline = (obj)c; }
    cx__2Asharp_2Dmacros_2A = (mknull());
    { static obj c[] = { obj_from_case(60) }; cx_set_2Dsharp_2Dmacro_21 = (obj)c; }
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(61);
    r[2] = (hendblk(0+1));
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(35))) = (r[2]));
    (void)(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(35))) = obj_from_bool(0));
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(63);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(122);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(102);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 23: /* set-standard-read-macro! k c terminating? proc */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k c terminating? proc */
    { const char_t v2957_c = char_from_obj(r[1]);
    (void)(vectorref((cx_read_2Ddispatch_2Dvector), ((fixnum_t)(v2957_c))) = (r[3]));
    r[4] = obj_from_void(vectorref((cx_read_2Dterminating_3F_2Dvector), ((fixnum_t)(v2957_c))) = (r[2]));
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump; } 

case 24: /* sub-read-list k c port close-token */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_sub_2Dread_2Dlist: /* k c port close-token */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(25);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_sub_2Dread;

case 25: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r close-token port k */
  if (((r[1]) == (cx_dot))) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23211);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  } else {
    r[0] = r[4];  
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    goto s_recur;
  }

case 26: /* clo k form */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_recur: /* k form close-token port */
  if ((iseof((r[1])))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__23204);
    r[2] = r[4];  
    goto gs_r_2Derror_2A;
  } else {
  if (((r[1]) == (r[2]))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((r[1]) == (cx_dot))) {
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(27);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    goto gs_sub_2Dread_2Dcarefully;
  } else {
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(26);
    r[4] = (hendblk(2+1));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = obj_from_case(29);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    goto gs_sub_2Dread;
  }
  }
  }

case 27: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k close-token */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(28);
    r[5] = (hendblk(4+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto gs_sub_2Dread;

case 28: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k r close-token */
  if (((r[1]) == (r[5]))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = r[3];  
    r[7+1] = (cx__23195);
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  }

case 29: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r recur k form */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(30);
    r[5] = (hendblk(2+1));
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 30: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k form */
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

case 31: /* sub-read-list-paren k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_sub_2Dread_2Dlist_2Dparen: /* k c port */
    r[3+0] = r[0];  
    r[3+1] = r[1];  
    r[3+2] = r[2];  
    r[3+3] = (cx_close_2Dparen);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_sub_2Dread_2Dlist;

case 32: /* sub-read-list-bracket k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    r[3+0] = r[0];  
    r[3+1] = r[1];  
    r[3+2] = r[2];  
    r[3+3] = (cx_close_2Dbracket);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_sub_2Dread_2Dlist;

case 33: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx_close_2Dparen);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 34: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx_close_2Dbracket);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 35: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(36);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;

case 36: /* clo ek r */
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
    *--hp = (mksymbol(internsym("quote")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 37: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(38);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;

case 38: /* clo ek r */
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
    *--hp = (mksymbol(internsym("quasiquote")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 39: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { FILE *p = iportdata((r[2])); int c = fgetc(p); ungetc(c, p);
    r[3] = (c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(40);
    r[4] = (hendblk(2+1));
  if ((iseof((r[3])))) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23255);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  } else {
  if ((char_from_obj(r[3]) == (64))) {
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    r[5] = (mksymbol(internsym("unquote-splicing")));
  } else {
    r[5] = (mksymbol(internsym("unquote")));
  }
    r[6+0] = obj_from_ktrap();
    r[6+1] = r[5];  
    r[6+2] = r[2];  
    r[6+3] = r[0];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v2658;
  }

case 40: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v2658: /* ek r port k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(41);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;

case 41: /* clo ek r */
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

case 42: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    r[3+0] = r[0];  
    r[3+1] = (mknull());
    r[3+2] = obj_from_fixnum(0);
    r[3+3] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v2636;

case 43: /* clo k l i */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v2636: /* k l i port */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(43);
    r[4] = (hendblk(1+1));
    { int c = fgetc(iportdata((r[3])));
    r[5] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[5])))) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__23303);
    r[2] = r[6];  
    goto gs_r_2Derror_2A;
  } else {
  if ((char_from_obj(r[5]) == (92))) {
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(44);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[3];  
    goto gs_decode_2Descape;
  } else {
  if ((char_from_obj(r[5]) == (34))) {
    { fixnum_t v2955_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2955_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v2955_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[1];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[6] = (o); } }
    { fixnum_t v2956_tmp;
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2956_tmp = (n); }
    { /* list->string */
    int i, c = (v2956_tmp); 
    obj o = hpushstr(7, allocstring(c, ' ')); /* 7 live regs */
    obj l = r[6];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[6] = (o); } }
    r[7+0] = r[0];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[6];  
    r[2] = obj_from_fixnum(fixnum_from_obj(r[2]) + (1));
    /* r[3] */    
    goto s_loop_v2636;
  }
  }
  }

case 44: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop i l k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[6];  
    r[7+3] = obj_from_fixnum(fixnum_from_obj(r[3]) + (1));
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 45: /* decode-escape k port */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_decode_2Descape: /* k port */
    { int c = fgetc(iportdata((r[1])));
    r[2] = (c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(46);
    r[3] = (hendblk(3+1));
  if ((iseof((r[2])))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[3];  
    r[5+1] = (cx__23303);
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  } else {
    r[4+0] = obj_from_ktrap();
    r[4+1] = obj_from_void(0);
    r[4+2] = r[1];  
    r[4+3] = r[0];  
    r[4+4] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v2604;
  }

case 46: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v2604: /* ek  port k c */
  if (((char_from_obj(r[4]) == (92)) || (char_from_obj(r[4]) == (34)))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[4]) == char_from_obj(cx__2Alinefeed_2A))) {
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v2617;
  } else {
  if ((char_from_obj(r[4]) == (97))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__2Aalarm_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[4]) == (98))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__2Abackspace_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[4]) == (116))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__2Atab_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[4]) == (110))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__2Alinefeed_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[4]) == (118))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__2Avtab_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[4]) == (102))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__2Apage_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[4]) == (114))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__2Areturn_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[4]) == (101))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__2Aescape_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[4]) == (120))) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(49);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(0+1), 6); /* 6 live regs */
    *--hp = obj_from_case(50);
    r[6] = (hendblk(0+1));
    r[0] = r[5];  
    r[1] = r[2];  
    r[2] = r[6];  
    r[3] = (cx__23318);
    goto gs_decode_2Dhex_2Ddigits;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[3];  
    r[1] = (cx__23317);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
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

case 47: /* clo k */
    assert(rc == 2);
    { obj* p = objptr_from_obj(r[0]);
    r[1+1] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v2617: /* k port */
    { FILE *p = iportdata((r[1])); int c = fgetc(p); ungetc(c, p);
    r[2] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[2])))) {
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[0];  
    r[4+1] = (cx__23303);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = obj_from_case(47);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(48);
    r[3] = (hendblk(3+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_char_2Dunicode_2Dwhitespace_3F;
  }

case 48: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop k port */
  if (bool_from_obj(r[1])) {
    { int c = fgetc(iportdata((r[4])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
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

case 49: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k port */
    { int c = fgetc(iportdata((r[3])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    r[4] = r[1];  
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 50: /* clo k c */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k c */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isequal((r[1]), obj_from_char(59)));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 51: /* char-semicolon? k c */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k c */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isequal((r[1]), obj_from_char(59)));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 52: /* decode-hex-digits k port delimiter? desc */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_decode_2Dhex_2Ddigits: /* k port delimiter? desc */
    r[4+0] = r[0];  
    r[4+1] = (mknull());
    r[4+2] = r[2];  
    r[4+3] = r[3];  
    r[4+4] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v2570;

case 53: /* clo k rev-digits */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_loop_v2570: /* k rev-digits delimiter? desc port */
    { FILE *p = iportdata((r[4])); int c = fgetc(p); ungetc(c, p);
    r[5] = (c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(53);
    r[6] = (hendblk(3+1));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = obj_from_case(54);
    r[6] = (hendblk(6+1));
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[5];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 54: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop port desc c rev-digits k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(1+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = obj_from_case(55);
    r[8] = (hendblk(1+1));
    { fixnum_t v2953_tmp;
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2953_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v2953_tmp);
    hreserve(hbsz(3)*c, 9); /* 9 live regs */
    l = r[6];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[9] = (o); } }
    { fixnum_t v2954_tmp;
    { /* length */
    int n; obj l = r[9];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2954_tmp = (n); }
    { /* list->string */
    int i, c = (v2954_tmp); 
    obj o = hpushstr(10, allocstring(c, ' ')); /* 10 live regs */
    obj l = r[9];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[9] = (o); } }
    r[0] = (cx_string_2D_3Efixnum);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[9];  
    r[3] = obj_from_fixnum(16);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((iseof((r[5])))) {
    { /* string-append */
    int *d = stringcat(stringdata((cx__23385)), stringdata((r[4])));
    r[8] = (hpushstr(8, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[7];  
    r[1] = r[8];  
    r[2] = r[9];  
    goto gs_r_2Derror_2A;
  } else {
    r[8] = (cxs_char_2Dhex_2Ddigit_3F((r[5])));
    r[8] = obj_from_bool(!bool_from_obj(r[8]));
  if (bool_from_obj(r[8])) {
    { /* string-append */
    int *d = stringcat(stringdata((cx__23378)), stringdata((r[4])));
    r[8] = (hpushstr(8, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[7];  
    r[1] = r[8];  
    r[2] = r[9];  
    goto gs_r_2Derror_2A;
  } else {
    { int c = fgetc(iportdata((r[3])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }

case 55: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = r[2];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_char((char_t)fixnum_from_obj(r[1]));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 56: /* char-hex-digit? k c */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k c */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_char_2Dhex_2Ddigit_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 57: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(58);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_gobble_2Dline;

case 58: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  port k */
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_sub_2Dread;

case 59: /* gobble-line k port */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_gobble_2Dline: /* k port */
    /* r[0] */    
    /* r[1] */    
    goto s_loop_v2541;

s_loop_v2541: /* k port */
    { int c = fgetc(iportdata((r[1])));
    r[2] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[2]) == char_from_obj(cx__2Alinefeed_2A))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    /* r[0] */    
    /* r[1] */    
    goto s_loop_v2541;
  }
  }

case 60: /* set-sharp-macro! k c proc */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_set_2Dsharp_2Dmacro_21: /* k c proc */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (cx__2Asharp_2Dmacros_2A);
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__2Asharp_2Dmacros_2A = (hendblk(3)); }
    r[3] = obj_from_void(0);
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 61: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { FILE *p = iportdata((r[2])); int c = fgetc(p); ungetc(c, p);
    r[3] = (c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(62);
    r[4] = (hendblk(2+1));
  if ((iseof((r[3])))) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23431);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  } else {
    r[5+0] = obj_from_ktrap();
    r[5+1] = obj_from_char(tolower(char_from_obj(r[3])));
    r[5+2] = r[2];  
    r[5+3] = r[0];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v2520;
  }

case 62: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v2520: /* ek r port k */
    { /* assq */
    obj x = (r[1]), l = (cx__2Asharp_2Dmacros_2A), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[4] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[4])) {
    r[5+0] = (cdr((r[4])));
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[3];  
    r[5+2] = r[1];  
    r[5+3] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = r[3];  
    r[6+1] = (cx__23441);
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  }

case 63: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(64);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(121);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(116);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 64: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(65);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(119);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(39);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 65: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(66);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(117);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(96);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 66: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(67);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(114);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(44);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 67: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(68);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(112);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(38);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 68: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(69);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(108);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(124);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 69: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(70);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(106);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(59);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 70: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (cx__2Adelete_2A);
    *--hp = (mksymbol(internsym("delete")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (cx__2Aescape_2A);
    *--hp = (mksymbol(internsym("esc")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (cx__2Apage_2A);
    *--hp = (mksymbol(internsym("page")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (cx__2Avtab_2A);
    *--hp = (mksymbol(internsym("vtab")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (cx__2Alinefeed_2A);
    *--hp = (mksymbol(internsym("linefeed")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (cx__2Alinefeed_2A);
    *--hp = (mksymbol(internsym("newline")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (cx__2Atab_2A);
    *--hp = (mksymbol(internsym("tab")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (cx__2Abackspace_2A);
    *--hp = (mksymbol(internsym("backspace")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (cx__2Aalarm_2A);
    *--hp = (mksymbol(internsym("alarm")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (cx__2Anul_2A);
    *--hp = (mksymbol(internsym("nul")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = obj_from_char(32);
    *--hp = (mksymbol(internsym("space")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__2Achar_2Dname_2Dtable_2A = (hendblk(3)); }
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(71);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(103);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(92);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 71: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    { static obj c[] = { obj_from_case(72) }; cx_char_2Dscalar_2Dvalue_2Dliteral_2Ddelimiter_3F = (obj)c; }
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(73);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(101);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = obj_from_char(40);
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;

case 72: /* char-scalar-value-literal-delimiter? k c */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k c */
  if ((iseof((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(iseof((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    /* r[0] */    
    /* r[1] */    
    goto gs_delimiter_3F;
  }

case 73: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(79);
    r[2] = (hendblk(0+1));
    r[0] = r[2];  
    r[1] = (cx__23641);
    goto s_loop_v2426;

case 74: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop_v2426: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(74);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(75);
    r[2] = (hendblk(3+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(76);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = (car((r[1])));
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;
  }

case 75: /* clo ek  */
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

case 76: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    r[3] = (c == EOF ? mkeof() : obj_from_char(c)); }
    r[4+0] = r[0];  
    r[4+1] = obj_from_fixnum(0);
    r[4+2] = r[3];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v2429;

s_loop_v2429: /* k n c port */
    { const char_t v2952_c = char_from_obj(r[2]);
  if ((isdigit((v2952_c)))) {
    { int c = fgetc(iportdata((r[3])));
    r[4] = (c == EOF ? mkeof() : obj_from_char(c)); }
    /* r[0] */    
    r[1] = obj_from_fixnum((fixnum_from_obj(r[1]) * (10)) + (((fixnum_t)(v2952_c)) - ((fixnum_t)(48))));
    r[2] = r[4];  
    /* r[3] */    
    goto s_loop_v2429;
  } else {
  if (((v2952_c) == (39))) {
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(77);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[3];  
    goto gs_sub_2Dread_2Dcarefully;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__23674);
    r[2] = r[4];  
    goto gs_r_2Derror_2A;
  }
  } } 

case 77: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r n k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(78);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    r[2] = (mknull());
    goto s_loop_v2438;

s_loop_v2438: /* k i l */
    { const fixnum_t v2951_i = fixnum_from_obj(r[1]);
  if (((v2951_i) == 0)) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    /* r[0] */    
    r[1] = obj_from_fixnum((v2951_i) - (1));
    r[2] = r[3];  
    goto s_loop_v2438;
  } } 

case 78: /* clo ek r */
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
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("function")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 79: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    { static obj c[] = { obj_from_case(80) }; cx_proper_2Dlist_3F = (obj)c; }
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(87);
    r[2] = (hendblk(0+1));
    r[0] = r[2];  
    r[1] = (cx__23723);
    goto s_loop_v2379;

case 80: /* proper-list? k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k x */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_proper_2Dlist_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 81: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop_v2379: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(81);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(82);
    r[2] = (hendblk(3+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(83);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = (car((r[1])));
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_set_2Dsharp_2Dmacro_21;
  }

case 82: /* clo ek  */
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

case 83: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(84);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = obj_from_char(35);
    /* r[2] */    
    goto gs_sub_2Dread_2Dtoken;

case 84: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(85);
    r[4] = (hendblk(3+1));
    r[5+0] = (cx_string_2D_3Efixnum);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r[5+3] = obj_from_fixnum(10);
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 85: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r port k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(86);
    r[5] = (hendblk(3+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_l_v2387;
  } else {
    r[0] = (cx_string_2D_3Eflonum);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 86: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v2387: /* ek r r port k */
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
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23721);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  }

case 87: /* clo ek  */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek  */
    { static obj c[] = { obj_from_case(88) }; cx_sub_2Dread_2Dtoken = (obj)c; }
    { static obj c[] = { obj_from_case(92) }; cx_parse_2Dtoken = (obj)c; }
    cx_strange_2Dsymbol_2Dnames = (cx__23826);
    { static obj c[] = { obj_from_case(95) }; cx_delimiter_3F = (obj)c; }
    { static obj c[] = { obj_from_case(97) }; cx_char_2Dunicode_2Dwhitespace_3F = (obj)c; }
    { static obj c[] = { obj_from_case(98) }; cx_binary_2Dsearch = (obj)c; }
    { static obj c[] = { obj_from_case(99) }; cx_reverse_2Dlist_2D_3Estring = (obj)c; }
    { static obj c[] = { obj_from_case(100) }; cx_make_2Dimmutable_21 = (obj)c; }
    r[2] = obj_from_void(0);
    r[3+0] = r[0];
    pc = 0; /* exit from module init */
    r[3+1] = r[2];  
    r += 3; /* shift reg wnd */
    assert(rc = 2);
    goto jump;

case 88: /* sub-read-token k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_sub_2Dread_2Dtoken: /* k c port */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[0];  
    r[4+1] = r[3];  
    r[4+2] = obj_from_fixnum(1);
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v2350;

case 89: /* clo k l n */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v2350: /* k l n port */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(89);
    r[4] = (hendblk(1+1));
    { FILE *p = iportdata((r[3])); int c = fgetc(p); ungetc(c, p);
    r[5] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[5])))) {
    { fixnum_t v2949_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2949_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v2949_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[1];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[6] = (o); } }
    { fixnum_t v2950_tmp;
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2950_tmp = (n); }
    { /* list->string */
    int i, c = (v2950_tmp); 
    obj o = hpushstr(7, allocstring(c, ' ')); /* 7 live regs */
    obj l = r[6];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[6] = (o); } }
    r[7+0] = r[0];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((char_from_obj(r[5]) == (92))) {
    { int c = fgetc(iportdata((r[3])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    { FILE *p = iportdata((r[3])); int c = fgetc(p); ungetc(c, p);
    r[6] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if (((iseof((r[6]))) || (!((120) == char_from_obj(r[6]))))) {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__23773);
    r[2] = r[7];  
    goto gs_r_2Derror_2A;
  } else {
    { int c = fgetc(iportdata((r[3])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(90);
    r[7] = (hendblk(5+1));
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(91);
    r[8] = (hendblk(0+1));
    r[0] = r[7];  
    r[1] = r[3];  
    r[2] = r[8];  
    r[3] = (cx__23763);
    goto gs_decode_2Dhex_2Ddigits;
  }
  } else {
    r[6] = obj_from_fixnum((fixnum_t)char_from_obj(r[5]));
  if (bool_from_obj((fixnum_from_obj(r[6]) < (128)) ? (vectorref((cx_read_2Dterminating_3F_2Dvector), fixnum_from_obj(r[6]))) : (cxs_binary_2Dsearch((cx__2Anon_2Dsymbol_2Dconstituents_2Dabove_2D127_2A), (r[6]))))) {
    { fixnum_t v2947_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2947_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v2947_tmp);
    hreserve(hbsz(3)*c, 7); /* 7 live regs */
    l = r[1];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[7] = (o); } }
    { fixnum_t v2948_tmp;
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2948_tmp = (n); }
    { /* list->string */
    int i, c = (v2948_tmp); 
    obj o = hpushstr(8, allocstring(c, ' ')); /* 8 live regs */
    obj l = r[7];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[7] = (o); } }
    r[8+0] = r[0];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = obj_from_ktrap();
    r[8+2] = r[7];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { int c = fgetc(iportdata((r[3])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[7];  
    r[2] = obj_from_fixnum(fixnum_from_obj(r[2]) + (1));
    /* r[3] */    
    goto s_loop_v2350;
  }
  }
  }

case 90: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop n l k port */
    { int c = fgetc(iportdata((r[6])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8+0] = r[2];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[5];  
    r[8+2] = r[7];  
    r[8+3] = obj_from_fixnum(fixnum_from_obj(r[3]) + (1));
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 91: /* clo k c */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k c */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isequal((r[1]), obj_from_char(59)));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 92: /* parse-token k string port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_parse_2Dtoken: /* k string port */
    { const char_t v2946_c = (*stringref((r[1]), (0)));
    r[3] = obj_from_bool((isdigit((v2946_c))) || (((v2946_c) == (43)) || (((v2946_c) == (45)) || ((v2946_c) == (46))))); } 
  if (bool_from_obj(r[3])) {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(93);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_string_2D_3Efixnum);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = obj_from_fixnum(10);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[3] = (cxs_make_2Dimmutable_21((r[1])));
    r[3] = (mksymbol(internsym(stringchars((r[3])))));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
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
    /* ek r port string k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(94);
    r[5] = (hendblk(3+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_l_v2333;
  } else {
    r[0] = (cx_string_2D_3Eflonum);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
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
s_l_v2333: /* ek r port string k */
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
  if (bool_from_obj(ismember((r[3]), (cx_strange_2Dsymbol_2Dnames)))) {
    r[5] = (cxs_make_2Dimmutable_21((r[3])));
    r[5] = (mksymbol(internsym(stringchars((r[5])))));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((strcmp(stringchars((r[3])), stringchars((cx__23806))) == 0)) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx_dot);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((stringlen((r[3]))) >= (2))) {
    { const char_t v2945_tmp = (*stringref((r[3]), (0)));
    r[5] = obj_from_bool((v2945_tmp) == (45)); } 
  if (bool_from_obj(r[5])) {
    { const char_t v2944_tmp = (*stringref((r[3]), (1)));
    r[5] = obj_from_bool((v2944_tmp) == (62)); } 
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (cxs_make_2Dimmutable_21((r[3])));
    r[5] = (mksymbol(internsym(stringchars((r[5])))));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23721);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  }
  }
  }
  }

case 95: /* delimiter? k c */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_delimiter_3F: /* k c */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(96);
    r[2] = (hendblk(2+1));
    r[0] = r[2];  
    /* r[1] */    
    goto gs_char_2Dunicode_2Dwhitespace_3F;

case 96: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k c */
    { const char_t v2943_c = char_from_obj(r[3]);
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = (bool_from_obj(r[1]) ? (r[1]) : obj_from_bool(((v2943_c) == (41)) || (((v2943_c) == (40)) || (((v2943_c) == (93)) || (((v2943_c) == (91)) || (((v2943_c) == (34)) || ((v2943_c) == (59))))))));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump; } 

case 97: /* char-unicode-whitespace? k c */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_char_2Dunicode_2Dwhitespace_3F: /* k c */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_binary_2Dsearch((cx__2Awhitespaces_2A), obj_from_fixnum((fixnum_t)char_from_obj(r[1]))));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 98: /* binary-search k vec val */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k vec val */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_binary_2Dsearch((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 99: /* reverse-list->string k l n */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k l n */
    { fixnum_t v2941_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2941_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v2941_tmp);
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    l = r[1];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[3] = (o); } }
    { fixnum_t v2942_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2942_tmp = (n); }
    { /* list->string */
    int i, c = (v2942_tmp); 
    obj o = hpushstr(4, allocstring(c, ' ')); /* 4 live regs */
    obj l = r[3];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[3] = (o); } }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 100: /* make-immutable! k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k x */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_make_2Dimmutable_21((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 101: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(102);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    /* r[1] */    
    /* r[2] */    
    goto gs_sub_2Dread_2Dlist_2Dparen;

case 102: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k */
  if (bool_from_obj(cxs_proper_2Dlist_3F((r[1])))) {
    { fixnum_t v2940_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2940_tmp = (n); }
    { /* list->vector */
    obj l; int i, c = (v2940_tmp);
    hreserve(hbsz(c+1), 4); /* 4 live regs */
    l = r[1];   /* gc-safe */
    for (i = 0; i < c; ++i, l = cdr(l)) hp[i-c] = car(l);
    hp -= c; *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(c+1)); } }
    r[5+0] = r[3];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[3];  
    r[1] = (cx__23636);
    r[2] = r[4];  
    goto gs_r_2Derror_2A;
  }

case 103: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    { FILE *p = iportdata((r[2])); int c = fgetc(p); ungetc(c, p);
    r[3] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[3])))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__23625);
    r[2] = r[4];  
    goto gs_r_2Derror_2A;
  } else {
  if (((120) == char_from_obj(r[3]))) {
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(104);
    r[4] = (hendblk(3+1));
    { FILE *p = iportdata((r[2])); int c = fgetc(p); ungetc(c, p);
    r[5] = (c == EOF ? mkeof() : obj_from_char(c)); }
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_delimiter_3F;
  } else {
  if ((isalpha(char_from_obj(r[3])))) {
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(105);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;
  } else {
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    r[4] = r[3];  
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }

case 104: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k c */
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[3];  
    r[1] = r[2];  
    r[2] = (cx_char_2Dscalar_2Dvalue_2Dliteral_2Ddelimiter_3F);
    r[3] = (cx__23617);
    goto gs_decode_2Dhex_2Ddigits;
  }

case 105: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k c */
    { fixnum_t v2939_tmp;
    r[5] = (hpushstr(5, newstring(symbolname(getsymbol((r[1]))))));
    v2939_tmp = (stringlen((r[5])));
    r[5] = obj_from_bool((v2939_tmp) == (1)); }
  if (bool_from_obj(r[5])) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* assq */
    obj x = (r[1]), l = (cx__2Achar_2Dname_2Dtable_2A), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[5] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[5])) {
    r[0] = r[3];  
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
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = r[3];  
    r[7+1] = (cx__23612);
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  }
  }

case 106: /* clo k char port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k char port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(107);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;

case 107: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  port k */
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_sub_2Dread;

case 108: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(111);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_recur_v2213;

case 109: /* clo k */
    assert(rc == 2);
    { obj* p = objptr_from_obj(r[0]);
    r[1+1] = p[1]; }
    r += 1; /* shift reg. wnd */
s_recur_v2213: /* k port */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = obj_from_case(109);
    r[2] = (hendblk(1+1));
    { int c = fgetc(iportdata((r[1])));
    r[3] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[3])))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[0];  
    r[5+1] = (cx__23520);
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  } else {
  if ((char_from_obj(r[3]) == (124))) {
    { FILE *p = iportdata((r[1])); int c = fgetc(p); ungetc(c, p);
    r[4] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[4])))) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = r[0];  
    r[6+1] = (cx__23520);
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  } else {
  if ((char_from_obj(r[4]) == (35))) {
    { int c = fgetc(iportdata((r[1])));
    r[5] = (c == EOF ? mkeof() : obj_from_char(c)); }
    r[6+0] = r[0];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    /* r[0] */    
    /* r[1] */    
    goto s_recur_v2213;
  }
  }
  } else {
  if ((char_from_obj(r[3]) == (35))) {
    { FILE *p = iportdata((r[1])); int c = fgetc(p); ungetc(c, p);
    r[4] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[4])))) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = r[0];  
    r[6+1] = (cx__23520);
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  } else {
  if ((char_from_obj(r[4]) == (124))) {
    { int c = fgetc(iportdata((r[1])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(110);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    /* r[1] */    
    goto s_recur_v2213;
  } else {
    /* r[0] */    
    /* r[1] */    
    goto s_recur_v2213;
  }
  }
  } else {
    /* r[0] */    
    /* r[1] */    
    goto s_recur_v2213;
  }
  }
  }

case 110: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  recur k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 111: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  port k */
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_sub_2Dread;

case 112: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    { FILE *p = iportdata((r[2])); int c = fgetc(p); ungetc(c, p);
    r[3] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if ((iseof((r[3])))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__23506);
    r[2] = r[4];  
    goto gs_r_2Derror_2A;
  } else {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(113);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;
  }

case 113: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* box */ 
    hreserve(hbsz(2), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = obj_from_size(BOX_BTAG); 
    r[3] = (hendblk(2)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 114: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    { FILE *p = iportdata((r[2])); int c = fgetc(p); ungetc(c, p);
    r[3] = (c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(115);
    r[4] = (hendblk(2+1));
  if ((iseof((r[3])))) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23255);
    r[2] = r[5];  
    goto gs_r_2Derror_2A;
  } else {
  if ((char_from_obj(r[3]) == (64))) {
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    r[5] = (mksymbol(internsym("unsyntax-splicing")));
  } else {
    r[5] = (mksymbol(internsym("unsyntax")));
  }
    r[6+0] = obj_from_ktrap();
    r[6+1] = r[5];  
    r[6+2] = r[2];  
    r[6+3] = r[0];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v2170;
  }

case 115: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v2170: /* ek r port k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(116);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;

case 116: /* clo ek r */
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

case 117: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(118);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;

case 118: /* clo ek r */
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
    *--hp = (mksymbol(internsym("quasisyntax")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 119: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(120);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;

case 120: /* clo ek r */
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
    *--hp = (mksymbol(internsym("syntax")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 121: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    r[3] = obj_from_bool(1);
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 122: /* clo k c port */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c port */
    { int c = fgetc(iportdata((r[2])));
    (void)(c == EOF ? mkeof() : obj_from_char(c)); }
    { FILE *p = iportdata((r[2])); int c = fgetc(p); ungetc(c, p);
    r[3] = (c == EOF ? mkeof() : obj_from_char(c)); }
  if (((!(iseof((r[3])))) && (isalpha(char_from_obj(r[3]))))) {
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(123);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 123: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r port k */
  if (((r[1]) == (mksymbol(internsym("load"))))) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(124);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_sub_2Dread_2Dcarefully;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[3];  
    r[5+1] = (cx__23452);
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_r_2Derror_2A;
  }

case 124: /* clo ek r */
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
    *--hp = (mksymbol(internsym("load")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
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
