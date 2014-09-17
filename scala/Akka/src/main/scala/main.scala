//http://alvinalexander.com/scala/simple-scala-akka-actor-examples-hello-world-actors

import akka.actor.Actor
import akka.actor.ActorRef
import akka.actor.ActorSystem
import akka.actor.Props
import scala.sys
import java.lang.Thread
import scala.collection.mutable.ArrayBuffer
import scala.util.Random
 
class DemoActor(printstr:String) extends Actor {
  def receive = {
    case other: ActorRef =>
    {
    	println(printstr)
    	other ! self 
    }
    case _       => println("unknown message")
  }
}

class CreationActor(printstr:String) extends Actor {
  /*override def preStart(): Unit =
  {
  	print(" ")
  	print(printstr)
  }*/
  def receive = 
  {
    case _       => println("got message")
  }
}

class ContextCreationActor(printstr:String) extends Actor {
  def receive = 
  {
    case _       =>
    {
    	val ab = ArrayBuffer[ActorRef]()
    	val amount = 1000000
    	for (x <- 0 to amount)
			{
				ab += context.actorOf(Props(classOf[CreationActor], scala.util.Random.nextString(5)))
			}
			ab(Random.nextInt(amount+1)) ! "mess"
    	println("actors created")
    	scala.sys.exit()
    }
  }
}


object Main extends App 
{
		def demo(): Unit =
		{
			val system = ActorSystem("DemoSystem")
  			// default Actor constructor
  			val aActor = system.actorOf(Props(classOf[DemoActor], "a"))
  			al bActor = system.actorOf(Props(classOf[DemoActor], "b"))
  			Actor ! bActor
  			hread.sleep(4)
  			cala.sys.exit()
		}   
		
		def creation(): Unit =
		{
			val system = ActorSystem("DemoSystem")
			val ab = ArrayBuffer[ActorRef]()
			val amount = 1000000
			for (x <- 0 to amount)
			{
				ab += system.actorOf(Props(classOf[CreationActor], scala.util.Random.nextString(5)))
			}
			ab(Random.nextInt(amount+1)) ! "mess"
			Thread.sleep(100)
			scala.sys.exit()
		}
		
		def contextcreation(): Unit =
		{
			val system = ActorSystem("DemoSystem")
			val top = system.actorOf(Props(classOf[ContextCreationActor], scala.util.Random.nextString(5)))
			top ! "mess"
			//Thread.sleep(1000)
			//scala.sys.exit()
		}

  creation()
}
