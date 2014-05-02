Testing Heartbleed using GNUTLS - Demo
======================================

The files in this repository form a functional demonstration of how GNUTLS can be used to test for heartbleed vulnerabilities.

## Installation

### GNUTLS

First, you need to build my custom version of GNUTLS, which contains some modifications to the internal implementation of heartbeats.  You can find it here:

https://github.com/vote539/gnutls/tree/gnutls_3_2_x

Note that you need to checkout the *gnutls_3_2_x* branch, not the *master* branch.

Once you clone this repo and checkout the correct branch, you will need to build GNUTLS.  Something along the lines of the following should probably work:

	$ make bootstrap
	$ ./configure
	$ make
	$ sudo make install

You will need to have all of the GNUTLS dependencies installed; see *README-alpha* in that repo.  GNUTLS requires recent versions of a lot of major libraries like Automake and Autogen, so I recommend using a recent distribution of Linux to make your life easier.  I was able to compile GNUTLS using the *apt-get* versions of these packages in Ubuntu 14.04 LTS (which was released just a few days ago).

### Demonstration Files

Second, clone this repository.

In the Makefile, change `GNUTLS_DIR = ../gnutls` to point to the path to the root directory of gnutls, which you cloned above.

Finally, executing `make run` in this repository should build and run the demonstration

## Customization

By default, this demonstration will hit the server at "www.cloudflarechallenge.com", which seems to be the default server people are using for testing Heartbleed.  If you want to test a custom server, pass arguments to `make run` like this:

    make run host=example.com ip=10.20.30.40

Go into `heartbleed.c` to change the number of heartbeats being sent to the server and the content of those heartbeats.

## Notes

You can customize the heartbeats that are sent by modifying *heartbleed.c*.

There are two flags you can pass to `send_random_heartbeat` and `send_custom_heartbeat`.  `GNUTLS_HEARTBEAT_BLEED_SMALL` will bleed 16 bytes from the remote host, and `GNUTLS_HEARTBEAT_BLEED_LARGE` will bleed 16 KB from the server.

Running `make run` will recompile the source files and execute the program.

You will be able to see any "bled" bytes of memory on the command line output.

## License

The contents of this repository is released under the X11/MIT license.
