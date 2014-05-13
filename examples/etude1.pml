/*
 * Etude Op. 12-1     F. Chopin
 *   With a little velocity and tempo control
 */
tempo(176)

def(pat_up, "qqqq:qq") {
  s 
  vmag(1.0)
  r $1 $2 $3 $4++ 
  {tp+=12 $1 $2 $3}
  if( $# == 6 ) { $5 $6 } 
  else {
    {tp+=12 $4++} {tp+=24 $1 $2 $3} 
    if( $# == 5 ) { $5 }
    else { {tp+=24 $4++} {tp+=36 $1 $2 $3} } 
  }
  vmag_to(1.2)
}

def(pat_down, "qqqq:qq") {
  s 
  vmag(1.2)
  {tp+=36 $4++ $3 $2 $1} 
  {tp+=24 $4++ $3 $2 $1} 
  if( $# == 6 ) { $5 $6 }
  else {
    {tp+=12 $4++ $3 $2 $1} 
    if( $# == 5 ) { $5 }
    else { $4++ $3 $2 $1 }
  }
  vmag_to(1.0)
}

def(pat_ud, "qqqq") {
  pat_up($1,$2,$3,$4)
  pat_down($1,$2,$3,$4)
}

newtrack(rh) {ch=1 v=70 ac=10  /*rand_vel(5) rand_ntime(w)*/  }
newtrack(lh) {ch=1 v=90}

===
marker("A")
rh { pat_ud(c3,g3,c4,e4) }
lh { 2w [c2 c3](hold) } 

rh { pat_up(c3,a3,c4,f4) pat_down(c3,a3,c4,e4, d4++ c4 a3 c3) }
lh { w [f1 f2](hold) [f#1 f#2](hold) } 

rh { pat_up(b2,g3,b3,d4, d6++ b5 g6 a6) pat_down(c3,f#3,a3,d4) }
lh { h [g1 g2](hold) q [f#1 f#2](hold) [e1 e2](hold) w [d1 d2](hold) }

rh { pat_up(c3,f3,ab3,d4) pat_down(b2,f3,g3,d4, d#4++ g3 f3 b2) }
lh { { w hold h. hold q hold }& 2w [g1 g2] } 

===
marker("A2")
rh { pat_ud(c3,g3,c4,e4) }
lh { 2w [c2 c3](hold) } 

rh { pat_up(c3,f3,c4,f4) pat_down(c3,f#3,c4,e4) }
lh { {w hold hold }& 2w [a1 a2] } 

rh { pat_up(c3,g3,c4,d4) pat_down(b2,g3,b3,d4) }
lh { {w hold hold }& 2w [g1 g2] } 

rh { pat_up(d3,g3,d4,e4) pat_down(c3,g3,c4,e4) }
lh { {w hold hold }& 2w [c2 c3] } 

===
marker("B")
rh { pat_up(e3,c4,e4,f4) pat_down(d3,b3,d4,f4) }
lh { w [a1 a2](hold) h [b1 b2](hold) [a1 a2](hold) }

rh { pat_up(d3,b3,d4,e4) pat_down(c3,a3,c4,e4) }
lh { w [g#1 g#2](hold) h [a1 a2](hold) [g1 g2](hold) }

rh { pat_up(c3,a3,c4,e4) pat_down(b2,a3,b3,d#4) }
lh { {w hold hold }& 2w [f1 f2] }

rh { pat_up(b2,a3,b3,e4) pat_down(b2,g#3,b3,e4) }
lh { {w hold hold }& 2w [e1 e2] }

===
marker("B2")
rh { pat_up(e3,a3,c#4,g4, g6++ c#6 a5 e5) pat_down(e2,a2,c3,g3, f#4++ c4 a3 d3) }
lh { w [a1 a2](hold) {h. hold q hold}& [d2 d3] }

rh { pat_up(d3,g3,c4,f4) pat_down(d3,g3,b3,f4) }
lh { {w hold hold }& 2w [g1 g2] }

rh { pat_up(c3,g3,bb3,e4) pat_down(c3,eb3,bb3,eb4) }
lh { w [c2 c3](hold) [gb1 gb2](hold) }

rh { pat_up(c3,eb3,a3,eb4) pat_down(cb3,eb3,ab3,eb4) }
lh { w [f1 f2](hold) [cb2 cb3](hold) }

rh { pat_up(bb2,f3,ab3,d4) pat_down(bb2,e3,g#3,d4) }
lh { w bb1(hold gt=2w) bb2(hold) }

rh { pat_ud(a2,e3,a3,c#4) }
lh { 2w [a1 a2](hold) }

rh { pat_up(a2,d3,f#3,c4) pat_down(g2,d3,f3,c4, b3++ f3 d3 g2) }
lh { w d2(hold) {h. hold q hold}& [g1 g2] }

rh { pat_up(g2,c3,e3,b3) pat_down(f2,c3,e3,b3, a3++ e3 c3 f2) }
lh { w c2(hold) {h. hold q hold}& [f1 f2] }

rh { pat_up(f2,b2,d3,a3) }
lh { w b1(hold) }

rh { s g6++ d6 b5 e5 g5++ d5 b4 e4 } 
lh { h [e1 e2](hold) }

rh { s a4++ c5 g5 e5 a5++ c6 g6 e6 }
lh { h [a1 a2](hold) }

rh { s f6++ c6 a5 d5 f5++ c5 a4 d4 }
lh { h [d1 d2](hold) }

rh { s g4++ b4 f5 d5 g5++ b5 f6 d6 }
lh { h [g1 g2](hold) }

rh { s e6++ b5 g5 c5 e5++ b4 g4 c4 }
lh { h [c1 c2](hold) }

rh { s f4++ a4 e5 c5 f5++ a5 e6 c6 }
lh { h [f1 f2](hold) }

rh { pat_down(b1,f2,a2,d3, r(q)) pat_down(b1,f#2,a2,d#3, r(q)) }
lh { {w hold hold}& {h. b2 r(q) b2 r(q)}& {h. b1 w b1++ q b1++}}

===
{r(w.) rtempo(1.0) r(h) rtempo_to(0.5)}& 
rh { pat_up(b2,e3,g#3,e4) pat_down(b2,e3,g#3,e4, f4++ b3 g3 d3) }
lh { w+h. [e1 e2](hold) q [d1 d2](hold) }

===
marker("C")
{r(i) rtempo_to(1.0) }&
rh { pat_ud(c3,g3,c4,e4) }
lh { 2w [c2 {w c1 c3}](hold) } 

rh { pat_up(c3,a3,c4,f4) pat_down(c3,a3,c4,e4, d4++ c4 a3 c3) }
lh { w [f1 f2](hold) [f#1 f#2](hold) } 

rh { pat_up(b2,g3,b3,d4, d6++ b5 g6 a6) pat_down(c3,f#3,a3,d4) }
lh { h [g1 g2](hold) q [f#1 f#2](hold) [e1 e2](hold) w [d1 d2](hold) }

rh { pat_up(c3,f3,ab3,d4) pat_down(b2,f3,g3,d4, d#4++ g3 f3 b2) }
lh { {w hold h. hold q hold }& 2w [g1 g2] } 

===
marker("C2")
rh { pat_ud(c3,g3,c4,e4) }
lh { 2w [c2 c3](hold) } 

rh { pat_up(c3,f3,c4,f4) pat_down(c3,f#3,c4,e4, eb5++ c5 f#4 c4, eb4++ c4 f#3 c3) }
lh { {w hold h hold q hold hold }& w+h. [a1 a2] q [ab1 ab2]} 

rh { pat_up(c3,g3,c4,d4) pat_down(b2,g3,b3,d4) }
lh { {w hold hold}& 2w [g1 g2] }

rh { pat_up(c3,a3,c4,d4) pat_down(c3,a3,c4,eb4, d#5++ b4 a4 b3, d#4++ b3 a3 b2) }
lh { {w hold h hold q hold hold }& w [f#1 f#2] [f1 f2]} 

rh { pat_ud(b2,g#3,b3,e4) }
lh { 2w [e2 {w e1 e3}](hold) } 

rh { pat_up(d3,a3,c4,f4) pat_down(d3,g3,b3,f4) }
lh { w d2(hold) [g1 g2](hold) }

rh { pat_up(c3,g3,c4,e4, e5++ c5 bb5 c6, g6++ c6 bb5 e5) 
     pat_down(eb2,a2,c3,f#3, f5++ b4 g#4 d4, f4++ b3 g#3 d3)}
lh { {w hold h hold q hold hold }& w c2(gt=2w) c3}

rh { pat_up(c3,g3,c4,e4, e6++ c6 g5 c5)
     pat_down(c2,f#2,a2,eb3, d5++ ab4 f4 b3, d4++ ab3 f3 b2) }
lh { {w hold h hold q hold hold }& w c2 [g1 g2]}

rh { pat_up(bb2,e3,g3,db4) pat_down(a2,eb3,f#3,db4, c5++ f#4 eb4 a3) }
lh { {w hold h. hold q hold }& 2w [g1 g2]}

rh { pat_up(ab3,d4,f4,c5, c7++ f6 d6 ab5) pat_down(g2,d3,f3,b3) }
lh { {w hold hold }&  2w [g1 g2]}

===
{r(w.) rtempo(1.0) r(h) rtempo_to(0.6) rtempo(0.8) }&
rh { pat_ud(g2,e3,g3,c4) }
lh { 2w [c1 c2](hold) }

lh { 2w [c1 c2](hold) }
