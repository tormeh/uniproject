name := "demo"

version := "1.0"

scalaVersion := "2.11.2"

fork := true

scalacOptions += "-optimise"

libraryDependencies +=
  "com.typesafe.akka" %% "akka-actor" % "2.3.6"
