println("iedrkbg")

val a = List(1,2,3,4,5,6,7,8,9)

val b = a.par.map(x=>2*x).toList

println(b)
