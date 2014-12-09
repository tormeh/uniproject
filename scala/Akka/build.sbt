name := "demo"

version := "1.0"

scalaVersion := "2.11.4"

fork := true

scalacOptions += "-optimise"

libraryDependencies ++= Seq(
  "com.typesafe.akka" %% "akka-agent" % "2.3.7",
  "com.typesafe.akka" %% "akka-actor" % "2.3.7"
  )
