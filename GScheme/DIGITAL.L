(define (half-adder a b s c)
  (let ((d (make-wire)) (e (make-wire)))
    (or-gate a b d)
    (and-gate a b c)
    (inverter c e)
    (and-gate d e s)))

(define (full-adder a b c-in sum c-out)
  (let ((s (make-wire))
        (c1 (make-wire))
        (c2 (make-wire)))
    (half-adder b c-in s c1)
    (half-adder a s sum c2)
    (or-gate c1 c2 c-out)))

(define (inverter input output)
  (define (invert-input)
    (let ((new-value (logical-not (get-signal input))))
      (after-delay inverter-delay
                   (lambda ()
                     (set-signal! output
                                  new-value)))))
  (add-action! input invert-input))

(define (logical-not s)
  (cond ((= s 0) 1)
        ((= s 1) 0)
        (else (error "Invalid signal" s))))

(define (and-gate a1 a2 output)
  (define (and-action-procedure)
    (let ((new-value
           (logical-and (get-signal a1) (get-signal a2))))
      (after-delay and-gate-delay
                   (lambda ()
                     (set-signal! output new-value)))))
  (add-action! a1 and-action-procedure)
  (add-action! a2 and-action-procedure))

(define (logical-and a b)
  (cond ((or (= a 0) (= b 0)) 0)
        ((and (= a 1) (= b 1)) 1)
        (else (error "Invalid signal" s))))

(define (or-gate a1 a2 output)
  (define (or-action-procedure)
    (let ((new-value
           (logical-or (get-signal a1) (get-signal a2))))
      (after-delay or-gate-delay
                   (lambda ()
                     (set-signal! output new-value)))))
  (add-action! a1 or-action-procedure)
  (add-action! a2 or-action-procedure))

(define (logical-or a b)
  (cond ((and (= a 0) (= b 0)) 0)
        ((or (= a 1) (= b 1)) 1)
        (else (error "Invalid signal" s))))





(define (make-wire)
  (let ((signal-value 0) (action-procedures '()))

    (define (set-my-signal! new-value)
      (if (not (= signal-value new-value))
          (sequence (set! signal-value new-value)
                    (call-each action-procedures))
          'done))
    (define (accept-action-procedure proc)
      (set! action-procedures (cons proc action-procedures))
      (proc))

    (define (dispatch m)
      (cond ((eq? m 'get-signal) signal-value)
            ((eq? m 'set-signal!) set-my-signal!)
            ((eq? m 'add-action!) accept-action-procedure)
            (else (error "Unknown operation -- WIRE" m))))

    dispatch))

(define (call-each procedures)
  (if (null? procedures)
      'done
      (sequence
       ((car procedures))
       (call-each (cdr procedures)))))

(define (get-signal wire)
  (wire 'get-signal))

(define (set-signal! wire new-value)
  ((wire 'set-signal!) new-value))

(define (add-action! wire action-procedure)
  ((wire 'add-action!) action-procedure))

(define (after-delay delay action)
  (add-to-agenda! (+ delay (current-time the-agenda))
                  action
                  the-agenda))

(define (propagate)
  (if (empty-agenda? the-agenda)
      'done
      (let ((first-item (first-agenda-item the-agenda)))
        (first-item)
        (remove-first-agenda-item! the-agenda)
        (propagate))))

(define (probe name wire)
  (add-action! wire
               (lambda ()
                 (print name)
                 (princ (current-time the-agenda))
                 (princ "  New Value = ")
                 (princ (get-signal wire)))))

(define (make-time-segment time queue)
  (cons time queue))
(define (segment-time s) (car s))
(define (segment-queue s) (cdr s))

(define (make-agenda)
  (list '*agenda*
        (make-time-segment 0 (make-queue))))
(define (segments agenda)
  (cdr agenda))
(define (first-segment agenda) (car (segments agenda)))
(define (rest-segments agenda) (cdr (segments agenda)))
(define (set-segments! agenda segments)
  (set-cdr! agenda segments))
(define (current-time agenda)
  (segment-time (first-segment agenda)))

(define (empty-agenda? agenda)
  (and (empty-queue? (segment-queue (first-segment agenda)))
       (null? (rest-segments agenda))))

(define (add-to-agenda! time action agenda)
  (define (add-to-segments! segments)
    (if (= (segment-time (car segments)) time)
        (insert-queue! (segment-queue (car segments))
                       action)
        (let ((rest (cdr segments)))
          (cond ((null? rest)
                 (insert-new-time! time action segments))
                ((> (segment-time (car rest)) time)
                 (insert-new-time! time action segments))
                (else (add-to-segments! rest))))))
  (add-to-segments! (segments agenda)))

(define (insert-new-time! time action segments)
  (let ((q (make-queue)))
    (insert-queue! q action)
    (set-cdr! segments
              (cons (make-time-segment time q)
                    (cdr segments)))))

(define (remove-first-agenda-item! agenda)
  (delete-queue! (segment-queue (first-segment agenda))))

(define (first-agenda-item agenda)
  (let ((q (segment-queue (first-segment agenda))))
    (if (empty-queue? q)
        (sequence (set-segments! agenda
                                 (rest-segments agenda))
                  (first-agenda-item agenda))
        (front q))))
