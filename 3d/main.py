#!/usr/bin/env python3
#coding: utf-8

from zencad import *

model = from_brep('./res/ttgo-t-energy-t18-v2.brep')
model += (
    from_brep('./res/max98357a-adafruit.brep')+
    from_brep('./res/2-Pin-connector.brep').up(1.7).rotateZ(deg(180)).forw(6.3)
).up(-5).forw(20.4).left(9.5)

display(model)
show()
