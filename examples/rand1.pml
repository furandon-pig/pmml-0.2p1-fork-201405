// a random music

srand()  // get a random seed from time and PID

newtrack(tk1) {
  s
  repeat(256) {
    if( irand(2) == 1 ) {
      note(n = irand(16) + c4)
    } else { R }
  }
}

newtrack(tk2) {
  h
  repeat(32) {
    x = irand(16)
    [note(n = c4+x)  note(n = c4+x+6)]
  }
}
