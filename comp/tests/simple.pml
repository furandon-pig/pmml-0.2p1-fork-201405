/* 
 * simple test on "def", "if", and "!="
 */ 
print("Testing `def' and `if'")
// comment should work
if( 1 != 1 ) {  // comment should work
  eprintf("PMML self test failed (even `if' doesn't work)\n")
  exit(1)
}
if( 1 != 2 ) {
} else {
  eprintf("PMML self test failed (even `if' doesn't work)\n")
  exit(1)
}
tmp_called = 0
def(tmp, "pp") {
  if( $1 != 1 ) {
    eprintf("PMML self test failed (even `def' doesn't work)\n")
    exit(1)
  }
  if( $2 != 2 ) {
    eprintf("PMML self test failed (even `def' doesn't work)\n")
    exit(1)
  }
  tmp_called = 1
}
tmp(1,2) 
if( tmp_called != 1 ) {
  eprintf("PMML self test failed (even `def' doesn't work)\n")
  exit(1)
}
