# Middle Process

**WIP**

`mid` is the middle process that sits between the GUI and dlp: it receives spike events from GUI's zmq interface, interprets them in the context of game logic, and sends requests to dlp accordingly.

To compile:

```
meson setup build
cd build && ninja
```

Available options:

`-p port number` port number used by the publisher-subscriber socket (DATA_PORT in ZMQ INTERFACE), default value 5556.

`-v` enable verbose output.
