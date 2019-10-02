(define-library (scheme time)
  (import (only (sharpf base)
   current-jiffy current-second jiffies-per-second))
  (export
   current-jiffy current-second jiffies-per-second))
  
