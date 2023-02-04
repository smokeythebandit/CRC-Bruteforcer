# crc-bruteforcer

CRC Bruteforcer is a C++ project that can be used to determine the polynomial used in a cyclic redundancy check (CRC) algorithm by brute forcing all possible polynomials and checking if they produce the desired checksum.

## Command Line Options

The following are the command line options available:

### Positional Arguments:

- `file`: File to process
- `checksum`: Checksum we need to match with

### Optional Arguments:

- `-h, --help`: Shows help message and exits
- `-v, --version`: Prints version information and exits
- `--start-polynomal`: Polynomal to start with (default: "00000000")
- `--end-polynomal`: Polynomal to end with (default: "FFFFFFFF")
- `--initial-value`: Initial CRC value (default: "FFFFFFFF")
- `--final-xor`: Initial CRC value (default: "FFFFFFFF")

## Usage

$ ./crc-bruteforcer file checksum [--start-polynomal "00000000"] [--end-polynomal "FFFFFFFF"] [--initial-value "FFFFFFFF"] [--final-xor "FFFFFFFF"]

## Example

To determine the polynomial used in a CRC algorithm that produces the checksum `1234ABCD` for the file `data.bin`, we can run:

$ ./crc-bruteforcer data.bin 1234ABCD

## Building the Project

The project uses [CMake](https://cmake.org/) as its build system. To build the project, follow these steps:

1. Clone the repository to your local machine

$ git clone https://github.com/<username>/crc-bruteforcer.git

2. Change into the cloned repository directory

$ cd crc-bruteforcer

3. Create a build directory
  
$ mkdir build

4. Change into the build directory

$ cd build

5. Run CMake to generate the build files

$ cmake ..

6. Build the project using the generated build files
 
$ make

7. The compiled binary will be located in the build directory and can be run using the instructions in the [Usage](#usage) section.




