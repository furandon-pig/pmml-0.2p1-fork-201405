/*
 * test on registers
 */
print("Testing registers")
assert(t, 0)
assert(tb, 0)
assert(dt, 0)
assert(sh, z)
assert(tk, 2)
assert(ch, 1)
assert(n, r)
assert(tp, 0)
assert(o, 4)
assert(key, 0)
assert(v, 80)
assert(nv, -1)
assert(ac, 16)
assert(l, q)
assert(do, 0)
assert(dp, 100)
{ ch = 2  assert(ch, 2) { ch = 3 }; assert(ch,2) }; assert(ch, 1)
{ { t = 100u };  assert(t, 100u) }&  assert(t, 0)
{
v = v + 10  assert(v, 90) 
v += 10  assert(v, 100)
v *= 0.5 assert(v, 50)
v %= 7   assert(v, 1)
v shl= 4  assert(v, 16)
v shr= 4  assert(v, 1)
v |= 12  v &= 5  v xor= 0xf  assert(v, 10)
t = 0 tb = q  rt = h  assert(t, q+h) assert(rt, h)
assert(du, 480u) du += 240u assert(du, 720u) assert(dp, 0) assert(do, 720u)
dr += 30 assert(dp, 30) assert(do, 0)
}&
