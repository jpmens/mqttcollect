mqtt‐sys(1)							   mqtt‐sys(1)



NAME
       mqtt‐sys − MQTT "$SYS/#"‐subscriber for collectd exec

SYNOPSYS
       mqtt‐sys  [‐h  host ] [‐p port ] [‐C CA‐cert ] [‐u username ] [‐P pass‐
       word ] [‐K psk‐key ] [‐I psk‐identity ] [‐s] [‐N nodename ] [‐f metrics
       ] [‐t topic...  ]


DESCRIPTION
       mqtt‐sys  is  an executable program which is used with collectd(1).  It
       subscribes to the MQTT $SYS/# topic and/or any  number  of  topics  you
       specify, and prints values to stdout for collectd to process in an exec
       plugin block.


   MQTT subscriptions
       By default, mqtt‐sys subscribes to the $SYS/# topic tree of the	speci‐
       fied  MQTT  broker, but any number of topic subscriptions can be set up
       using ‐t.  Whether or not the payload of a received message is actually
       passed to collectd is defined by a metrics map which maps an MQTT topic
       name to a metric name with a particular type (and value).  It  is  thus
       possible  that  mqtt‐sys is subscribed to a topic but doesn’t pass that
       to collectd.


   Metrics map
       The MQTT topic to collectd metrics mapping is configured through a met‐
       rics  file  you	provide.  Lines which start with # and empty lines are
       ignored.  All other lines must have three white‐space separated tokens.
       The  metric  name  which is what is passed to collectd, the type of the
       metric (from types.db), and the MQTT topic name.

       temp.living	  gauge   arduino/temp/celsius
       bROKER.UPTIme	  uptime  $SYS/broker/uptime
       msgs.count	  gauge   $SYS/broker/retained messages/count‘



   influxdb
       Configure InfluxDB to launch the native collect input:

       [input_plugins]

	 # Configure the collectd api
	 [input_plugins.collectd]
	 enabled = true
	 # address = "0.0.0.0" # If not set, is actually set to bind‐address.
	 port = 25826
	 database = "collectd"
	 # types.db can be found in a collectd installation or on github:
	 # https://github.com/collectd/collectd/blob/master/src/types.db
	 typesdb = "/usr/share/collectd/types.db" # The path to the collectd types.db file


   collectd
       Configure collectd to send its metrics to InfluxDB  via	the  _network_
       plugin  which  talks  to  InfluxDB.  (Compare the port numbers here and
       above in InfluxDB.)

       LoadPlugin network

       <Plugin "network">
	 # influxdb
	   Server "127.0.0.1" "25826"
       </Plugin>

       Configure collectd to load our executable  via  the  _exec_  mechanism.
       Specify options as you need them.

       LoadPlugin exec

       <Plugin exec>
	  Exec "mosquitto:mosquitto" "/usr/bin/mqtt‐sys" "‐u" "jjolie" "‐P" "secret" "‐f" "/etc/metrics"
       </Plugin>


   Testing
       To see if whether your values are being passed to InfluxDB:

       select * from "hippo/mqtt‐sys/connections‐connections.1m"
       select * from "hippo/mqtt‐sys/counter‐msgs.received"


OPTIONS
       −C ca‐file
	      specifies the PEM‐encoded CA certificate file to use for TLS.


       −h host
	      is  the hostname or address of the MQTT broker. Unless overriden
	      by ‐N this also sets the collectd nodename.


       −p port
	      gives the TCP port number for the MQTT broker; defaults to 1883.


       −s     use insecure TLS.


       −f metrics
	      is  the  path to the metrics file which defaults to "./metrics".
	      If this file cannot be read, the program exits with a diagnostic
	      message.


       −I psk‐identity
	      sets the identity for PSK.


       −t topic
	      may be specified multiple times and causes mqtt‐sys to subscribe
	      to that topic. Defaults to "$SYS/#" if not specified.


       −u  username
	      for the MQTT connection.


       −P  password
	      for the MQTT connection.


       −K  psk‐key
	      for the MQTT connection.



       −N  nodename
	      Overrides the collectd node name,  which	without  this  option,
	      defaults	to  the  short	local  hostname or the value of the ‐h
	      option


AUTHOR
       Jan‐Piet Mens


SEE ALSO
       collectd(1),	       mosquitto(8)	       uthash(http://troydhan‐
       son.github.io/uthash/userguide.html)




				   May 2015			   mqtt‐sys(1)
