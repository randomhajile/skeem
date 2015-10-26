;; not a good or, but will do for now
(define (or x y) (if x #t (if y #t #f)))

(define (inc n) (+ n 1))

(define (dec n) (- n 1))

(define (fib-iter n a b)
  (if (= n 0)
      a
      (fib-iter (- n 1) b (+ a b))))

(define (fib n)
  (fib-iter n 0 1))

(define nil '())

(define (caar x)
  (car (car x)))

(define (cadr x)
  (car (cdr x)))

(define (cddr x)
  (cdr (cdr x)))

(define (caaar x)
  (car (car (car x))))

(define (caadr x)
  (car (car (cdr x))))

(define (cadar x)
  (car (cdr (car x))))

(define (cdaar x)
  (cdr (car (car x))))

(define (caddr x)
  (car (cdr (cdr x))))

(define (cdadr x)
  (cdr (car (cdr x))))

(define (cddar x)
  (cdr (cdr (car x))))

(define (cdddr x)
  (cdr (cdr (cdr x))))

(define (length x)
  (if (null? x)
      0
      (+ 1 (length (cdr x)))))

(define (reverse x)
  (if (null? x)
      '()
      (append (reverse (cdr x)) (list (car x)))))

(define (list-tail list k)
  (if (zero? k)
      list
      (list-tail (cdr list) (- k 1))))

(define (list-ref list k)
  (if (= k 0)
      (car list)
      (list-ref (cdr list) (- k 1))))

(define (reduce op coll n)
  (if (null? coll)
      n
      (op (car coll)
          (reduce op (cdr coll) n))))

(define (map proc list &rest rest)
  (define (map1 func coll)
    (if (null? coll)
        '()
        (cons (func (car coll))
              (map1 func (cdr coll)))))
  (define (some-null? coll)
    (if (pair? coll)
        (or (null? (car coll))
            (some-null? (cdr coll)))
        #f))
  (let ((lists (cons list rest)))
    (if (some-null? lists)
        '()
        (cons (apply proc (map1 car lists))
              (apply map (cons proc (map1 cdr lists)))))))

(define (reduce func xs init)
  (if (null? xs)
      init
      (reduce func (cdr xs) (func (car xs) init))))

(define (filter pred xs)
  (if (null? xs)
      '()
      (if (pred (car xs))
          (cons (car xs) (filter pred (cdr xs)))
          (filter pred (cdr xs)))))
