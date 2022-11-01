#!/usr/bin/env python3
#coding: utf-8

from zencad import *
from metric import *

case = from_brep('./res/1590bb_open.brep').rotateX(deg(90)).up(10)
lid = from_brep('./res/1590bb_lid.brep').rotateX(deg(90)).up(10)
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
p_mod = p_mod.down(8.4+1.7/2 + 2.5)

stuffing += p_mod

btn = metric_screw(30, 1, 10.5, True).down(10.5)
btn += cylinder(r=30/2 - 1, h=15.5).down(15.5)
hex = linear_extrude(ngon(r=40.4/2, n=6), (0, 0, 5), True).rotateZ(deg(90))
btn += hex.down(10.5/2)
h=4.5
btn += cylinder(r=35/2, h=h).chamfer(1, refs=[point3(0, 0, h)])
btn -= cylinder(r=22.9/2, h=0.5).up(4.5-0.5)
btn += cylinder(r=16.5/2, h=0.5).up(4.5-0.5)

btn = btn.forw(21).up(14)
#model = btn + lid
model = btn + case + stuffing.left(5)
display(model)
display(lid, color = (0.5, 0.5, 0.5, 0.5))
show()
