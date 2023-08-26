# secure.server.quic


## For Testing

start bind9 daemon with docker and echo_server

```sh
    bash test/prepare.sh
```

 for testing

```sh
    make check
```

for memory checking

```sh
    make checkvalgrind
```

this code started as c implementation, then switched to rust

