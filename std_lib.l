(define (inc n) (+ n 1))

(define (dec n) (- n 1))

(define (fib-iter n a b)
  (if (zero? n)
      a
      (fib-iter (dec n) b (+ a b))))

(define (fib n)
  (fib-iter n 0 1))

(define nil '())

(define (caar x) (car (car x)))
(define (cadr x) (car (cdr x)))
(define (cddr x) (cdr (cdr x)))

(define (caaar x) (car (car (car x))))
(define (caadr x) (car (car (cdr x))))
(define (cadar x) (car (cdr (car x))))
(define (cdaar x) (cdr (car (car x))))
(define (caddr x) (car (cdr (cdr x))))
(define (cdadr x) (cdr (car (cdr x))))
(define (cddar x) (cdr (cdr (car x))))
(define (cdddr x) (cdr (cdr (cdr x))))

(define (length x)
  (if (null? x)
      0
      (+ 1 (length (cdr x)))))

(define (list-tail list k)
  (if (zero? k)
      list
      (list-tail (cdr list) (dec k))))

(define (list-ref list k)
  (if (zero? k)
      (car list)
      (list-ref (cdr list) (dec k))))

(define (reduce op coll init)
  (if (null? coll)
      n
      (op (car coll)
          (reduce op
                  (cdr coll)
                  n))))

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

(define (filter pred xs)
  (if (null? xs)
      '()
      (if (pred (car xs))
          (cons (car xs) (filter pred (cdr xs)))
          (filter pred (cdr xs)))))
