(define-library (scheme char)
  (import (only (sharpf base)
   char-alphabetic? char-ci<=? char-ci<? char-ci=? char-ci>=? char-ci>?
   char-downcase char-foldcase char-lower-case? char-numeric?
   char-upcase char-upper-case? char-whitespace? digit-value
   string-ci<=? string-ci<? string-ci=? string-ci>=? string-ci>?
   string-downcase string-foldcase string-upcase))
  (export
   char-alphabetic? char-ci<=? char-ci<? char-ci=? char-ci>=? char-ci>?
   char-downcase char-foldcase char-lower-case? char-numeric?
   char-upcase char-upper-case? char-whitespace? digit-value
   string-ci<=? string-ci<? string-ci=? string-ci>=? string-ci>?
   string-downcase string-foldcase string-upcase))
  
