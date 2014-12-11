(println (format "hello world"))

(def x (ref 1))

(dosync
(ref-set x 0)
)

(println (format "hello again %d" @x))

(defn increment [i]
  (if (> i 0) 
    (
      (dosync
        (alter x inc)
        (comment println (format "in increment %d" @x))
      )
      (Thread/sleep 1)
      (increment (- i 1))
    )
  )
)

(defn decrement [i]
  (if (> i 0)
    (
      (dosync
        (alter x dec)
        (comment println (format "in decrement %d" @x))
      )
      (Thread/sleep 1)
      (decrement (- i 1))
    )
  )
)

(defn printref [i]
  (if (> i 0) 
    (
      (dosync
        (println (format "in printref %d" @x))
      )
      (Thread/sleep 1)
      (printref (- i 1))
    )
  )
)

(future
  (increment 10)
)

(future
  (printref 15)
)

(future
  (decrement 10)
)