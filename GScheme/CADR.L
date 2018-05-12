(define (cadr x) (car (cdr x)))

(define (caar x) (car (car x)))

(define (cdar x) (cdr (car x)))

(define (cddr x) (cdr (cdr x)))

(define (caddr x) (car (cdr (cdr x))))

(define (cadddr x) (car (cdr (cdr (cdr x)))))

(define (cdadr x) (cdr (car (cdr x))))

(define (pair? l) (not (atom? l)))

(define (memq item x)
  (cond ((null? x) '())
        ((eq? item (car x)) x)
        (else (memq item (cdr x)))))

(define (mapcar proc x)
  (if (null? x)
      '()
      (cons (proc (car x))
            (mapcar proc (cdr x)))))

(define map mapcar)

(define (assq key records)
  (cond ((null? records) nil)
        ((eq? key (caar records)) (car records))
        (else (assq key (cdr records)))))

(define (assoc key records)
  (cond ((null? records) nil)
        ((equal? key (caar records)) (car records))
        (else (assoc key (cdr records)))))

(define (^ base exp)
  (if (= exp 0)
      1
      (* base (^ base (- exp 1)))))
