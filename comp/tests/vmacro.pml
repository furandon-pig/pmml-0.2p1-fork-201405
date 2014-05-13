/*
 * test on variable macros
 */
print("Testing variable macros")
x = 80
x = x + 10  assert(x, 90) 
x += 10  assert(x, 100)
x *= 0.5 assert(x, 50)
x %= 7   assert(x, 1)
x = 1
x shl= 4  assert(x, 16)
x shr= 4  assert(x, 1)
x |= 12  x &= 5  x xor= 0xf  assert(x, 10)

x = "abc"
x = x + "d"
x += "e"
assert(x, "abcde")

x = 'x = 3'
x
assert(x, 3)

assert(defined(x), 1)
undef(x)
assert(defined(x), 0)
