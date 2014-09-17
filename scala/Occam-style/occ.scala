/*
Replicating this Occam-snippet:
SEQ
  PAR
    x = function(1)
    y = function(2)
  z = x+y
*/

import scala.concurrent._
import scala.concurrent.ExecutionContext.Implicits.global
import scala.util.{Failure, Success}
import scala.concurrent.duration._
import scala.language.postfixOps

object occ extends App
{
  def function(i: Int) = i*i

  val x = Future {function(1)}
  val y = Future {function(2)}

  val results = List(x, y).par.map(v => Await.result(v, 0 nanos))
  val zval = results(0) + results(1)

  println(zval)
}