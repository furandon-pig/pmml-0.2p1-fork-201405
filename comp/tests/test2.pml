/*
 * testing code for interthread communication
 */
defeff(outmsg) {
  case(note) {
    signal($e, #(t, n, v))
    reject
  }
  wrap {
    signal($f)
  }
}

newtrack(t1) {
  outmsg()
  c d e f g
}

newtrack(t2) {
  i
  alt { 
    case($e, msg) {
      v=msg[3] note(n=msg[2]+1)
    }
    case($f) {
      break
    }
  }
  ^c
}

newtrack(t3) {
  repeat(2) {
  signal($set, 'c3')
  i c d e f g a b ^c
  signal($set, 'g2')
  i ^c b a g f e d c
  }
  signal($fin)
}

newtrack(t4) {
  ch=2
  nt = 'c3'
  alt { 
    case($set, nt) {}
    case($fin) { break }
    default { q nt }
  }
}
