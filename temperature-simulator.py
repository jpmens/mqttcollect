#!/usr/bin/env python
# -*- coding: utf-8 -*-

import paho.mqtt.publish as mqtt
import random
import json
import time

topic = 'arduino/temp/002'

while True:
    celsius = float("%.2f" % (random.random() * 40))

    data = {
        'room'       : 'kitchen',
        'celsius'    : celsius,
        'fahrenheit' : float("%.2f" % (9.0 / 5.0 * celsius + 32)),
    }


    payload = json.dumps(data)

    print topic, payload
    mqtt.single(topic, payload, retain=True)

    time.sleep(1)
