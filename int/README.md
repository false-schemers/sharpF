# Interpreters for Compatibility Libraries
                         
This directory contains interpreters for #F compatibility libraries. Each interpreter is implemented as a single .sf file and needs to be compiled and linked with the corresponding library, e.g.

```
$ sfc libs.sf ints.sf     # sfc produces 2 C files
$ gcc -lm libs.c ints.c   # gcc produces a.out (libs refers to math functions, so -lm may be needed)
```

## IntS, an Interpreter for LibS (Small) Library

IntS (see [ints.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/int/ints.sf)) allows interactive calling of LibS functions and use of LibS syntax forms, including `define-syntax` and `syntax-rules`. It provides full argument checking (LibS' own argument checking is limited to C asserts in debug mode).

There are some restrictions on the functionality available in IntS, compared to the #F code compiled with LibS:

  *  `read` uses LibS reader, limited to reading data types implemented in LibS
  *  fixnum (`fx`) and flonum (`fl`) - specific operations are not available
  *  C primitives and C code cannot be used
  *  `main` can be defined, but is not called by the interpreter

Since in IntS `load` is no longer a special form for separate compilation, it cannot be used to refer
to arbitrary #F files - only to files that can be dynamically loaded and executed by the interpreter. If you want to 'mask' your compile-time `load` forms from the interpreter, you may use the `#fload "myfile.sf"` abbreviated form, which the interpreter ignores.

Here is the list of IntS additions and things that behave differently between IntS and LibS:

  *  single-argument `eval` is available (macroexpands, compiles, and executes the argument)
  *  single-argument `expand` is available (macroexpands the argument)
  *  `load` is a procedure that dynamically loads and interprets Scheme code via `eval` 
  *  command-line file arguments are dynamically loaded 
  *  there is a traditional REPL (read-eval-print loop)
