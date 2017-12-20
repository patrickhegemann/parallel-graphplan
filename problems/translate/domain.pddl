;; Longest simple path sequential
;;

(define (domain rpg)
  (:requirements :typing)
  (:types
        location hp-number - object
  )

  (:predicates 
     (road ?l1 ?l2 - location)
     (hero-at ?l - location)
     (hero-hp ?h - hp-number)
     (monster-at ?l - location)
     (potion-at ?l - location)
     (hp-predecessor ?h1 ?h2 - hp-number)
  )

  (:action fight
    :parameters (?l1 ?l2 - location ?h1 ?h2 - hp-number)
    :precondition (and
        (hero-at ?l1)
        (monster-at ?l2)
        (road ?l1 ?l2)
        (hero-hp ?h1)
        (hp-predecessor ?h2 ?h1)
    )
    :effect(and
        (not (monster-at ?l2))
        (not (hero-hp ?h1))
        (hero-hp ?h2)
    )
  )
  
  (:action heal
    :parameters(?l - location ?h1 ?h2 - hp-number)
    :precondition(and
        (hero-at ?l)
        (potion-at ?l)
        (hp-predecessor ?h1 ?h2)
        (hero-hp ?h1)
    )
    :effect(and
        (not (potion-at ?l))
        (not (hero-hp ?h1))
        (hero-hp ?h2)
    )
  )

  (:action move
    :parameters (?l1 ?l2 - location)
    :precondition (and
        (hero-at ?l1)
        (road ?l1 ?l2)
        (not (monster-at ?l2))
      )
    :effect (and
        (not (hero-at ?l1))
        (hero-at ?l2)
      )
  )
)
