/*
 * test on message passing
 */
print("Testing message passing")
t = 0
defthread(s1, s2, s3)

s1 {
  r(w)
  signal($s1)
  r(h)
  signal($s1, #("hello", 3))
  r(h)
  signal($s2)
}

s2 {
  wait($s1)
  assert(t, 1920u)
  wait($s1, x)
  assert(t, 2880u)
  assert(x, #("hello", 3))
  r(w)
  wait($s2)
  assert(t, 4800u)
}

s3 {
  repeat(2) { {wait($s1)} }
  assert(t, 2880u)
  { wait($s2) assert(t, 3840u) }
}

undef(s1) undef(s2) undef(s3)

/* --- alt --- */
t=0
defthread(s1, s2, s3)

s3 {
  t = 300u
  signal($se, "hi")
}

s2 {
  t = 500u
  signal($se, "hello")
  t = 1000u
  signal($sf)
}

s1 {
  x = 0u
  alt {
    case($se, msg) {
      if( msg == "hello" ) { assert(t, 500u) }
      else { assert(t, 300u) }
    }
    case($sf) {
      break
    }
    default {
      x += t
      t += 100u
    }
  }
  assert(t, 1000u)
  assert(x, 4500u)
  undef($se)
  undef($sf)
}

undef(s1) undef(s2) undef(s3)
