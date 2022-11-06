#!/usr/bin/env python3
#coding: utf-8
from zencad import *
from metric import *

#is_asm = False
is_asm = True
is_sq = False
k_tol = 0.5

def get_bbox(o):
    bb = o.bbox()

    d = bb.xmax - bb.xmin
    x = (d + k_tol*2) / d
    d = bb.ymax - bb.ymin
    y = (d + k_tol*2) / d
    d = bb.zmax - bb.zmin
    z = (d + k_tol*2) / d

    #return bb.shape().scale(1 + k_tol)
    return bb.shape().scaleXYZ(x, y, z)
    #return box(x, y, z, center=True)

def get_case(is_bboxes):
    global case, lid, bread_board, pin_f, pin_m, clemma6, pincon, t18

    ##[ периферийная плата ]########
    brd = (bread_board ^ box(21, 1.62, 53, True).left(2.54/2)).right(2.54/2).rotateX(deg(90))
    obj_p_mod = brd
    cl6 = clemma6.up(1.62/2+0.1).rotateZ(deg(-90)).forw(2.54*7).left(2.54*2)
    obj_p_mod += cl6

    #bb = brd + cl6

    p_mod = nullshape()
    pm = pin_m.back(2.54/2)
    pm = pm.back(2.54 * 9)
    pm20 = pm.right(2.54 * 3)
    for i in range(20):
        p_mod += pm20
        pm20 = pm20.forw(2.54)

    #bb += p_mod
    obj_p_mod += p_mod

    p_mod = nullshape()
    pm7 = pm.left(2.54 * 2)
    for i in range(7):
        p_mod += pm7
        pm7 = pm7.forw(2.54)

    #bb += p_mod
    obj_p_mod += p_mod

    if is_bboxes:
        obj_p_mod = get_bbox(obj_p_mod)

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

    obj_amp += pincon
    if is_bboxes:
        obj_amp = get_bbox(amp)
        obj_amp += get_bbox(p_mod)
        obj_amp += get_bbox(pincon)

    obj_amp = obj_amp.transform(forw(0.4+2.54*5) * left(4.5+2.54*3))

    ##[ TTGO T18v3 ]################
    obj_t18 = t18

    pf = pin_f.moveX(0.45 - 2.54*8).moveY(-6.85).down(2.55)
    p_mod = nullshape()
    for i in range(20):
        p_mod += pf
        pf = pf.moveX(2.54)
    obj_t18 += p_mod

    if is_bboxes:
        obj_t18 = get_bbox(t18)
        obj_t18 += get_bbox(p_mod)

    ##[ соединённые платы ]#########
    obj_stuff = obj_p_mod
    obj_stuff += obj_amp
    obj_stuff += obj_t18
    obj_stuff = obj_stuff.transform(rotateX(deg(-90)) * down(10) * left(11))

    ##[ кнопка ]####################
    btn = metric_screw(30, 1, 10.5, True).down(10.5)
    t = cylinder(r=27.5/2, h=17.5, center=True).down(17.5/2)
    btn += t
    hex = linear_extrude(ngon(r=39.0/2, n=6), (0, 0, 3.8), True).rotateZ(deg(90)).down(3.8)

    btn += hex
    h=4.5
    btn += cylinder(r=34.1/2, h=h, center=True).up(h/2).chamfer(0.3, refs=[point3(0, 0, h)])
    d = 0.05
    btn -= cylinder(r=17.0/2 + 3.0, h=d*2, center=True).up(d + 4.5-d*2)
    btn += cylinder(r=17.0/2, h=d, center=True).up(d/2 + 4.5-d)
    d = 0.1
    btn -= cylinder(r=17.0/2 - 1.0, h=d, center=True).up(d/2 + 4.5-d)

    obj_btn = btn
    d = 0.05
    obj_btn += (cylinder(r=17.0/2 + 3.0, h=d, center=True) - cylinder(r=17.0/2, h=d, center=True)).up(d/2 + 4.5-d)
    
    if is_bboxes:
        if is_sq:
            bb = get_bbox(t)
            bb += get_bbox(cylinder(r=39.0/2, h=3.8, center=True).rotateZ(deg(90)).down(3.8))
        else:
            bb = cylinder(r=27.5/2 + k_tol, h=17.5 + k_tol, center=True).down(17.5/2).rotateZ(deg(90)).down(3.8)
            bb += cylinder(r=39.0/2 + k_tol, h=3.8 + k_tol, center=True).rotateZ(deg(90)).down(3.8)
        obj_btn = bb

    obj_btn = obj_btn.transform(rotateX(deg(-90)) * forw(22) * up(14-10))
    
    ##[ динамик ]###################
    spk = cylinder(r=23/2, h=7, center=True).up(7/2).fillet(2, [(23,23,0)])
    spk += cylinder(r=32/2, h=3.5, center=True).up(3.5/2).fillet(1).up(7)
    spk += cylinder(r=35/2, h=7.5, center=True).up(7.5/2).fillet(2, [(35,35,0)]).up(10.5)
    spk += cylinder(r=41/2, h=3.5, center=True).up(3.5/2).fillet(1).up(18)
    spk -= sphere(41).up(21.5+41-4.5)
    spk += sphere(20/2).up(21.5+20/2-20)
    spk += box(23, 3, 8, True).back((23+3)/2).up(8/2)
    spk = spk.down(1)
    obj_spk = spk

    if is_bboxes:
        if is_sq:
            bb = get_bbox(spk) + box(26, 26, 10, True).up(21.5 + 10/2)
        else:
            #bb = cylinder(r=39/2 + k_tol, h=10, center=True).up(10/2).fillet(1).up(18)
            #bb += cylinder(r=41/2 + k_tol, h=3.5, center=True).up(3.5/2+18).fillet(1).up(18)
            #bb += cylinder(r=35.0/2, h=7.5, center=True).up(7.5/2+10.5)
            bb = cylinder(r=23/2+k_tol, h=7, center=True).up(7/2).fillet(2, [(23,23,0)])
            bb += cylinder(r=32/2+k_tol, h=3.5, center=True).up(3.5/2).fillet(1).up(7)
            #bb += cylinder(r=35/2+k_tol, h=7.5, center=True).up(7.5/2).fillet(2, [(35,35,0)]).up(10.5)
            #bb += cylinder(r=41/2+k_tol, h=3.5, center=True).up(3.5/2).fillet(1).up(18)
            t=2
            bb += cylinder(r=41/2+k_tol, h=11-t, center=True).up((11-t)/2).up(10.5+t)
            bb += cylinder(r=35/2+k_tol, h=12, center=True).up(12/2).up(15)
            bb += box(23+k_tol, 3+k_tol, 8+k_tol, True).back((23+3)/2).up(8/2)
            bb = bb.down(1)

        obj_spk = bb

    obj_spk = obj_spk.transform(rotateX(deg(-90)) * forw(22+1) * down(21.5+2) * right((35+40)/2 - 1))
    
    ##[ дополнительные вырезы ]#####
    #if is_bboxes:
    #obj_spk += box()

    ##[ корпус ]####################
    obj_case = case
    #obj_case += lid
    ################################
    obj_case += obj_stuff
    obj_case += obj_btn
    obj_case += obj_spk
    
    return obj_case


##[ загрузка моделей ]##########
case = from_brep('./res/1590bb_open.brep')
lid = from_brep('./res/1590bb_lid.brep')
bread_board = from_brep('./res/bread-board-4x6cm.brep').up(30).left(20).back(1.62/2)
pin_f = from_brep("./res/pin-f-2.54.brep")
pin_m = from_brep("./res/pin-m-2.54.brep").rotateX(deg(-90)).up(8.4+1.62/2)
clemma6 = from_brep('./res/KF128-2.54-6P.brep')
pincon = from_brep('./res/2-Pin-connector.brep').rotateZ(deg(180)).rotateY(deg(180)).forw(6.3)
t18 = from_brep('./res/ttgo-t-energy-t18-v2.brep').right(5.0).back(20)

d = 5
brick = box(120-d, 32, 95-d, True).moveY(-32/2+4 - 1)
t = brick
brick -= case
brick -= lid

dx=9.3
dy=9.8
x = (120-dx)/2
y = (95-dy)/2
eraser = cylinder(r=3.5, h=35).rotateX(deg(90)).moveY(5)
brick -= eraser.moveX(-x).moveZ(y)
brick -= eraser.moveX(-x).moveZ(-y)
brick -= eraser.moveX(x).moveZ(-y)
brick -= eraser.moveX(x).moveZ(y)

obj_case = get_case(False)
brick -= (get_case(True) + lid)
#h=19.85
h=16.7
obj_case = obj_case.transform(rotateX(deg(90)) * up(10))
disp(obj_case)
#m = (brick - t.moveY(h)).moveZ(-100)
m = (brick - t.moveY(h))
if not is_asm:
    m += (brick - t.moveY(-(29.75-h))).mirrorXZ().mirrorXY().moveZ(100).moveY(-29.75+4)
m = m.transform(rotateX(deg(90)) * up(10))
disp(m, (1, 0.75, 0.5))

if is_asm:
    m = brick - t.moveY(-(29.75-h))
    m = m.transform(rotateX(deg(90)) * up(10))
    disp(m, (0.75, 1, 0.5, 0.5))

lid = lid.transform(rotateX(deg(90)) * up(10))
#disp(lid, (0.5, 0.5, 0.5, 0.75))
show()