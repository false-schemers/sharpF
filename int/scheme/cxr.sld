(define-library (scheme cxr)
  (import (only (sharpf base) 
   caaar caadr cadar caddr cdaar cdadr cddar cdddr
   caaaar caaadr caadar caaddr cadaar cadadr caddar cadddr
   cdaaar cdaadr cdadar cdaddr cddaar cddadr cdddar cddddr))
  (export
   caaar caadr cadar caddr cdaar cdadr cddar cdddr
   caaaar caaadr caadar caaddr cadaar cadadr caddar cadddr
   cdaaar cdaadr cdadar cdaddr cddaar cddadr cdddar cddddr))
