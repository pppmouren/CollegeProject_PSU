[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-c66648af7eb3fe8bc4f294546bfd86ef473780cde1dea487d3c4ff354943c9ae.svg)](https://classroom.github.com/online_ide?assignment_repo_id=10355762&assignment_repo_type=AssignmentRepo)
# Project 2

## Overview

This repository contains code for emulation of a single CPU with the Shortest Remaining Time First scheduling policy and two IO Devices based thread scheduler.

## File Structure

- `interface.{h, c}` - This contains the header file and the source code needed to be implemented by the students.
- `schedule.{h, c}` - This is where all the variables and utility functions should be defined.
- `main.c` - Contains the code to run the built project based on the inputs.

## Building

To build this project on W135 machines. Clone this repository and within this folder run `make` -

```sh
> make
gcc -std=gnu11 main.c scheduler.c interface.c -pthread -lm -o proj2
```

### Debugging with GDB

To enable support for debug symbols, you should run `make debug` -

```sh
> make debug
gcc -g -std=gnu11 main.c scheduler.c interface.c -pthread -lm -o proj2
```

## Execution

After building, the `proj2` executable will be generated in the same folder. You can use that to run the sample inputs
or your custom inputs. This can be done as shown below -

```shell
./proj2 input_file
```

For example to run `sample_input/input_0` we do -

```shell
> ./proj2 sample_input/input_0
main: Hello!
main: number of threads: 5
init_scheduler: start
main: Output file: output/gantt-input_0
main: Bye!
```
