(define-library (scheme lazy)
  (import (only (sharpf base)
   delay delay-force force make-promise promise?))
  (export
   acos asin atan cos exp finite?
   delay delay-force force make-promise promise?))
  
