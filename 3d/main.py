#!/usr/bin/env python3
#coding: utf-8
from zencad import *
from metric import *

#is_bboxes = False
is_bboxes = True

##[ загрузка моделей ]##########
case = from_brep('./res/1590bb_open.brep')
lid = from_brep('./res/1590bb_lid.brep')
bread_board = from_brep('./res/bread-board-4x6cm.brep').up(30).left(20).back(1.62/2)
pin_f = from_brep("./res/pin-f-2.54.brep")
pin_m = from_brep("./res/pin-m-2.54.brep").rotateX(deg(-90)).up(8.4+1.62/2)
clemma6 = from_brep('./res/KF128-2.54-6P.brep')

#d = 5.5
d = 0
brick = box(120-d, 30, 95-d, True).moveY(-(30 - 6)/2)
##[ периферийная плата ]########
brd = (bread_board ^ box(21, 1.62, 53, True).left(2.54/2)).right(2.54/2).rotateX(deg(90))
obj_p_mod = brd
cl6 = clemma6.up(1.62/2+0.1).rotateZ(deg(-90)).forw(2.54*7).left(2.54*2)
obj_p_mod += cl6

bb = brd + cl6

p_mod = nullshape()
pm = pin_m.back(2.54/2)
pm = pm.back(2.54 * 9)
pm20 = pm.right(2.54 * 3)
for i in range(20):
    p_mod += pm20
    pm20 = pm20.forw(2.54)

bb += p_mod
obj_p_mod += p_mod

p_mod = nullshape()
pm7 = pm.left(2.54 * 2)
for i in range(7):
    p_mod += pm7
    pm7 = pm7.forw(2.54)

bb += p_mod
obj_p_mod += p_mod

if is_bboxes:
    obj_p_mod += bb.bbox().shape()

obj_p_mod = obj_p_mod.transform(rotateZ(deg(-90)) * down(8.4+1.7/2 + 2.5) * left(0.7) * back(-4.3))

##[ плата усилителя ]###########
amp = from_brep('./res/max98357a-adafruit.brep')
obj_amp = amp

pf = pin_f.moveX(2.54*3).moveY(-7.3).down(2.55)
p_mod = nullshape()
for i in range(7):
    p_mod += pf
    pf = pf.moveX(-2.54)
obj_amp += p_mod

pincon = from_brep('./res/2-Pin-connector.brep').rotateZ(deg(180)).rotateY(deg(180)).forw(6.3)
obj_amp += pincon
if is_bboxes:
    obj_amp += amp.bbox().shape()
    obj_amp += p_mod.bbox().shape()
    obj_amp += pincon.bbox().shape()

obj_amp = obj_amp.transform(forw(0.4+2.54*5) * left(4.5+2.54*3))

##[ TTGO T18v3 ]################
t18 = from_brep('./res/ttgo-t-energy-t18-v2.brep').right(5.0).back(20)
obj_t18 = t18

pf = pin_f.moveX(0.45 - 2.54*8).moveY(-6.85).down(2.55)
p_mod = nullshape()
for i in range(20):
    p_mod += pf
    pf = pf.moveX(2.54)
obj_t18 += p_mod

if is_bboxes:
    obj_t18 += t18.bbox().shape()
    obj_t18 += p_mod.bbox().shape()

##[ соединённые платы ]#########
obj_stuff = obj_p_mod
obj_stuff += obj_amp
obj_stuff += obj_t18
obj_stuff = obj_stuff.transform(rotateX(deg(-90)) * down(10) * left(11))

##[ кнопка ]####################
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

obj_btn = btn
d = 0.05
obj_btn += (cylinder(r=17.0/2 + 3.0, h=d) - cylinder(r=17.0/2, h=d)).up(4.5-d)

if is_bboxes:
    obj_btn += cylinder(r=(btn.bbox().xmax - btn.bbox().xmin)/2, h=22).mirrorXY()

obj_btn = obj_btn.transform(rotateX(deg(-90)) * forw(22) * up(14-10))

##[ корпус ]####################
t = brick
obj_case = case
#obj_case += lid
brick -= case
brick -= lid
################################
obj_case += obj_stuff
obj_case += obj_btn

brick -= obj_case
brick -= t.moveY(14)
obj_case += brick.right(150)
obj_case = obj_case.transform(rotateX(deg(90)) * up(10))
disp(obj_case)
show()