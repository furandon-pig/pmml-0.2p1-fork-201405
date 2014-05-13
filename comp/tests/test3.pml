/*
 * testing code for continuous control change
 */

ctrl(7,0)
q C D
ctrl_to(7,40,10u,1)
E F
ctrl_to(7,20,10u,0)

ctrl(1,20)
q C
ctrl_to(1,53,35u,1)

ctrl(1,20)
q C
ctrl_to(1,53,5u,10)

ctrl(7,0) ctrl(1,100)
q C D
ctrl_to(7,40,10u,1) ctrl_to(1,0,10u,1)

ctrl(4,0)
q C D
ctrl_pt(4,40)
E F
ctrl_cto(4,20,15u,1)

ctrl(3,0)
q C
ctrl_cto(3,100,15u,1,0,3)
ctrl(3,0)
q C
ctrl_cto(3,100,15u,1,3,0)
ctrl(3,0)
q C
ctrl_cto(3,100,15u,1,1,1)

C(kp(50) rt=120u kp_to(60))

tempo(100)
c
tempo_to(120)
