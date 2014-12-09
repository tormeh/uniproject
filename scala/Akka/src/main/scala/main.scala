//http://alvinalexander.com/scala/simple-scala-akka-actor-examples-hello-world-actors

import akka.actor.Actor
import akka.actor.ActorRef
import akka.actor.ActorSystem
import akka.actor.Props
import akka.agent.Agent
import scala.sys
import java.lang.Thread
import scala.collection.mutable.ArrayBuffer
import scala.util.Random
import akka.util.Timeout
import scala.concurrent._
import scala.concurrent.ExecutionContext.Implicits.global
import scala.language.postfixOps
import akka.pattern.ask
import scala.util.{Failure, Success}
 
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

class LockingActor(printstr:String) extends Actor {
  var othermessagesanswered:Int = 0
  def receive = 
  {
    case other: ActorRef =>
    {
      val response = other.ask("please reply")(50000)
      response onComplete 
      { 
        case Success(result) => println("success: " + result)
        case Failure(failure) => println(failure)
      }
      othermessagesanswered += 1
      println(printstr + ": " + othermessagesanswered.toString())
    }
    case "please reply" =>
    {
      sender() ! "this is a reply"
    }
    case s: String =>
    {
      println("got string: " + s)
    }
  }
}

class FutureSTMActor(printstr:String, agent:Agent[String]) extends Actor {
  var othermessagesanswered:Int = 0
  def receive = 
  {
    case other: ActorRef =>
    {
      val response = other.ask("please reply")(50000)
      response onComplete 
      { 
        case Success(result) => println("success: " + result); println(printstr + ": " + "STM Int says main thread in iteration: " + agent.get)
        case Failure(failure) => println(failure)
      }
      othermessagesanswered += 1
      println(printstr + ": " + othermessagesanswered.toString())
    }
    case "please reply" =>
    {
      sender() ! "this is a reply"
    }
    case s: String =>
    {
      println("got string: " + s)
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
  		val bActor = system.actorOf(Props(classOf[DemoActor], "b"))
  		aActor ! bActor
  		Thread.sleep(4)
  		scala.sys.exit()
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

    def deadlockattempt(): Unit =
    {
      val system = ActorSystem("DemoSystem")
      val aActor = system.actorOf(Props(classOf[LockingActor], "a"))
      val bActor = system.actorOf(Props(classOf[LockingActor], "b"))
      for (x <- 0 to 100000)
      {
        println(x)
        Future{ aActor ! bActor }
        Future{ bActor ! aActor }
        //Thread.sleep(1)
      }
      Thread.sleep(1000) 
      //scala.sys.exit()
    }
    
    def actorStmFuture(): Unit =
    {
      val agent = Agent("0"*10)
      val system = ActorSystem("DemoSystem")
      val aActor = system.actorOf(Props(classOf[FutureSTMActor], "a", agent))
      val bActor = system.actorOf(Props(classOf[FutureSTMActor], "b", agent))
      for (x <- 0 to 1000)
      {
        println("m: " + x.toString)
        agent send ((x.toString) * 10)
        println("m: " + agent.get.toString)
        Future{ aActor ! bActor }
        Future{ bActor ! aActor }
        //if(x%1==0) { Thread.sleep(2) }
      }
      Thread.sleep(1000)
      scala.sys.exit()
    }

  actorStmFuture()
}
