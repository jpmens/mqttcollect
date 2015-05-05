# collectd-mqtt-sys

This is a plugin for the [Collectd Exec plugin](https://collectd.org/documentation/manpages/collectd-exec.5.shtml).

## influxdb


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

```
LoadPlugin network

<Plugin "network">
  # influxdb
    Server "127.0.0.1" "25826"
</Plugin>
```

## Testing

```
select * from "hippo/mqtt-sys/connections-connections.1m"
select * from "hippo/mqtt-sys/counter-msgs.received"
```

## Requirements

* [collectd](http://collectd.org)
* [InfluxDB](

## Credits

* [uthash](http://troydhanson.github.io/uthash/userguide.html)
