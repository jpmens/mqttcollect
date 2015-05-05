# collectd-mqtt-sys

This is an executable  for the [collectd Exec plugin](https://collectd.org/documentation/manpages/collectd-exec.5.shtml).

An internal hash is loaded with the following data:

* a _metric_ which is sent to collectd
* a _topic_ from MQTT
* a metric _type_ as defined by [types.db](https://github.com/astro/collectd/blob/master/src/types.db)

Configuration of the values you need is done in the `metrics.h` file; specify NULL as the name of the metric if you don't want it sent to InfluxDB.

```c
/*
 * The table of metric names we want to process.
 * Each metric has three values:
 *
 * 1. METRIC name which is what collectd gets. NULL if you don't want this topic.
 * 2. TYPE from the collectd types.db (left column)
 * 3. TOPIC as read from your MQTT broker
 *
 * This must be valid C source code.
 */

  { NULL,               "gauge",        "$SYS/broker/version"   },
  { NULL,               "gauge",        "$SYS/broker/timestamp" },
  { "broker.uptime",    "uptime",       "$SYS/broker/uptime"                            },
  { NULL,               "gauge",        "$SYS/broker/clients/total"                     },
  { NULL,               "gauge",        "$SYS/broker/clients/inactive"                  },
  { "clients.active",   "gauge",        "$SYS/broker/clients/active"                    },
```

After building _mqtt-sys_, point it at your Mosquitto (or other broker with `$SYS/` support) and, on the command-line you should see something like this:

```
PUTVAL tiggr/mqtt-sys/uptime-broker.uptime 1430850265:362164.00
PUTVAL tiggr/mqtt-sys/gauge-clients.active 1430850265:2.00
PUTVAL tiggr/mqtt-sys/gauge-heap.max 1430850265:48656.00
...
```


## influxdb

Configure InfluxDB to launch the native collect input:

```
[input_plugins]

  # Configure the collectd api
  [input_plugins.collectd]
  enabled = true
  # address = "0.0.0.0" # If not set, is actually set to bind-address.
  port = 25826
  database = "collectd"
  # types.db can be found in a collectd installation or on github:
  # https://github.com/collectd/collectd/blob/master/src/types.db
  typesdb = "/usr/share/collectd/types.db" # The path to the collectd types.db file
```
## collectd

Configure collectd to send its metrics to InfluxDB via the _network_ plugin which talks to InfluxDB. (Compare the port numbers here and above in InfluxDB.)

```
LoadPlugin network

<Plugin "network">
  # influxdb
    Server "127.0.0.1" "25826"
</Plugin>
```

Configure collectd to load our executable via the _exec_ mechanism. Specify options as you need them.

```
LoadPlugin exec

<Plugin exec>
   Exec "mosquitto:mosquitto" "/usr/bin/mqtt-sys" "-u" "jjolie" "-P" "secret"
</Plugin>
```

## Testing

```
select * from "hippo/mqtt-sys/connections-connections.1m"
select * from "hippo/mqtt-sys/counter-msgs.received"
```

## Requirements

* [collectd](http://collectd.org)
* [InfluxDB](http://influxdb.com)

## Credits

* [uthash](http://troydhanson.github.io/uthash/userguide.html)
