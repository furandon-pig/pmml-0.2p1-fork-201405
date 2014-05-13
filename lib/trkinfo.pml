/*
 * Show the list of tracks  (necessary for the PMML mode of Emacs)
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

defeff(trkinfo) {
  init {
    def(shrink, "s") {  // remove leading and trailing spaces
      local::$i = 1
      local::$j = strlen($1)
      while( local::$c = substr($1, $i, 1) $c == " " || $c == "\t" ) {
	$i += 1
      } 
      while( $j > $i && {local::$c = substr($1, $j, 1)
	     $c == " " || $c == "\t"} ) {
	$j -= 1
      }
      substr($1, $i, $j-$i+1)
    }

    def(output) {
      while( track < tk ) {
	printf("  %2d  %-20.20s  %-40.40s\n",
	       track, trkname, concat(texts, " "))
	trkname = ""
	texts = #()
	track += 1
      }
    }

    trkname = ""
    texts = #()
    track = 1
    printf("  Trk   Trk/Seq Name          Texts\n");
    printf("  ==  ====================  ========================================\n");
  }
  
  case(text(1)) {
    output()
    $s = shrink(val)
    if( $s != "" ) {
      append(texts, $s)
    }
  }
  
  case(text(3)) {
    output()
    $s = shrink(val)
    if( $s != "" && trkname == "" ) {
      trkname = $s
    }
  }

  case(all) {
  }

  wrap {
    tk+=1
    output()
  }
}

trkinfo()
