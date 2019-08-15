# Compatibility Libraries
                         
This directory contains #F libraries for some subsets of Scheme. Each library is implemented as a single .sf file and can be compiled and linked in the regular manner, e.g.

```
$ sfc libs.sf myprog.sf     # sfc produces 2 C files
$ gcc libs.c myprog.c       # gcc produces a.out
```
Please note that to dress an exisiting Scheme source file as a #F program that
uses a library like LibS, one has to add `(load "libs.sf")` line to the beginning of the
file and `(define (main argv) #f)` to the end.

## LibS (Small) Library

LibS (see [libs.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libs.sf)) generally targets
R^5RS feature set. It has the following known limitations:

  *  SFC reader used to read #F source code is case-sensitive
  *  `read` and `string->symbol` are also case-sensitive
  *  there is no support for `eval` and environment functions
  *  no dynamic `load` or dymamic macroexpansion/compilation
  *  fixnums are limited to 24 bits, flonums are doubles
  *  no support for bignums/rational/complex numbers
  *  `max` and `min` do not preserve inexactness
  *  `dynamic-wind` has pre-R^6RS semantics
  *  `set!` to built-in bindings is not allowed
  *  there is no REPL and no transcript functions

In addition to R^5RS-level functionality, LibS supports some popular extensions
defined in pre-R^5RS Scheme standards, SRFIs, and R^6RS/R^7RS libraries:

  *  many fixnum (`fx`) and flonum (`fl`) - specific operations
  *  `letrec*`, `rec`, `receive`, `let-values`, `let*-values`, `case-lambda` forms
  *  `reverse!`, `for-all`, `exists`, `fold-left`, `fold-right`
  *  `filter`, `partition`, `remq`, `remv`, `remove`, `remp`, `memp`, `assp`
  *  `list-sort!`, `list-sort`, `list-merge`, `vector-sort!`
  *  operations on boxes: `box?`, `box`, `unbox`, `set-box!`
  *  `error`, `assertion-violation` (not based on exceptions)
  *  `file-exists?`, `delete-file`, `rename-file`, `open-input-string`
  *  `exit`, `abort`, `reset`, `command-line`
  *  `get-environment-variable`, `system`, `current-jiffy`, `jiffies-per-second` 
