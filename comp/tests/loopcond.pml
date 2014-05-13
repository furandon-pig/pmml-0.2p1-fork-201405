/*
 * test on loops, more on `if'
 */
print("Testing conditional and loops")

/* --- if --- */ 
def(iftest, "nn") {
  if( $1 ) { 3 }
  elsif( $2 ) { 4 } 
  else { 5 }
}
assert(iftest(0,0), 5)
assert(iftest(0,1), 4)
assert(iftest(1,0), 3)
assert(iftest(1,1), 3)

/* --- repeat --- */
y = 0
repeat(2) {
  y += 1
  x = 0
  while( x < 3 ) {
    y += 1
    x = x + 1
  }
}
assert(y, 8)

/* --- while and break --- */
x = 0
while( x != 3 ) {
  if( x == 1 ) { break }
  x += 1
}
assert(x, 1)

/* --- for --- */
x = 0
y = 1
for($i, 1, 5) {
  x += $i
  y *= $i
}
assert(x, 15)
assert(y, 120)

x = 0
y = 1
for($i, 5, 1, -1) {
  x += $i
  y *= $i
}
assert(x, 15)
assert(y, 120)

x = 0
for($i, 1, 3) {
  $i = 5
  x += 1
}
assert(x, 1)

/* --- foreach and break --- */
x = 0
foreach($j, #(1,4,7)) {
  for($k, 1, 3) {
    x += $j * $k
    if( $j == 4 && $k == 2 ) {
      break(2)
    }   
  }
}
assert(x, 18)

x = 0
def(test_foreach) {
  foreach($i) { x += $i }
}
test_foreach(1,3,7)
assert(x, 11)

def(add1, "n") { ($1 + 1) }
assert( repeat(3){add1(} 4 ))) , 7)  

/* --- switch --- */
x1 = 0
x2 = 0
x3 = 0
for($i,1,5) {
  switch($i) {
    case(1) { x1 += $i }
    case(3,5) { x2 += $i }
    default { x3 += $i }
  }
}
assert(x1, 1)
assert(x2, 8)
assert(x3, 6)

x = 0
for($i,0,1) {
  for($j,0,1) {
    switch(#($i,$j)) {
    case(#(0,0)) { assert(x, 0) }
    case(#(0,1)) { assert(x, 1) }
    case(#(1,0)) { assert(x, 2) }
    case(#(1,1)) { assert(x, 3) }
    }
    x += 1
  }
}

x1 = 0
x2 = 0
for($x,0,3) {
  switch(1) {
  case($x < 2) { x1 += $x }
  default { x2 += $x }
  }
}
assert(x1, 1)
assert(x2, 5)

/* --- break into parent thread --- */
x = 0
for($i,1,3) {
  x += $i
  {
    if($i == 2) { break }
  }
}
assert(x, 3)

x = 0
for($i,1,3) {
  x += $i
  if($i == 2) { [ break ] }
}
assert(x, 3)
