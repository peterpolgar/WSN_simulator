# Millisecond resolution WSN simulator
Wireless Sensor Network (WSN) simulator software package including mobility.

Built with the Qt framework.

Acknowledgement to Tommy Hinks for the implemented Poisson-disk sampling algorithm which I used as a sink and node placement algorithm in certain softwares of this repo:

[https://github.com/thinks/poisson-disk-sampling](https://github.com/thinks/poisson-disk-sampling)

## Build from source

### WSN mobility software

After cloning this repo, the WSN mobility software can be compiled by installing Qt, and then building by Qt Creator or running the following command in the WSN_mobility directory:

```cmake.exe --build /target/build/directory --target all```

### The WSN simulation software

The "the_simulation" software can be compiled by running the following command:

```HPC_compile_all_things.bash /path/to/the/the_simulation/```

## Using a prebuilt binary of the WSN mobility software

See the [releases page](https://github.com/peterpolgar/WSN_simulator/releases).
