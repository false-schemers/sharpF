
; SFC (#F Scheme to C compiler) -- esl

; Chez bootstrap code

(define construct-name 
  (lambda (template-id . args)
    (datum->syntax-object template-id
      (string->symbol
        (apply string-append
          (map (lambda (x)
                 (if (string? x)
                   x
                   (symbol->string (syntax-object->datum x))))
            args))))))

(define-syntax define-variant
  (lambda (x)
    (syntax-case x ()
      [(_ name (field0 ...))
       (with-syntax
         ([constructor (construct-name (syntax name) (syntax name))] 
          [predicate (construct-name (syntax name) (syntax name) "?")]
          [(reader ...)
           (map (lambda (field)
                  (construct-name (syntax name) (syntax name) "->" field))
             (syntax (field0 ...)))]
          [count (length (syntax (name field0 ...)))])
         (with-syntax
           ([(index ...)
             (let f ([i 1])
               (if (= i (syntax-object->datum (syntax count)))
                 '()
                 (cons i (f (1+ i)))))])
           (syntax
             (begin
               (define constructor
                 (lambda (field0 ...)
                   (vector 'name field0 ...)))
               (define predicate
                 (lambda (object)
                   (and (vector? object)
                     (= (vector-length object) count)
                     (eq? (vector-ref object 0) 'name))))
               (define reader
                 (lambda (object)
                   (vector-ref object index)))
               ...))))])))

(define-syntax variant-case
  (lambda (x)
    (syntax-case x (else)
      [(_ var) (syntax (error 'variant-case "no clause matches ~s" var))]
      [(_ var (else exp1 exp2 ...)) (syntax (begin exp1 exp2 ...))]
      [(_ exp clause ...)
       (not (identifier? (syntax exp)))
       (syntax (let ([var exp]) (_ var clause ...)))]
      [(_ var (name (field ...) exp1 exp2 ...) clause ...)
       (with-syntax
         ([predicate (construct-name (syntax name) (syntax name) "?")]
          [(reader ...)
           (map (lambda (fld)
                  (construct-name (syntax name) (syntax name) "->" fld))
             (syntax (field ...)))])
         (syntax
           (if (predicate var)
             (let ([field (reader var)] ...) exp1 exp2 ...)
             (_ var clause ...))))])))


(define-syntax define-inline 
  (syntax-rules () 
    [(_ (id . args) . body) (define (id . args) . body)] 
    [(_ id val) (define id val)]))

(define-syntax define-integrable 
  (syntax-rules () 
    [(_ (id . args) . body) (define (id . args) . body)] 
    [(_ id val) (define id val)]))
    
    
; redefine load to skip .sf dependencies when loaded into Chez

(define main-load "c.sf")
(define do-not-load '("0.sf"))
(define sub-load-once '("1.sf" "2.sf" "3.sf" "4.sf" "5.sf" "6.sf" "7.sf"))

(set! load
  (let ([scheme-load load] [loaded '()])
    (lambda (fname)
      (cond [(member fname do-not-load)
             (printf "; sfc module ~s not needed in bootstrap~%" fname)]
            [(equal? fname main-load)
             (set! loaded '())
             (printf "; loading main sfc module ~s~%" fname)
             (scheme-load fname)
             (printf "; now you may run~%;  ~s~%; to generate .c files~%"
               '(sfc "-v" "0.sf" "1.sf" "2.sf" "3.sf" "4.sf" "5.sf" "6.sf" "7.sf" "c.sf"))]
            [(member fname sub-load-once)
             (if (member fname loaded)
                 (printf "; sfc module ~s already loaded~%" fname)
                 (begin
                   (printf "; loading sfc module ~s~%" fname)
                   (set! loaded (cons fname loaded))
                   (scheme-load fname)))]             
            [else (scheme-load fname)]))))

(define (argv->list argv) argv)

(define (sfc . args)
  (main (cons "sfc-bootstrap" args)))

(load "c.sf")
