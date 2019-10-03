![](https://raw.githubusercontent.com/false-schemers/sharpF/master/misc/lambda-sunrise.png)

# #F - A Minimalistic Scheme System
                         
#F (Sharp-F or False) is a portable compiler/runtime for a subset of the Scheme programming language. Its main purpose is to facilitate creation of specialized code fragments which use Scheme-like computational model (garbage-collected memory, proper tail recursion, call/cc, closures, multiple return values) and can be easily integrated with regular C code in a direct way (no FFI or "scripting language bindings").

#F's portability is a consequence of its ability to compile itself into a standard portable C code. There is a single source distribution for all platforms; there are no OS- or hardware-specific parts, no compiler-specific tricks, no dependency on platform-specific building tools. There is no distributive to install, just a bunch of C files you can compile with your favorite C compiler, link the object files together and be done with it.

#F compiler (SFC) produces reasonably fast and very readable C code which can be tailored
to fit into a regular C project with minimal effort. The main distinguishing feature of
the #F system is its scalable runtime - the compiler itself has no embedded knowledge of
any standard functions or data types, with a single exception - `#f` (hence the name).
The `#f` value (represented as immediate 0) is hard-wired since SFC needs to compile
traditional conditional forms; everything else should be explicitly defined before
use. If one's code does not use lists or strings, they do not need to be implemented!

The starting point is the obligatory "Hello, World!" example: [hello.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/hello.sf)

It can be compiled and executed as follows:

```
$ sfc hello.sf             # sfc produces hello.c (273 lines, ~8K in size)
$ gcc hello.c              # gcc produces a.out (~50K in size)
$ ./a.out
Hello, World!
```
You can see the compiled C program here: [hello.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/compiled/hello.c). As it defines no data types or functions, the whole file
is basically just startup/shutdown/gc code.

Here is an example of what a bigger #F "Scheme" program might look like and what
happens when it is compiled and executed:

```
$ sfc tak.sf               # sfc produces tak.c (370 lines, ~11K in size)
$ gcc -O3 -DNDEBUG tak.c   # gcc produces a.out (~61K in size)
$ ./a.out
70000
```

The files can be seen here:
[tak.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/tak.sf)
[tak.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/compiled/tak.c)

As in the first example, since tak.sf defines a global function called "main", SFC added the #F runtime system
to the output C file. There is no need to link with anything else except for the
standard C runtime.

SFC supports C-style separate compilation for #F programs. Here's a slightly more
complicated example, with lists and 3 separate compilation units, one containing
all tak runtime/library functions, another for tak and ltak functions themselves, 
and the third one for the main test function. It is compiled and linked in the same 
direct manner:

```
$ sfc tlib.sf tfun.sf tmain.sf     # sfc produces 3 C files (~33K total size)
$ gcc tlib.c tfun.c tmain.c        # gcc produces a.out (~67K in size)
$ ./a.out
7000
7000
```

The files can be seen here:
[tlib.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/tlib.sf)
[tfun.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/tfun.sf)
[tmain.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/tmain.sf)
[tlib.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/compiled/tlib.c)
[tfun.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/compiled/tfun.c)
[tmain.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/compiled/tmain.c)

There's no need to compile all .sf files in a single run. Doing it one file
at a time is just as good. Dependent units (files) are scanned for macros during
compilation of a unit, so they should be present and in a good shape. Dependencies
are declared via `(load "foo.sf")` special form (or its syntactic shortcut `#fload "foo.sf"`), 
which serves two purposes: to tell
SFC where to look for macros and how to order initialization code. SFC turns
all globally defined Scheme identifiers into C identifiers (e.g. foo => cx_foo), 
generating "extern" specifications as needed. To do that, SFC assumes that
no set! can cross a compilation unit boundary; if this rule is violated,
there will be duplicate symbol definition warnings from the linker.

Note that cross-module function calls are slower than intra-module calls,
which can sometimes be compiled as direct jumps (tail call to a known entry 
point). On the other hand, very big output files can choke some C compilers, 
so some planning may be needed to organize the code for best performance.

## Scheme Compatibility

"Bare-bones" #F is in no way a standard-compliant Scheme implementation; it 
cuts many corners to get a much simpler system that can serve as a base
for a more ambitious dialect of Scheme-like language. Built-in support for
Scheme not only omits all functionality related to any particular data type,
it also lacks such features as apply, variable-arity functions, and dynamic
wind. All missing functionality can be defined using the facilities existing
in the core #F language.

As an example of language-building, and as an aid to porting Scheme programs,
#F distribution includes a simple Scheme portability library, LibS (see [libs.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libs.sf)).

LibS is just a regular #F source file, and it is included in the project in a regular
manner:

```
$ sfc libs.sf myprog.sf     # sfc produces 2 C files
$ gcc libs.c myprog.c -lm   # gcc produces a.out (libs refers to math functions, so -lm may be needed)
```

To dress an existing pre-R6RS Scheme source file as a #F program that
uses LibS, one has to add `(load "libs.sf")` line to the beginning of the
file and `(define (main argv) #f)` to the end. LibS generally targets
R5RS feature set, but it has the following known limitations:

  *  SFC reader used to read #F source code is case-sensitive
  *  `read` and `string->symbol` are also case-sensitive
  *  no support for `s`, `f`, `d`, `l` exponent markers and `#` digit placeholders 
  *  there is no support for `eval` and environment functions
  *  no dynamic `load` or dynamic macroexpansion/compilation
  *  fixnums are limited to 24 bits, flonums are doubles
  *  no support for bignums/rational/complex numbers
  *  `set!` to built-in bindings is not allowed
  *  there is no REPL and no transcript functions

In addition to formal limitations listed above, there are issues that
may affect the real-life performance of your converted Scheme
programs:

  *  very large list/vector literals converted to C code can choke your C compiler
  *  there is a noticeable performance penalty related to use of `apply` and functions with improper argument lists
  *  compared to #F's native `letcc`/`withcc`, there is a performance penalty for using `call/cc` because of `dynamic-wind`'s need to preserve multiple return values in a list
  *  run-time errors such as fixnum overflows trigger asserts in C code unless `NDEBUG` is defined during compilation
  *  deciphering syntax errors in the source code can be tricky

In addition to R5RS-level functionality, LibS supports many popular extensions
defined in pre-R5RS Scheme standards, SRFIs, and R7RS libraries:

  *  many fixnum (`fx`) and flonum (`fl`) - specific operations
  *  `letrec*`, `rec`, `receive`, `case-lambda` forms
  *  operations on boxes: `box?`, `box`, `unbox`, `set-box!`
  *  `error`, `assertion-violation` (not based on exceptions)
  *  `file-exists?`, `delete-file`, `rename-file`, `open-input-string`
  *  `exit`, `abort`, `reset`, `command-line`
  *  `get-environment-variable`, `system`, `current-jiffy`, `jiffies-per-second` 

Please check the `/lib` directory to see the full list of available libraries. Some of them have corresponding interpreters
to simplify development and debugging. Interpreters are located in the `/int` directory.


## Interoperability with C code


#F memory model was designed to be C-friendly. #F programs can freely 
use pointers to C objects because #F's garbage collector does not scan
pointers pointing outside its heap. For #F's wrapped C objects requiring finalization,
the collector provides support for a finalization call. Finalization
procedures are registered on per-datatype basis, e.g.:

```
(%localdef "static cxtype_t cxt_string = { \"string\", free };")
```

C code can access objects from a garbage-collected heap via #F's
global variables, which are regular C's global variables, updated
by the garbage collector when the corresponding object is moved in
memory. Since the hard-wired parts of the runtime are minimal,
each project can have the set of data types and operations best
suited for the task at hand, with all the necessary glue written
explicitly as C code fragments.
  

## Macros

 
#F uses a slightly modified version of Al Petrofsky's R5RS-compatible 
Alexpander macro system. It has many advanced features critical for what 
#F tries to accomplish, such as extending the definition of an existing
macro, local syntax definitions, syntax-binding a code to a name etc.
In addition, #F's expander features non-hygienic extensions, allowing 
on-the-fly construction of identifiers, predicates in patterns etc. 
Here are some examples:

```
(let-syntax ([foo (let-syntax ([bar (syntax-rules () [(bar x) (- x)])])
                      (syntax-rules () [(foo) (bar 2)]))])
   (foo))  => -2

((syntax-rules ()
   [(_ ([var init] ...) . body)
    ((lambda (var ...) . body) init ...)])
 ([x 1] [y 2])
 (+ x y))  => 3

(let-syntax ([q quote]) (q x))  => x

(let-syntax ([x append]) ((x x)))  => ()

(define-syntax variant-case
  (syntax-rules (else)
    [(_ (a . d) clause ...)
     (let ([var (a . d)]) (variant-case var clause ...))]
    [(_ var) (error 'variant-case "no clause matches ~s" var)]
    [(_ var (else exp1 exp2 ...)) (begin exp1 exp2 ...)]
    [(_ var (name (field ...) exp1 exp2 ...) clause ...)
     (if (#&(string->id #&(string-append #&(id->string name) "?")) var)
         (let ([field (#&(string->id #&(string-append #&(id->string name) 
                                         "->" #&(id->string field))) var)] ...)
           exp1 exp2 ...)
         (variant-case var clause ...))]))
```


## Code Generator


SFC is not a very sophisticated Scheme compiler, but it does many things
other compilers do:

```
  (define (compile-file filename)
    (write-code filename
      (code-generate
        (unbox-values
          (beta-substitute3
            (lambda-lift
              (beta-substitute2
                (cps-convert
                  (beta-substitute
                    (constant-fold 
                      (remove-assignments 
                        (fix-letrecs 
                          (parse-file filename)))))))))))))
```
 
Its code performance is not very far behind that of some more sophisticated 
Scheme-to-C compilers which support full R5RS Scheme, but are many times 
more complex and have large run-time systems, dominating any C code they 
might link to. SFC, while being rather simple and ignorant as to what data 
types its user will decide to support, does a reasonably good job at 
optimizing function calls and unboxing non-immediate objects like floating-point 
numbers. As an example, here's a piece of code generated for the expression 

``` 
    (fl- 0.0 (fl+ (fl/ Vc (fl* R C)) (fl/ Il C))
```

from RnRS's Runge-Kutta example ([rk.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/rk.sf),
[rk.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/compiled/rk.c)):

```
    { const flonum_t v2500_C = flonum_from_obj(r[3]);
    r[5] = (vectorref((r[1]), (0)));
    r[6] = (vectorref((r[1]), (1)));
    r[7] = obj_from_flonum(7, flonum_from_obj(r[5]) / flonum_from_obj(r[4]));
    { const flonum_t v2505_tmp = (0.0);
    { flonum_t v2504_tmp;
    { flonum_t v2503_tmp;
    { const flonum_t v2502_tmp = (flonum_from_obj(r[6]) / (v2500_C));
    { const flonum_t v2501_tmp = (flonum_from_obj(r[2]) * (v2500_C));
    v2503_tmp = (flonum_from_obj(r[5]) / (v2501_tmp)); } 
    v2504_tmp = ((v2503_tmp) + (v2502_tmp)); }  }
    r[8] = obj_from_flonum(8, (v2505_tmp) - (v2504_tmp)); } } 
```

Since the library provided unboxed type `flonum_t` for floating point
numbers, SFC used unboxed representation for all intermediate results,
converting them to a boxed form only when they have to be stored
in general-purpose registers or storage locations. Many compilers
can do a better job optimizing floating point operations, but SFC can
go surprisingly far with just a few simple tricks and no embedded
knowledge of supported datatypes.


## Debugging

SFC does not insert any run-time checks into a release version of the
code, so getting the car of a number or passing a wrong number of arguments 
to a function will likely lead to a coredump. If `NDEBUG` is not defined
(which may or may not be your C compiler's default), the code produced 
by SFC switches to debug versions of data access primitives, catching
data access and parameter passing violations with C run-time asserts.


## Installation

To install SFC, grab the C files from the `fixpoint` subdirectory of the master tree and compile them with your favorite C compiler.

If you are too lazy to learn GIT, the latest versions of the C source files are here:

[0.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/fixpoint/0.c) [1.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/fixpoint/1.c) [2.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/fixpoint/2.c) [3.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/fixpoint/3.c) [4.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/fixpoint/4.c) [5.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/fixpoint/5.c) [6.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/fixpoint/6.c) [7.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/fixpoint/7.c) [c.c](https://raw.githubusercontent.com/false-schemers/sharpF/master/fixpoint/c.c)

Here's how you can compile SFC on a unix box:

```
gcc -o sfc -Wall -O3 -DNDEBUG 0.c 1.c 2.c 3.c 4.c 5.c 6.c 7.c c.c
```

The rest is up to you - the compiler has no dependencies and can be run from any location.


## Documentation

The documentation is not yet complete and might be slightly out-of-date.
The .html files are located in the `doc` subdirectory of the master tree.


## Examples

You may also learn some #F basics by looking at the examples:

[hello.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/hello.sf) [tak.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/tak.sf) [stak.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/stak.sf) [rk.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/rk.sf)

They can be compiled in the following manner:

```
sfc tak.sf   # produces tak.c 
gcc -o tak -Wall -O3 -DNDEBUG tak.c
```

As a demonstration of how far you can go, here is a full-scale R5RS interpreter in a single #F file:

[siof.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/examples/siof.sf)

(please see [SIOF](https://github.com/false-schemers/siof) repository for details)


## Historical note

SFC is based on ideas from Marc Feeley's "90 minute Scheme to C compiler" presented 
at the Montreal Scheme/Lisp User Group on October 20, 2004. SFC's hygienic macroexpander is derived from 
Al Petrofsky's alexpander v1.65 (please see the 2.sf source file for the original copyright notice).
