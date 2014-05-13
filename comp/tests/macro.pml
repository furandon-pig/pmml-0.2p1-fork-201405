/*
 * test on macros
 */ 
print("Testing macros")

def(abc, "qnirfsppate*") {
  assert($1, c4)
  assert($2, 15)
  assert($3, 15)
  assert(type($3), "i")
  assert($4, 15)
  assert(type($4), "r")
  assert($5, 15)
  assert(type($5), "f")
  assert($6, "doushite")
  assert($7, 20)
  assert($8, "attode")
  assert($9, #(1,2,3))
  assert($10, 'c4')
  assert($11, 'w')
  assert($12, c5)
  assert($#, 12)
}
abc(c4, 10+5, 15.0, 15.0, 15.0, 
	"doushite", 20, "attode", #(1,2,3), 'c4', 'w', c5) 

def(macro1) {
  opt = 0
  $1
  assert(opt, 3)
}
macro1(opt=3)

def(m1, "qns") {
  assert($1, c4) 
  assert($2, 5) 
  assert($3, "x")
} 
def(m2) { m1(c4, $@) }
m2(5, "x")

def(abc, "n*") {
  assert($*, #(4,5,7))
  $*[3] = 6
  assert($*, #(4,5,6))
  assert($1, 4) assert($2, 5) assert($3, 6)
  assert($#, 3)
  shift
  assert($*, #(5,6))
  assert($1, 5) assert($2, 6)
  assert($#, 2)
  shift($*, -1)
  assert($*, #(5))
  assert($1, 5)
  assert($#, 1)
}
abc(4,5,7)

def(abc, "e*") { x = $* }
abc(1,2,3)
assert(x, #(1,2,3))
append(x, 4)
assert(x, #(1,2,3,4))

def(abc, "n:qs*") {
  assert($#, $1)
  if( $# == 1 ) { assert(null($2), 1) }
}
abc(1)
abc(2, x)
abc(3, x, "a")

x = 0
zzz = 'foreach($i, #(1,2,3)) {'
zzz x += $i }
assert(x, 6)

x = 0
def(abc, "l") { foreach($1, #(1,2,5)) }
abc($i) { x += $i }
assert(x, 8)

def(xxx) {
  local::x = 0
  def(abc, "l") { foreach($1, #(1,2,5)) }
  abc($i) { x += $i }
  assert(x, 8)
}
x = 1
xxx()
assert(x, 1)

def(abc, "n*") {
  shift
  shift
}
def(xxx, "n*") {
  abc(1,2,3)
  assert($*, #(5,6,7))
}
xxx(5,6,7)

def(abc) { assert($1[1], 1) }
def(ccc) { abc($*) } 
ccc(1,2,3)

def(xxx, "qn") { 
  edef($1) { $$1 = $2 }
}
xxx(x, 5)
x(y)
assert(y,5)

y = 0
def(xxx, "n*") {
  while( !null($1) ) {
    y += $1
    shift
  }
}
xxx(3,6,7) 
assert(y, 16)

def(inner) {
  local::x = 3
  assert(x, 3)
  undef(x)
  assert(x, 1)
}
def(mac) {
  local::x = 5
  inner()
  assert(x, 5)
  undef(x)
  assert(x, 1)
}
x = 1
mac()


def(abc) {
  assert($1, 5)
  $1 = 6
}
def(ccc) {
  abc($1)
}
def(cccc) {
  local::$t = 5
  ccc($t)
  assert($t, 6)
}
cccc()

/* sum test */
def(sum, "a") {
   local::$s = 0
   foreach($i, $1) { $s += $i }
   $s
}
x = #(1,2,3,5)
assert(sum(x), 11)
y = 2 + sum(x) * 2
assert(y, 24)

/* sort test */
def(swap, "qq") {
   local::$t = $1
   $1 = $2
   $2 = $t
}
def(sort, "a:t") {   // bubble sort
   if( $# > 1 ) { edef(local::compare, "ee") { $2 } }
   else { def(local::compare, "ee") { $1 > $2 } } 

   for($i, 1, #$1-1) {
     for($j, $i+1, #$1) { 
        if( compare($1[$i], $1[$j]) ) {
	   swap($1[$i], $1[$j])
	}
     }
   }
}
def(reverse, "a") {  // reverse elements
   local::$t = #()
   foreach($i, $1) { insert($t, $i) } 
   $t
}
def(reverse2, "a") {  // another implementation
   for($i, 1, floor(#$1/2)) {
      swap($1[$i], $1[#$1 + 1 - $i])
   }
}
data = #(3,2,4,8,1,6,7,5)
sort(data)
assert( data, #(1,2,3,4,5,6,7,8) )
assert( reverse(data), #(8,7,6,5,4,3,2,1) )
reverse2(data)
assert( data, #(8,7,6,5,4,3,2,1) )
data = #(3,2,4,8,1,6,7,5)
sort(data, '$$1 < $$2')
assert( data, #(8,7,6,5,4,3,2,1) )
data = #(3,2,4,8,1,6,7,5)
sort(data, '$$1 < $$2')
assert( data, #(8,7,6,5,4,3,2,1) )
data = #(#(3, "doushite"), #(5, "attode"),
	 #(1, "chigauyou"), #(2, "iindesu"))
sort(data, '$$1[1] > $$2[1]')
assert( data, #(#(1,"chigauyou"),#(2,"iindesu"),#(3,"doushite"),#(5,"attode")))

/* recursion test */
def(fib, "n") {
  if( $1 < 2 )  { 1 }
  else { (fib($1-1) + fib($1-2)) }
}
def(fib2, "n") {
  if( $1 < 2 ) { 1 }
  else { local::tmp = fib2($1-1) + fib2($1-2)  tmp }
}
assert(fib(7), 21)
assert(fib2(7), 21)

/* eval tests */
x = 'y'
eval(x) = 5
assert(defined(y), 1)
assert(y, 5)
y = '$z'
eval(eval(x)) = 5
assert(defined($z), 1)
assert($z, 5)

$table = #('one', 'two', 'three', #('four'))
for($i, 1, 3) {
  eval($table[$i]) = $i
}
eval($table[4][1]) = 4
assert(defined(one) && defined(two) && defined(three) && defined(four), 1)
assert(one, 1)
assert(two, 2)
assert(three, 3)
assert(four, 4)

y = 5
edef(xxx) { x = eval(y) xx = eval(#(y,y+1,y+2)) }
y = 3
xxx()
assert(x, 5)
assert(xx, #(5,6,7))

table = #( #("a", 5), 
	   #("b", 6),
	   #("c", 7) ) 
x = 1
while( x <= 3 ) {
  evalstr("xxx" + table[x][1]) = table[x][2]
 x += 1
}
assert(xxxa, 5)
assert(xxxb, 6)
assert(xxxc, 7)

x = "for"
y = 0
evalstr(x)($i,1,2) { y += $i }
assert(y, 3)

def(macro1) {
  assert($1, 1)
  { assert($1, 1) }
  { assert($1, 1) }&
  [ assert($1, 1) ]
  [ assert($1, 1) ]&
}
macro1(1)

assert(ecode(note_on), 403)
assert(ecode(ctrl(7)), 7)
assert(ecode(timesig), 344)
assert(ecode(text(2)), 258)

