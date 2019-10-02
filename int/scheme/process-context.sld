(define-library (scheme process-context)
  (import (only (sharpf base)
   command-line emergency-exit
   exit get-environment-variable 
   get-environment-variables))
  (export
   command-line emergency-exit
   exit get-environment-variable 
   get-environment-variables))
  
