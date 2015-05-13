NAME
====

mqttcollect - MQTT-based Exec-plugin for collectd

SYNOPSIS
========

mqttcollect [-v ] [-f *file*]

DESCRIPTION
===========

*mqttcollect* is an executable program which is used with collectd(1).
It subscribes to any number of MQTT topics you specify, and prints
values to stdout for collectd to process in an exec plugin block.

    PUTVAL tiggr/mqtt‐sys/gauge‐clients.inactive 1430914033:0.00

*collectd* launches *mqttcollect* which connects to the configured MQTT
broker, subscribes and waits for publishes to subscribed topics in an
endless loop. If an error occurs or the program exits for whichever
reason, *collectd* will restart and log the reason in its log file.

OPTIONS
=======

*mqttcollect* understands the following options.

-f *file*
:   Specify an ini-type configuration file (see below), which defaults
    to `/usr/local/etc/mqttcollect.ini`.
-v
:   Verbose.

CONFIGURATION
=============

*mqttcollect* requires a configuration file to operate. This ini-type
file must have a `[defaults]` section in which general program
parameters are configured, and it will have any number of additional
sections specifying the MQTT topics it is to subscribe to.

Within a *topic* section, metrics collected by *collectd* are specified.

    [defaults]
    host = localhost
    port = 1883

    ; (1) subscribe to a wildcard and produce three metrics per subscription.
    ; the metric names are interpolated with `tid' from the JSON message
    ; payload, and the values of each metric are obtained from the
    ; JSON element behind the `<'

    [owntracks/+/+]
    gauge = vehicle/{tid}/speed<vel
    gauge = vehicle/{tid}/altitude<alt
    counter = vehicle/{tid}/odometer<trip

    ; (2) subscribe to one topic and rename the metric

    [$SYS/broker/clients/inactive]
    gauge = clients.inactive

    ; (3) subscribe to one topic and KEEP its name

    [$SYS/broker/load/messages/received/1min]
    gauge = *

Example `1` is complex. *mqttcollect* will subscribe to the wildcarded
`owntracks/+/+` topic, and for each message received on that topic, will
produce three metrics. The special character `<` in the line indicates
the MQTT message payload is expected to be JSON. Each of the metric
names will have the JSON element `tid` from the payload interpolated
into their names, and the actual value of the metric will be obtained
(`<`) from the specified JSON element (`vel`, `alt`, and `trip`
respectively). Using this configuration, *mqttcollect* could produce the
following three metrics for *collectd*:

    PUTVAL tiggr/mqttcollect/gauge-vehicle/BB/speed 1431535440:2.00
    PUTVAL tiggr/mqttcollect/gauge-vehicle/BB/altitude 1431535440:48.00
    PUTVAL tiggr/mqttcollect/counter-vehicle/BB/odometer 1431535440:8246531.00

In example `2`, the program will subscribe to a single topic, and will
produce a metric renamed to `clients.inactive`.

    PUTVAL tiggr/mqttcollect/gauge-clients.inactive 1431535434:1.00

Example `3` subscribes to the single topic and does *not* rename the
metric:

    PUTVAL tiggr/mqttcollect/gauge-$SYS/broker/load/messages/received/1min 1431535557:61.47

INFLUXDB
========

As an example, we show how to configure InfluxDB to accept values from
*collectd* via the latter’s network plugin. Configure InfluxDB to launch
the native *collectd* input:

    [input_plugins]

      [input_plugins.collectd]
      enabled = true
      # address = "0.0.0.0" # defaults to bind‐address.
      port = 25826
      database = "collectd"
      # https://github.com/collectd/collectd/blob/master/src/types.db
      typesdb = "/usr/share/collectd/types.db"

COLLECTD
========

Configure *collectd* to send its metrics to InfluxDB via the network
plugin which talks to InfluxDB. (Compare the port numbers here and above
in InfluxDB.)

    LoadPlugin network

    <Plugin "network">
       # influxdb
       Server "127.0.0.1" "25826"
    </Plugin>

Configure *collectd* to load our executable *mqttcollect* via its exec
mechanism. Specify *mqttcollect*'s options as individual strings in the
`Exec` invocation.

    LoadPlugin exec

    <Plugin exec>
       Exec "mosquitto:mosquitto" "/usr/bin/mqttcollect" "‐f" "/etc/my.ini"
    </Plugin>

BUGS
====

Yes.

AVAILABILITY
============

<https://github.com/jpmens/mqttcollect>

CREDITS
=======

-   This program uses *libmosquitto*, a library provided by the
    Mosquitto project <http://mosquitto.org> as well as some of the
    excellent include files provided by
    <http://troydhanson.github.io/uthash>

INSTALLATION
============

-   Obtain the source code for *mqttcollect*, adjust the `Makefile` and
    run `make`.

SEE ALSO
========

-   `collectd`(1).
-   <https://github.com/jpmens/mqttwarn>

AUTHOR
======

Jan-Piet Mens <http://jpmens.net>

