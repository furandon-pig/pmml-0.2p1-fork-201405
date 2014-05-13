/*
 * test on effectors
 */ 
print("Testing effectors")

defeff(tester) {
   attach {
     expected = #( 
		  #(0u, 2, 1, 72, 80, -1, 480u),
		  #(0u, 2, 1, 60, 90, -1, 480u),
		  #(480u, 2, 1, 74, 80, -1, 480u),
		  #(480u, 2, 1, 62, 90, -1, 480u),
		  #(960u, 2, 1, 76, 80, -1, 480u),
		  #(960u, 2, 1, 64, 90, -1, 480u),
		  #(1440u, 2, 1, 77, 80, -1, 480u),
		  #(1440u, 2, 1, 65, 90, -1, 480u),
	        )
     cnt = 1
   }
   case(note) {
     assert(#(t, tk, ch, n, v, nv, gt), expected[cnt])
     cnt += 1
     reject
   }
}
defeff(octaver) {
   case(note) {
      note(n+=12)&
      v+=10
   }
}

t = 0
{
tester()
{octaver() t=0 C D E} F(octaver())
}&


defeff(tester) {
   case(note) {
     assert(#(t, tk, ch, n, v, nv, gt), #(0u, 2, 1, 60, 80, 30, 480u))
     reject
   }
   case(kp) {
     assert(#(t, tk, etype, val, n), #(0u, 2, 129, 70, 73))
     reject
   }
   case(ctrl) {
     assert(#(t, tk, etype, val), #(0u, 2, 7, 120))
     reject
   }
   case(arbit) {
     assert(#(t, tk, etype, val), #(0u, 2, 0x191, #(1,2)))
     reject
   }
   case(seqno) {
     assert(#(t, tk, etype, val), #(0u, 1, 0x100, 1237))
     reject
   }
   case(text) {
     assert(#(t, tk, etype, val), #(0u, 2, 0x102, "hower"))
     reject
   }
   case(keysig) {
     assert(#(t, tk, etype, val), #(0u, 2, 0x159, #(5,0)))
     reject
   }
}
defeff($e1) { case(note_off) { nv = 30 } }
defeff($e2, "") { case(ctrl(7)) { val *= 1.5 }
                  case(kp) { n += 1 } }
defeff($e3, "") { case(excl,arbit) { val = #(1,2) } }
defeff($e4, "nn") { 
   init { seqno_inc = $1 }
   case(seqno) { val += seqno_inc } 
   case(text) { val += "er" } 
   case(meta) { val[1] = $2 }
}
defthread(t1)
t1 {   
  tester()
  t=0
  $e2()
  attach($e3(), howhow, 1)
  attach($e4(3,5), instance, 1)
  c4($e1())&
  vol(80)
  n=c5
  kp(70)
  arbit(#(1,2,3,0xff))
  seqno(1234)
  text(2, "how")
  keysig(3,0)
}
undef(t1)


defeff(tester) {
   attach {
     expected = #( 
		  #(0u, 2, 1, 62, 80, -1, 480u),
	        )
     cnt = 1
   }
   case(note) {
     assert(#(t, tk, ch, n, v, nv, gt), expected[cnt])
     cnt += 1
     reject
   }
   case(excl) {
     assert(val, #(1,0xf7))
     reject
   }
}
defeff(accept_channels) {
  attach {
    eff_events(note, ctrl(0-191))
    thru_chs($@)
  }
}
defeff(reject_events) { 
  attach {
    eff_events($@)
  }
  case(all) {  
    reject
  }
}
{ tester() accept_channels(1) reject_events(arbit) 
  t=0 d4(ch=1) d4(ch=2) excl(#(1)) arbit(#(1)) }&


defeff(tester) {
   attach {
     expected = #( 
		  #(0u,2,1,60,100,-1,480u),
		  #(480u,2,1,61,120,-1,480u),
		  #(960u,2,1,62,100,-1,480u),
		  #(1440u,2,1,64,80,-1,480u),
		  #(1920u,2,1,65,120,-1,480u),
                 )
     cnt = 1
   }
   case(note) {
     assert(#(t, tk, ch, n, v, nv, gt), expected[cnt])
     cnt += 1
     reject
   }
}
defeff(yyy) {
  case(note) { v += 20 }
}
{
tester()
attach(yyy(), yyy1, 0)
attach(yyy(), yyy2, 1)
v=80
c4
enable(yyy1)
c#4
disable(yyy2)
d4
disable(yyy1)
e4
enable(yyy)
f4
}&

x = #()
defeff(eff, "s") { 
  attach { name = $1 }
  wrap { append(x, name) }
}
defthread(t1)
t1 {
  attach(eff("eff1"), eff1, 1)
  {
     attach(eff("eff2"), eff2, 1)
     defthread(t2, t3)
     t2 {
	attach(eff("eff3"), eff3, 1)
     }
     t3 {
	attach(eff("eff4"), eff4, 1)
     }
  }
}
undef(t1)
assert(x, #("eff4","eff3","eff2","eff1"))
