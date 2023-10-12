# Spike Detector Calibrator

**WIP**

Spike Detector Calibrator (sdc) interacts with Open Ephys GUI's zmq-interface plugin. The code at the moment should be pretty self-explanatory, but here are some quick comments:
- sdc accepts two flags, `-p port number` to specify the port for receiving continuous data, `-v` for the output to be verbose.
- sdc only runs on Linux because pthreads are in use.
- The dependencies as of now are *zeromq* and *gsl*.
- Hearbeating is implemented. My plan for data processing is using the running statistics library from *gsl* to update mean and standard deviation estimates on the fly, because running stats uses a one-pass algorithm. With these values we can calculate the absolute deviation for each incoming voltage and pool the array for calculating Median Absolute Deviation.

More on what exactly sdc is supposed to do: [notes](https://cowsay.rip/pong/notes.md).
