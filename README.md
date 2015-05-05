# collectd-mqtt-sys

This is an executable  for the [collectd Exec plugin](https://collectd.org/documentation/manpages/collectd-exec.5.shtml).

An internal hash is loaded with the following data:

* a _metric_ which is sent to collectd
* a _topic_ from MQTT
* a metric _type_ as defined by [types.db](https://github.com/astro/collectd/blob/master/src/types.db)


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
