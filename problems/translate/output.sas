begin_version
3
end_version
begin_metric
0
end_metric
5
begin_variable
var0
-1
5
Atom hero-at(l1)
Atom hero-at(l2)
Atom hero-at(l3)
Atom hero-at(l4)
Atom hero-at(l5)
end_variable
begin_variable
var1
-1
4
Atom hero-hp(hp1)
Atom hero-hp(hp2)
Atom hero-hp(hp3)
Atom hero-hp(hp4)
end_variable
begin_variable
var2
-1
2
Atom monster-at(l3)
NegatedAtom monster-at(l3)
end_variable
begin_variable
var3
-1
2
Atom monster-at(l5)
NegatedAtom monster-at(l5)
end_variable
begin_variable
var4
-1
2
Atom potion-at(l3)
NegatedAtom potion-at(l3)
end_variable
2
begin_mutex_group
5
0 0
0 1
0 2
0 3
0 4
end_mutex_group
begin_mutex_group
4
1 0
1 1
1 2
1 3
end_mutex_group
begin_state
0
1
0
0
0
end_state
begin_goal
1
0 4
end_goal
13
begin_operator
fight l2 l3 hp2 hp1
1
0 1
2
0 1 1 0
0 2 0 1
1
end_operator
begin_operator
fight l2 l3 hp3 hp2
1
0 1
2
0 1 2 1
0 2 0 1
1
end_operator
begin_operator
fight l2 l3 hp4 hp3
1
0 1
2
0 1 3 2
0 2 0 1
1
end_operator
begin_operator
fight l4 l5 hp2 hp1
1
0 3
2
0 1 1 0
0 3 0 1
1
end_operator
begin_operator
fight l4 l5 hp3 hp2
1
0 3
2
0 1 2 1
0 3 0 1
1
end_operator
begin_operator
fight l4 l5 hp4 hp3
1
0 3
2
0 1 3 2
0 3 0 1
1
end_operator
begin_operator
heal l3 hp1 hp2
1
0 2
2
0 1 0 1
0 4 0 1
1
end_operator
begin_operator
heal l3 hp2 hp3
1
0 2
2
0 1 1 2
0 4 0 1
1
end_operator
begin_operator
heal l3 hp3 hp4
1
0 2
2
0 1 2 3
0 4 0 1
1
end_operator
begin_operator
move l1 l2
0
1
0 0 0 1
1
end_operator
begin_operator
move l2 l3
1
2 1
1
0 0 1 2
1
end_operator
begin_operator
move l3 l4
0
1
0 0 2 3
1
end_operator
begin_operator
move l4 l5
1
3 1
1
0 0 3 4
1
end_operator
0
