/*
 * macro utilities  <undocumented>
 */

/*
 *  Copyright (C) 1997,1998   Satoshi Nishimura
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * equal: examine the conincidence of two arrays
 *   Each element of the array should not be a token list.
 */
/* Now '==' can be used instead of 'equal' */
/*
def(equal, "aa") {
  local::$t = 1
  if( #$1 != #$2 ) { $t = 0 }
  else { 
    for($i, 1, #$1) {
      if( type($1[$i]) != type($2[$i]) ) {
        $t = 0  break
      } elsif( type($1[$i]) == "a" ) {
        if( !equal($1[$i], $2[$i]) ) { 
          $t = 0  break
        }
      } else {
        if( $1[$i] != $2[$i] ) {
          $t = 0  break
        }
      }
    }
  }
  $t
}
*/

/*
 * sum(array): calculate the sum of all the elements in an array
 */
def(sum, "a") {
  local::$s = 0
  foreach($i, $1) { $s += $i }
  $s
}

/*
 * concat(array, delimiter): concatenate all the string elements in an array
 */
/* Now this is implemented as built-in */
/*
def(concat, "as") {
  local::$s = ""
  for($i, 1, #$1) {
    if( $i == #$1 ) {
      $s += $1[$i]
    } else {
      $s += $1[$i] + $2
    }
  }
  $s
}
*/

/*
 * swap: swap two objects
 */
def(swap, "qq") {
  local::$t = $1
  $1 = $2
  $2 = $t
}

/*
 * sort: sort array elements using quick-sort
 *   The second argument is a comparing equation which should be true
 *   when "$$1" is considered to be less than "$$2". 
 *   The equation must be surrounded by single quotes.
 *   If the second argument is omitted, '$$1 < $$2' is assumed, in that case,
 *   elements are sorted in accending order of numbers or strings. 
 *   This sort is not stable; the order in the output of two elements
 *   which compare as equal is unpredictable.
 *
 *   Example:  sort(array)                // accending order
 *   Example:  sort(array, '$$1 > $$2')   // decending order
 */
def(sort, "a:t") {
   if( $# > 1 ) { edef($_sort_compare, "ee") { $2 } }
   else { def($_sort_compare, "ee") { $1 < $2 } } 
   $_sort($1, 1, #$1)
}

def($_sort, "ann") {
   if( $2 == $3 - 1 ) {
       if( $_sort_compare($1[$3], $1[$2]) ) {
	  swap($1[$2], $1[$3])
       }	   
   } elsif( $2 < $3 ) {
       local::$pivot = $1[($2 + $3) shr 1]  
       local::$i = $2
       local::$j = $3
       while($i <= $j) {
	  while( $_sort_compare($1[$i], $pivot) ) { $i += 1 }
	  while( $_sort_compare($pivot, $1[$j]) ) { $j -= 1 }
	  if( $i < $j ) { 
	      swap($1[$i], $1[$j])
	      $i += 1
	      $j -= 1
	  } elsif( $i == $j ) {
	      $i += 1
	      $j -= 1
	  }
       }
       $_sort($1, $2, $j)
       $_sort($1, $i, $3)
   }
}
