(define-library (scheme r5rs-null)
  (import (scheme r5rs))
  (export
    syntax-rules else ... => 
    and begin case cond 
    define define-syntax
    delay do if lambda 
    let let* let-syntax letrec letrec-syntax
    or quote
    quasiquote unquote unquote-splicing))
