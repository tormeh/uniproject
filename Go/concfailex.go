package main

import "fmt"
import "runtime"
import "time"

type shared struct {
  boolean bool
}

func main() {
  runtime.GOMAXPROCS(4)
  messages := make(chan string)
  sharingchan := make(chan *shared)
  sharedstruct := shared{true}
  go disaster(messages, sharingchan)
  sharingchan <- &sharedstruct
  sharedstruct.boolean = false
  msg := <-messages
  fmt.Println(msg)
}

func disaster(messages chan string, sharingchan chan *shared) {
  sharedstruct := <- sharingchan
  time.Sleep(10*time.Millisecond)
  fmt.Println(sharedstruct.boolean)
  messages <- ""
}

