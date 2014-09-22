use std::sync::Future;

fn main() 
{   
  fn fib(n: u64) -> u64 
  {
    // lengthy computation returning an uint
    12586269025
  }

let mut delayed_fib = Future::spawn(proc() fib(50));
println!("fib(50) = {}", delayed_fib.get());
} 