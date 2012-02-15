# the logger for Apache2.

## EXPERIMENTAL


## Install

````
sudo apxs2 -a -i -I/home/vagrant/libuv/include -lpthread -lrt -lm -lmsgpack -Wc,-g3 -c mod_log_fluentd.c ../libuv/uv.a -D_FILE_OFFSET_BITS=64
````

also you need to install msgpack for C and libuv. please specify -fPIC flag when you building libuv.

## Todo

So many!
