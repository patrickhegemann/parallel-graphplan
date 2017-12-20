; Transport city-sequential-5nodes-1000size-2degree-100mindistance-2trucks-4packages-2014seed

(define (problem lsp01)
 (:domain rpg)
 (:objects
  l1 l2 l3 l4 l5 - location
  hp1 hp2 hp3 hp4 - hp-number
 )
 (:init
    (hp-predecessor hp1 hp2)
    (hp-predecessor hp2 hp3)
    (hp-predecessor hp3 hp4)
    
    (road l1 l2)
    (road l2 l3)
    (road l3 l4)
    (road l4 l5)
    
    (monster-at l3)
    (monster-at l5)
    
    (potion-at l3)

    (hero-at l1)
    (hero-hp hp2)
 )
 (:goal (and
    (hero-at l5)
 ))
)
