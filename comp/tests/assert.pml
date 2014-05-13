/*
 * assert macro
 */
::assert_count = 0

def(assert, "ee") {
  if( $1 != $2 ) {
    eprintf("*** Test No. %d : ", ::assert_count) 
    eprint($1, " should be ", $2)
    error("PMML self test failed");
  }
  ::assert_count += 1
}

