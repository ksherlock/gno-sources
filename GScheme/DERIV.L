;; This is some code that does symbolic differentiation of a list.
;; format is (deriv <exprn> <deriv-by>)  where <exprn> is a SCHEME-style
;; expression to be differentiated and <deriv-by> is a symbol of which variable
;; to derivate by.
;; for instance, if you do a (deriv '(^ x 3) 'x) you'll get
;; (* 3 (^ x 2)).  This does some pretty nice simplifications.

;; this is the routine you call
(Define (deriv exp var)
  (cond ((constant? exp) 0)    ;; a constant? if so, d/dx is zero
        ((variable? exp)       ;; if a var, and the same one (dx/dx)
         (if (same-variable? exp var) 1 0))
        ((sum? exp)
         (make-sum (deriv (addend exp) var)
                   (deriv (augend exp) var)))
        ((exponentiation? exp)
         (make-product (exponent exp)
                       (make-product
                         (make-exponent (base exp)
                                        (- (exponent exp) 1))
                         (deriv (base exp) var))))
        ((product? exp)
         (make-sum
           (make-product (multiplier exp)
                         (deriv (multiplicand exp) var))
           (make-product (deriv (multiplier exp) var)
                         (multiplicand exp))))))

(define (constant? x) (number? x)) ;; check an exprn for constantness
(define (variable? x) (symbol? x)) ;; do the same for variableness
(define (same-variable? v1 v2)
  (and (variable? v1)
       (and (variable? v2)
            (eq? v1 v2))))

;; construct a sum expression
(define (make-sum a1 a2)
  (cond ((and (number? a1) (number? a2)) (+ a1 a2))
        ((number? a1) (if (= a1 0) a2 (list '+ a1 a2)))
        ((number? a2) (if (= a2 0) a1 (list '+ a1 a2)))
        (else (list '+ a1 a2))))

;; construct a product expression. pretty complex cause it checks
;; for nested multiplications (such as (* 8 (* x 3)), which is re-works
;; as (* 24 x)

(define (make-product m1 m2)
  (cond ((and (number? m1) (number? m2)) (* m1 m2))
        ((number? m1)
         (cond ((= m1 0) 0)
               ((= m1 1) m2)
               ((product? m2)
                (let ((mm1 (multiplicand m2))
                      (mm2 (multiplier m2)))
                  (cond ((number? mm1)
                         (list '* (* m1 mm1) mm2))
                        ((number? mm2)
                         (list '* (* m1 mm2) mm1)))))
               (else (list '* m1 m2))))
        ((number? m2)
         (cond ((= m2 0) 0)
               ((= m2 1) m1)
               ((product? m1)
                (let ((mm1 (multiplicand m1))
                      (mm2 (multiplier m1)))
                  (cond ((number? mm1)
                         (list '* (* m2 mm1) mm2))
                        ((number? mm2)
                         (list '* (* m2 mm2) mm1)))))
               (else (list '* m1 m2))))
        (else (list '* m1 m2))))

;; construct a power-to expression. simple (nothing to it!)

(define (make-exponent base exp)
  (cond ((and (number? base) (number? exp)) (^ base exp))
        ((and (number? exp) (= exp 1)) base)
        (else (list '^ base exp))))

(define (sum? x)  ;; see if an exprn is an addition expression
  (if (not (atom? x)) (eq? (car x) '+) nil))
(define (addend s) (cadr s))    ;; pick out the various parts of a
(define (augend s) (caddr s))   ;; addition expression

(define (product? x)  ;;see if an exprn is a product
  (if (not (atom? x)) (eq? (car x) '*) nil))
(define (multiplier p) (cadr p))     ;; grab the pieces of a multiplication
(define (multiplicand p) (caddr p))  ;;

(define (exponentiation? x);; see if an expression is a ^ type
  (if (not (atom? x)) (eq? (car x) '^) nil))
(define (base x) (cadr x))
(define (exponent x) (caddr x))  ;
