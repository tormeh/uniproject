fn main()
{
  let (tx, rx) = channel();
  let txclone = tx.clone();

  spawn(proc() tx.send("message"));

  spawn(proc() txclone.send("another message"));
  
  spawn(proc() println!("{:s}, {:s}" ,rx.recv(), rx.recv()) );
}
