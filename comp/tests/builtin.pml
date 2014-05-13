/*
 * test on built-in macros
 */ 
print("Testing built-in macros")

srand(1)
assert(rand() != rand(), 1)
x = #()
repeat(8) { append(x, irand(1,8)) }
if( x != #(4,7,8,2,7,4,8,7) ) {
    eprintf("Warnning: The random number generator is different from the original one\n")
}
repeat(10) {
  $r = irand(3)
  assert($r >= 1 && $r <= 3, 1)
}

assert(floor(2.5), 2)
assert(ceil(2.5), 3)
assert(floor(-2.5), -3)
assert(ceil(-2.5), -2)
assert(abs(3), 3)
assert(abs(-3), 3)
assert(abs(2.5), 2.5)
assert(abs(-2.5), 2.5)

assert(strlen(""), 0)
assert(strlen("abc"), 3)
assert(strlen("x"+"y"), 2)

$s = "abcd"
assert(substr($s, 1, 1), "a")
assert(substr($s, 1), "abcd")
assert(substr($s, 2, 1), "b")
assert(substr($s, 2), "bcd")
assert(substr($s, 2, 2), "bc")
assert(substr($s, 2, 10), "bcd")
assert(substr($s, 4), "d")
assert(substr($s, 5), "")
assert(substr($s, 5, 1), "")
assert(substr($s, 0, 1), "a")
assert(substr($s, -1, 2), "ab")
assert(substr($s, 1, 0), "")
assert(substr($s, 1, -1), "")
assert(substr($s, 1, strlen(substr("xxxx",4,2))), "a")

assert(index($s, "a"), 1)
assert(index($s, "b"), 2)
assert(index($s, "c"), 3)
assert(index($s, "d"), 4)
assert(index($s, "e"), 0)
assert(index($s, "ab"), 1)
assert(index($s, "aa"), 0)
assert(index($s, "bc"), 2)
assert(index($s, "cd"), 3)
assert(index($s, "de"), 0)
assert(index($s, ""), 0)
assert(index("", ""), 0)
assert(index("", "a"), 0)
assert(index("", "ab"), 0)
assert(index("a", "ab"), 0)

assert(atoi("25"), 25)
assert(atof("-30.25"), -30.25)
assert(atof("1e3"), 1000)
assert(atoi("xxx"), 0)

assert(charcode("a"), 97)
assert(charcode(""), 0)
assert(charcode("a",0), 0)
assert(charcode("abc",2), 98)
assert(charcode("abc",4), 0)

assert(toupper("Doushite?"), "DOUSHITE?")
assert(tolower("Doushite?"), "doushite?")
assert(notename(61), "C#4")
assert(notename(0), "C-1")

assert(sprintf("howhow%%"), "howhow%")
assert(sprintf("%d%d%3d%03d%+4d%-4d% 3d%3.2d",32,32.8,32,32,32,32,32,3),
       "3233 32032 +3232   32 03")  
assert(sprintf("%o%03o%x%04x%X%x%02x", 27, 27, 27, 27, 27, -1, 0),
       "330331b001b1Bffffffff00")
assert(sprintf("%f %.0f %4.1f", 32.14, 32.8, 3.141),
       "32.140000 33  3.1")
assert(sprintf("%e %E", 10000, 0.25),
       "1.000000e+04 2.500000E-01")
assert(sprintf("%c%3s%-3s%5.2s%5.2s", 0x41, "ab", "ab", "abc", "a"),
       "A abab    ab    a")
assert(sprintf("%*d%*.*f", 4, 10, 5, 2, 2.1234), 
       "  10 2.12")

assert(type(1), "i")
assert(type("x"), "s")
assert(type(#()), "a")
assert(type('x'), "t")
assert(type(w), "r")
assert(type(1.2), "f")
assert(idstr(aaa), "aaa")

assert(idtype(undefined_name), 0)
def(xxx) {
  local::$t = 1
  assert(idtype($t), 1)
}
xxx()
assert(idtype(xxx), 2)
assert(idtype(printf), 3)

defthread(a_thread)
assert(idtype(a_thread), 4)
defeff(a_class) {}
attach(a_class(), a_instance, 1)
assert(idtype(a_class), 5)
assert(idtype(a_instance), 6)

assert(concat(#("ABC","D")), "ABCD")
assert(concat(#("A","B"), ","), "A,B")
assert(concat(#(), ","), "")
