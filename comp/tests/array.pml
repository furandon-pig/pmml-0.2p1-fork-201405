/*
 * test on arrays
 */
print("Testing arrays")

assert(# #(), 0)
assert(# #(1,1), 2)
assert(#(1,3)[2], 3)
$a = #(10,20,30)
assert($a[3], 30)

$bb = #( #(1,2,3), #(4,5,6), #(7,8,9) )
assert($bb[2][2], 5)
x = $bb[1]
assert(x[3], 3)

$c = #('x = 1', 'y = 1')
x = 0 
y = 0
$c[1]
assert(x, 1)
assert(y, 0)

assert(#() == #(), 1)
assert(#(1,2) == #(1,2), 1)
assert(#(1,2) != #(1,2), 0)
assert(#(1,2) == #(1,3), 0)
assert(#(1,2) == #(1), 0)
assert(#(1,2) == #(1,"abc"), 0)
assert(#(1, #(2,3)) == #(1, #(2,3)), 1)
assert(#(1, #(2,3)) == #(1, #(2,4)), 0)

$a = #(10,20,30)
$a[3] = 40
$a[1] += 10
assert($a, #(20,20,40))

$a = #(1,2,3)
$b = $a
$c = dup($a)
$a[3] = 5
assert($a, #(1,2,5))
assert($b, #(1,2,5))
assert($c, #(1,2,3))
$d = #(@$a, @$c)
assert($d, #(1,2,5,1,2,3))
$e = #(1, #(2,3))
$d = #(@$a, @$e)
assert($d, #(1,2,5,1,#(2,3)))

assert(rep(3,2), #(2,2,2))

for($j, 1, 10) {
  $a = #()
  for($i, 1, $j) { append($a, $i) }
  for($i, $j+1, $j*2, 2) { append($a, $i, $i+1) }
  for($i, 0, -$j, -1) { insert($a, $i) }
  for($i, -$j, $j*2) {
    assert($i, $a[$i + $j + 1])
  }
}

$a = #( for($i,1,5) { $i, } )
assert($a, #(1,2,3,4,5))

$a = #(1,2,3,4,5)
shift($a, 2)
assert($a, #(3,4,5))
shift($a, -2)
assert($a, #(3))
append($a,7,8,9)
insert($a,0)
assert($a, #(0,3,7,8,9))
shift($a)
assert($a, #(3,7,8,9))
shift($a,-2)
assert($a, #(3,7))
shift($a,3)
assert($a, #())
