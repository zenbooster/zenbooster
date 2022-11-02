#!/usr/bin/env python3
#coding: utf-8
from zencad import *
import zencad.assemble
from metric import *

is_bboxes = True

##[ загрузка моделей ]##########
bread_board = from_brep('./res/bread-board-4x6cm.brep').up(30).left(20).back(1.62/2)
pin_f = from_brep("./res/pin-f-2.54.brep")
pin_m = from_brep("./res/pin-m-2.54.brep").rotateX(deg(-90)).up(8.4+1.62/2)
clemma6 = from_brep('./res/KF128-2.54-6P.brep')

##[ периферийная плата ]########
asm_p_mod = zencad.assemble.unit()
brd = (bread_board ^ box(21, 1.62, 53, True).left(2.54/2)).right(2.54/2).rotateX(deg(90))
asm_p_mod.add(brd).set_color(0.5, 0.65, 0.5)
cl6 = clemma6.up(1.62/2+0.1).rotateZ(deg(-90)).forw(2.54*7).left(2.54*2)
asm_p_mod.add(cl6).set_color(0, 0.5, 0)

bb = brd + cl6

p_mod = nullshape()
pm = pin_m.back(2.54/2)
pm = pm.back(2.54 * 9)
pm20 = pm.right(2.54 * 3)
for i in range(20):
    p_mod += pm20
    pm20 = pm20.forw(2.54)

bb += p_mod
asm_p_mod.add(p_mod).set_color(0, 0, 0)

p_mod = nullshape()
pm7 = pm.left(2.54 * 2)
for i in range(7):
    p_mod += pm7
    pm7 = pm7.forw(2.54)

bb += p_mod
asm_p_mod.add(p_mod).set_color(0, 0, 0)

if is_bboxes:
    asm_p_mod.add(bb.bbox().shape()).set_color(1.0, 1.0, 0.5, 0.75)

asm_p_mod.relocate(rotateZ(deg(-90)) * down(8.4+1.7/2 + 2.5) * left(0.7) * back(-4.3))
##[ плата усилителя ]###########
asm_amp = zencad.assemble.unit()
amp = from_brep('./res/max98357a-adafruit.brep')
asm_amp.add(amp)

pf = pin_f.moveX(2.54*3).moveY(-7.3).down(2.55)
p_mod = pf # nullshape()
for i in range(7):
    p_mod += pf
    pf = pf.moveX(-2.54)
asm_amp.add(p_mod)

pincon = from_brep('./res/2-Pin-connector.brep').rotateZ(deg(180)).rotateY(deg(180)).forw(6.3)
asm_amp.add(pincon)
if is_bboxes:
    asm_amp.add(amp.bbox().shape()).set_color(1.0, 1.0, 0.5, 0.75)
    asm_amp.add(p_mod.bbox().shape()).set_color(1.0, 1.0, 0.5, 0.75)
    asm_amp.add(pincon.bbox().shape()).set_color(1.0, 1.0, 0.5, 0.75)

asm_amp.relocate(forw(0.4+2.54*5) * left(4.5+2.54*3))
##[ TTGO T18v3 ]################
asm_t18 = zencad.assemble.unit()
t18 = from_brep('./res/ttgo-t-energy-t18-v2.brep').right(5.0).back(20)
asm_t18.add(t18)

pf = pin_f.moveX(0.45 - 2.54*8).moveY(-6.85).down(2.55)
p_mod = nullshape()
for i in range(20):
    p_mod += pf
    pf = pf.moveX(2.54)
asm_t18.add(p_mod)

if is_bboxes:
    asm_t18.add(t18.bbox().shape()).set_color(1.0, 1.0, 0.5, 0.75)
    asm_t18.add(p_mod.bbox().shape()).set_color(1.0, 1.0, 0.5, 0.75)
##[ соединённые платы ]#########
asm_stuff = zencad.assemble.unit()
asm_stuff.relocate(rotateX(deg(-90)) * down(10) * left(11))
##[ кнопка ]####################
asm_btn = zencad.assemble.unit()

btn = metric_screw(30, 1, 10.5, True).down(10.5)
btn += cylinder(r=27.5/2, h=17.5).down(17.5)
hex = linear_extrude(ngon(r=39.0/2, n=6), (0, 0, 3.8), True).rotateZ(deg(90))
btn += hex.down(10.5/2)
h=4.5
btn += cylinder(r=34.1/2, h=h).chamfer(0.3, refs=[point3(0, 0, h)])
d = 0.05
btn -= cylinder(r=17.0/2 + 3.0, h=d*2).up(4.5-d*2)
btn += cylinder(r=17.0/2, h=d).up(4.5-d)
d = 0.1
btn -= cylinder(r=17.0/2 - 1.0, h=d).up(4.5-d)

asm_btn.add(btn)
d = 0.05
asm_btn.add((cylinder(r=17.0/2 + 3.0, h=d) - cylinder(r=17.0/2, h=d)).up(4.5-d)).set_color(1.0, 1.0, 1.0)
asm_btn.relocate(rotateX(deg(-90)) * forw(22) * up(14-10))
##[ корпус ]####################
case = from_brep('./res/1590bb_open.brep')
lid = from_brep('./res/1590bb_lid.brep')
asm_case = zencad.assemble.unit()
asm_case.add(case)
asm_case.add(lid).set_color(0.5, 0.5, 0.5, 0.5)
################################
asm_stuff.link(asm_t18)
asm_stuff.link(asm_p_mod)
asm_stuff.link(asm_amp)
asm_case.link(asm_stuff)
asm_case.link(asm_btn)

asm_case.relocate(rotateX(deg(90)) * up(10))
disp(asm_case)
show()