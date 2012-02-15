# the logger for Apache2.

## EXPERIMENTAL

currently, this module only output [debug.test,fixed_time,{hello:world}] to localhost fluentd.

## Install

````
sudo apxs2 -a -i -I/home/vagrant/libuv/include -lpthread -lrt -lm -lmsgpack -Wc,-g3 -c mod_log_fluentd.c ../libuv/uv.a -D_FILE_OFFSET_BITS=64
````

also you need to install msgpack for C and libuv. please specify -fPIC flag when you building libuv.

## Todo

So many!


## Configurations (draft)

````
CustomLog fluentd:<default_tag>@<host>[:port] "format"
````

for example:

````
CustomLog fluentd:debug.test@localhost:24224 "%u"
````

this feature hasn't implement yet.

## variable names (draft)

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
