/*
 * test on threads
 */ 
print("Testing threads")
x = 5  y = 10 
{ x = 6  assert(x,6) ...::x = 7 ..::y = 11 }
assert(x,7)
assert(y,11)

defthread(t1)
x = 0
t1 {
  x = 1
  v = 100
  defthread(t2, t3)
  t2 {
    x += 1
    v = 120
    { ...::y = 8 }
  }
}
assert(x, 0)
assert(t1::x, 1)
assert(t1::t2::x, 2)
assert(global::t1::t2::x, 2)
t1 {
  assert(x, 1)
  assert(t2::x, 2)
  assert(v, 100)
}
t1::t2 {
  assert(x, 2)
  assert(v, 120)
  assert(y, 8)
}
::t1::t2 {
  assert(x, 2)
}
t2 {
  assert(x, 2)
}
t1 {
  undef(x)
  assert(x, 0)
  t2 {
    assert(x, 2)
    { x += 1  assert(x,3) }
    assert(x, 2)
    { x += 1  assert(x,3) undef(x) assert(x,2) }
    undef(x)
    assert(x, 0)
  }
}
t1 {
  undef(t3)
}
undef(t1)

{ defthread(t4) }

/* --- sync --- */
defthread(th1,th2)
t += 100u
th1 { t += 100u }
th1 { t += 50u }
assert(t, 100u)
===
assert(t, 250u)


/* --- inheritance of local macros --- */ 
defthread(t_melody, t_bass)

def(part1) {
  local::$t = 8

  t_melody {
    assert($t, 8) assert($1, 5)
  }
  {
    assert($t, 8) assert($1, 5)
  }&
}

part1(5)
