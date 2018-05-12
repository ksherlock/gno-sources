(define (deriv f dx)
  (lambda (x)
    (/ (- (f (+ x dx)) (f x))
       dx)))

(define (newton f guess)
  (if (good-enough? guess f)
      guess
      (newton f (improve guess f))))

(define (improve guess f)
  (- guess (/ (f guess)
              ((deriv f 0.001) guess))))

(define (good-enough? guess f)
  (< (abs (f guess)) 0.001))
