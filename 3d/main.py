#!/usr/bin/env python3
#coding: utf-8

from zencad import *

case = from_brep('./res/1590bb_open.brep').rotateX(deg(90)).up(10)
stuffing = from_brep('./res/ttgo-t-energy-t18-v2.brep')
stuffing += (
    from_brep('./res/max98357a-adafruit.brep')+
    from_brep('./res/2-Pin-connector.brep').up(1.7).rotateZ(deg(180)).forw(6.3)
).up(-5).forw(20.4).left(9.5)

stuffing = stuffing.moveY(-25)

model = case + stuffing
display(model)
show()
