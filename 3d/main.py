#!/usr/bin/env python3
#coding: utf-8

from zencad import *

case = from_brep('./res/1590bb_open.brep').rotateX(deg(90)).up(10)
bread_board = from_brep('./res/bread-board-4x6cm.brep').up(30).left(20).back(1.62/2)
pin_f = from_brep("./res/pin-f-2.54.brep")
pin_m = from_brep("./res/pin-m-2.54.brep").rotateX(deg(-90)).up(8.4+1.62/2)
clemma2 = from_brep('./res/KF128-2.54-6P.brep')
stuffing = from_brep('./res/ttgo-t-energy-t18-v2.brep')
stuffing += (
    from_brep('./res/max98357a-adafruit.brep')+
    from_brep('./res/2-Pin-connector.brep').rotateZ(deg(180)).rotateY(deg(180)).forw(6.3)
).forw(20.4+2.54*5).left(9.5+2.54*3)


pf20 = pin_f.moveY(29/2-2.54/2).moveX(0.6 - 10*2.54).down(2.5)
pf7 = pf20.forw(5*2.54)

for i in range(20):
    stuffing += pf20
    pf20 = pf20.moveX(2.54)

for i in range(7):
    stuffing += pf7
    pf7 = pf7.moveX(2.54)

stuffing = stuffing.moveY(-25)

p_mod = bread_board ^ box(21, 1.62, 53, True).left(2.54/2) # case + stuffing
p_mod = p_mod.right(2.54/2)
p_mod = p_mod.rotateX(deg(90))
pm = pin_m.back(2.54/2)
pm = pm.back(2.54 * 9)
pm20 = pm.right(2.54 * 3)
pm7 = pm.left(2.54 * 2)

for i in range(20):
    p_mod += pm20
    pm20 = pm20.forw(2.54)

for i in range(7):
    p_mod += pm7
    pm7 = pm7.forw(2.54)

clemma2 = clemma2.up(1.62/2+0.1)
clemma2 = clemma2.rotateZ(deg(-90))
p_mod += clemma2.forw(2.54*7).left(2.54*2)
p_mod = p_mod.rotateZ(deg(-90))
p_mod = p_mod.left(0.7)
p_mod = p_mod.back(4.3)
#p_mod = p_mod.down(8.4+2.5)
p_mod = p_mod.down(8.4+1.7/2 + 2.5)

model = p_mod + case + stuffing
display(model)
show()
