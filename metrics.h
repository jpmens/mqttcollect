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

  { "temp.living",	"gauge",	"arduino/temp/celsius"	},

  { NULL,		"gauge",	"$SYS/broker/version"	},
  { NULL,		"gauge",	"$SYS/broker/timestamp"	},
  { "broker.uptime",	"uptime",	"$SYS/broker/uptime"				},
  { NULL,		"gauge",	"$SYS/broker/clients/total"			},
  { "clients.inactive",	"gauge",	"$SYS/broker/clients/inactive"			},
  { "clients.active",	"gauge",	"$SYS/broker/clients/active"			},
  { "clients.connected","gauge",	"$SYS/broker/clients/connected"			},
  { "clients.disconnected","gauge",	"$SYS/broker/clients/disconnected"			},
  { NULL,		"gauge",	"$SYS/broker/clients/maximum"			},
  { NULL,		"gauge",	"$SYS/broker/clients/expired"			},
  { "msgs.stored",	"gauge",	"$SYS/broker/messages/stored"			},
  { NULL,		"gauge",	"$SYS/broker/messages/received"			},
  { NULL,		"gauge",	"$SYS/broker/messages/sent"			},
  { NULL,		"gauge",	"$SYS/broker/subscriptions/count"		},
  { NULL,		"gauge",	"$SYS/broker/retained messages/count"		},
  { NULL,		"gauge",	"$SYS/broker/heap/current"			},
  { "heap.max",		"gauge",	"$SYS/broker/heap/maximum"			},
  { NULL,		"gauge",	"$SYS/broker/publish/messages/dropped"		},
  { "msgs.received",	"gauge",	"$SYS/broker/publish/messages/received"		},
  { NULL,		"gauge",	"$SYS/broker/publish/messages/sent"		},
  { NULL,		"gauge",	"$SYS/broker/publish/bytes/received"		},
  { NULL,		"gauge",	"$SYS/broker/publish/bytes/sent"		},
  { NULL,		"gauge",	"$SYS/broker/bytes/received"			},
  { NULL,		"gauge",	"$SYS/broker/bytes/sent"			},
  { NULL,		"gauge",	"$SYS/broker/load/messages/received/1min"	},
  { "broker.msgs.rec.5m",	"gauge",	"$SYS/broker/load/messages/received/5min"	},
  { NULL,		"gauge",	"$SYS/broker/load/messages/received/15min"	},
  { NULL,		"gauge",	"$SYS/broker/load/messages/sent/1min"		},
  { NULL,		"gauge",	"$SYS/broker/load/messages/sent/5min"		},
  { NULL,		"gauge",	"$SYS/broker/load/messages/sent/15min"		},
  { NULL,		"gauge",	"$SYS/broker/load/bytes/received/1min"		},
  { NULL,		"gauge",	"$SYS/broker/load/bytes/received/5min"		},
  { NULL,		"gauge",	"$SYS/broker/load/bytes/received/15min"		},
  { NULL,		"gauge",	"$SYS/broker/load/bytes/sent/1min"		},
  { NULL,		"gauge",	"$SYS/broker/load/bytes/sent/5min"		},
  { NULL,		"gauge",	"$SYS/broker/load/bytes/sent/15min"		},
  { NULL,		"gauge",	"$SYS/broker/load/sockets/1min"			},
  { NULL,		"gauge",	"$SYS/broker/load/sockets/5min"			},
  { NULL,		"gauge",	"$SYS/broker/load/sockets/15min"		},
  { "connections.1m",	"gauge",	"$SYS/broker/load/connections/1min"		},
  { NULL,		"gauge",	"$SYS/broker/load/connections/5min"		},
  { NULL,		"gauge",	"$SYS/broker/load/connections/15min"		},
  { NULL,		"gauge",	"$SYS/broker/load/publish/received/1min"	},
  { NULL,		"gauge",	"$SYS/broker/load/publish/received/5min"	},
  { NULL,		"gauge",	"$SYS/broker/load/publish/received/15min"	},
  { NULL,		"gauge",	"$SYS/broker/load/publish/sent/1min"		},
  { NULL,		"gauge",	"$SYS/broker/load/publish/sent/5min"		},
  { NULL,		"gauge",	"$SYS/broker/load/publish/sent/15min"		},
