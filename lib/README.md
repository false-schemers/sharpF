# Compatibility Libraries
                         
This directory contains #F libraries for some subsets of Scheme. Each library is implemented as a single .sf file and can be compiled and linked in the regular manner, e.g.

```
$ sfc libs.sf myprog.sf     # sfc produces 2 C files
$ gcc -lm libs.c myprog.c   # gcc produces a.out (libs refers to math functions, so -lm may be needed)
```

All libraries adhere to #F's minimalistic approach to error checking: run-time errors are checked with C asserts, so programs compiled in debug mode report them and exit; there is no error checking in release mode. If you need to debug your code that uses one of these libraries, you may use the corresponding interpreter with traditional error checking (interpreters are located in the `/int` subdirectory).

Please note that to dress an exisiting Scheme source file as a #F program that
uses a library like LibS, one has to add `(load "libs.sf")` line to the beginning of the
file and `(define (main argv) #f)` to the end.


## LibXXS (Extra Extra Small) Library

LibXXS (see [libxxs.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libxxs.sf)) provides bare-bones Scheme-like functionality. It has the following known limitations:

  *  SFC reader used to read #F source code is case-sensitive
  *  library function `string->symbol` is also case-sensitive
  *  no support for `s`, `f`, `d`, `l` exponent markers and `#` digit placeholders 
  *  there is no support for `eval` and environment functions
  *  no dynamic `load` or dymamic macroexpansion/compilation
  *  no support for `read`
  *  no `dynamic-wind`, so `call/cc` is faster
  *  fixnums are limited to 24 bits, generic math is fixnum-only
  *  no support for flonums/bignums/rational/complex numbers
  *  no support for `sqrt` and trigonometry functions 
  *  no support for dotted/variable-length argument lists
  *  all variable-argument and high-order primitives like `+` and `map` are macros
  *  `apply` is a macro limited to macros `+`, `*`, `list`, `append`, `vector`, `string`, `string-append` 
  *  `set!` to built-in bindings is not allowed
  *  there is no REPL

LibXXS supports the following additional functions:

  *  many fixnum (`fx`) - specific operations
  *  `letrec*`, `rec`, `when`, `unless` forms
  *  `error` macro (not based on exceptions)
  *  `current-jiffy`, `jiffies-per-second`
  *  `exit`, `abort`, and `reset`


## LibXS (Extra Small) Library

LibXS (see [libxs.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libxs.sf)) is an extended version of LibXXS. It has the following additional functionality:

  *  support for flonums, generic math is mixed fixnum/flonum
  *  many flonum (`fl`) - specific operations
  *  `sqrt` and trigonometry functions are available in (`fl`) form (e.g. `flsqrt`)


## LibS (Small) Library

LibS (see [libs.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libs.sf)) generally targets
R5RS feature set; most of the forms and procedures behave as expected. Compared to a regular R5RS system, it has the following known limitations:

  *  SFC reader used to read #F source code is case-sensitive
  *  `read` and `string->symbol` are also case-sensitive
  *  no support for `s`, `f`, `d`, `l` exponent markers and `#` digit placeholders 
  *  there is no support for `eval` and environment functions
  *  no dynamic `load` or dynamic macroexpansion/compilation
  *  fixnums are limited to 24 bits, flonums are doubles
  *  no support for bignums/rational/complex numbers
  *  `set!` to built-in bindings is not allowed
  *  there is no REPL and no transcript functions

In addition to R5RS-level functionality, LibS supports some popular extensions
defined in pre-R5RS Scheme standards, SRFIs, and R7RS libraries:

  *  many fixnum (`fx`) and flonum (`fl`) - specific operations
  *  `letrec*`, `rec`, `receive`, `when`, `unless`, `case-lambda` forms
  *  operations on boxes: `box?`, `box`, `unbox`, `set-box!`
  *  `error`, `assertion-violation` (not based on exceptions)
  *  `file-exists?`, `delete-file`, `rename-file`, `open-input-string`
  *  `exit`, `abort`, `reset`, `command-line`
  *  `get-environment-variable`, `system`, `current-jiffy`, `jiffies-per-second` 


## LibM (Medium) Library

LibM (see [libm.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libm.sf)) is an extended version of LibS. It has the following additional functionality from R7RS-Small:

  *  support for bytevectors, with basic set of operations
  *  support for `define-record-type` records
  *  simple binary input from files and bytevectors
  *  simple binary output to files and bytevectors
  *  simple text output to/from strings
  *  `let-values`, `let*-values`, `define-values`, `make-parameter`, `parameterize`
  *  exceptions, errors, `guard` form
  *  current port access procedures are parameters
  *  fixnums are extended to 28 bits
  *  additional R7RS math and port operations
  

## LibL (Large) Library

LibL (see [libl.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libl.sf)) targets
R7RS-Small feature set; most of the forms and procedures behave as expected. Compared to a regular R7RS-Small
system, a program compiled with LibL will have the following limitations:

  *  there is no support for `eval` and environment functions
  *  no dynamic `load` or dynamic macroexpansion/compilation
  *  fixnums are limited to 24 bits, flonums are doubles
  *  no support for bignums/rational/complex numbers
  *  no support for Unicode; strings are 8-bit clean, use system locale
  *  `set!` to built-in bindings is not allowed
  *  vectors are not self-evaluating; need to be quoted as in R5RS
  *  source code literals cannot be circular (R7RS allows this)
  *  there is no support for libraries and import 
  *  there is no REPL
  
Some features of the R7RS-Small standard are not yet implemented or implemented in a simplified or non-conforming way:

  *  SFC reader and `read` procedure are always case-sensitive
  *  `#!fold-case` and `#!no-fold-case` directives are not supported
  *  `include` and `include-ci` forms are not supported
  *  `cond-expand` form is not implemented
  *  `get-environment-variable` is implemented, but `get-environment-variables` is not
  *  `current-jiffy` and `jiffies-per-second` return inexact integers
  *  `current-second` is defined as C `difftime(time(0), 0)+37`
  *  macroexpander treats `_` as a regular identifier, not match-all pattern
  *  macroexpander does not support `(... escaped)` pattern escapes
  *  macroexpander does not support patterns with internal ellipsis and improper tail variable
  
  
