# the logger for Apache2.

## EXPERIMENTAL

## Install

````
sudo apxs2 -a -i -I/home/vagrant/libuv/include -lpthread -lrt -lm -lmsgpack -Wc,-g3 -c mod_log_fluentd.c ../libuv/uv.a -D_FILE_OFFSET_BITS=64
````

also you need to install msgpack for C and libuv. please specify -fPIC flag when you building libuv.

## Todo

So many!


## Outputs

this module provides direct logging to fluentd.

````
2012-02-15 03:16:05 +0000 debug.test: ["127.0.0.1"," ","-"," ","-"," ","[18/Feb/2012:02:27:42 +0000]"," \"","GET / HTTP/1.1","\" ","200"," ","44"," \"","-","\" \"","curl/7.21.3 (x86_64-pc-linux-gnu) libcurl/7.21.3 OpenSSL/0.9.8o zlib/1.2.3.4 libidn/1.18","\"","\n"]
````

## Configurations (draft)

````
CustomLog fluentd:<default_tag>@<host>[:port] "format"
````

for example:

````
CustomLog fluentd:debug.test@localhost:24224 "%u"
````

this feature hasn't implement yet. (always connect to 127.0.0.1:24224 and use debug.test tag)

## variable names (draft)

it's very useful to identify log value. currently, it does not support ;(
this feature requires fully implement own logger for apache2. mod_log_fluentd uses mod_log_config's writer dispatch mechanism.

````
remote_addr: %a
server_addr: %A
response_size: %B
cookies.FooBar: %{FooBar}C
processing_time: %d
file_name: %f
remote_host: %h
request_protocol: %H
headers.FooBar: %{FooBar}e
ident_name: %l
request_method: %m
memo.FooBar: %{FooBar}n
response.FooBar: %{FooBar}o
server_port: %p
process_id: %P
worker_id: %{format}P
query_string: %q
first_line: %r
status: %s
request_time: %t
formated_time: %{format}t
processing_time: %T
remote_user: %u
path: %U
server_name: %v
canonical_name: %V
response_status: %X
recived_size: %I (mod_logio)
send_size: %O (mod_logio)
````
