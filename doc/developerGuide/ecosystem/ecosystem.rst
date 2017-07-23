.. ecosystem

###############
Ecosystem Guide
###############

=====================================
Red Pitaya ecosystem and applications
=====================================

Here you will find the sources of various software components of the
Red Pitaya system. The components are mainly contained in dedicated
directories, however, due to the nature of the Xilinx SoC "All 
Programmable" paradigm and the way several components are interrelated,
some components might be spread across many directories or found at
different places one would expect.

+--------------+-------------------------------------------------------------------------------------------------------+
| directories  | contents                                                                                              |
+==============+=======================================================================================================+
| api          | ``librp.so`` API source code                                                                          |
+--------------+-------------------------------------------------------------------------------------------------------+
| api2         | ``librp2.so`` API source code                                                                         |
+--------------+-------------------------------------------------------------------------------------------------------+
| Applications | WEB applications (controller modules & GUI clients)                                                   |
+--------------+-------------------------------------------------------------------------------------------------------+
| apps-free    | WEB application for the old environment (also with controller modules & GUI clients)                  |
+--------------+-------------------------------------------------------------------------------------------------------+
| apps-tools   | WEB interface home page and some system management applications                                       |
+--------------+-------------------------------------------------------------------------------------------------------+
| Bazaar       | Nginx server with dependencies, Bazaar module & application controller module loader                  |
+--------------+-------------------------------------------------------------------------------------------------------+
| fpga         | FPGA design (RTL, bench, simulation and synthesis scripts) SystemVerilog based for newer applications |
+--------------+-------------------------------------------------------------------------------------------------------+
| OS/buildroot | GNU/Linux operating system components                                                                 |
+--------------+-------------------------------------------------------------------------------------------------------+
| patches      | Directory containing patches                                                                          |
+--------------+-------------------------------------------------------------------------------------------------------+
| scpi-server  | SCPI server                                                                                           |
+--------------+-------------------------------------------------------------------------------------------------------+
| Test         | Command line utilities (acquire, generate, ...), tests                                                |
+--------------+-------------------------------------------------------------------------------------------------------+

-------------------
Supported platforms
-------------------

Red Pitaya is developed on Linux (64bit Ubuntu 16.04),
so Linux is also the only platform we support.

---------------------
Software requirements
---------------------

You will need the following to build the Red Pitaya components:

1. Various development packages.

   .. code-block:: shell-session

      # generic dependencies
      sudo apt-get install make curl xz-utils
      # U-Boot build dependencies
      sudo apt-get install libssl-dev device-tree-compiler u-boot-tools
      # secure chroot
      sudo apt-get install schroot
      # QEMU
      sudo apt-get install qemu qemu-user qemu-user-static
      # 32 bit libraries
      sudo apt-get install lib32z1 lib32ncurses5 lib32bz2-1.0 lib32stdc++6

2. Meson Build system (depends on Python 3) is used for some new code.
   It is not required but can be used during development on x86 PC.

   .. code-block:: shell-session

      sudo apt-get install python3 python3-pip
      sudo pip3 install --upgrade pip
      sudo pip3 install meson
      sudo apt-get install ninja-build

3. Xilinx `Vivado 2017.1 <http://www.xilinx.com/support/download.html>`_ FPGA development tools.
   The SDK (bare metal toolchain) must also be installed, be careful during the install process to select it.
   Preferably use the default install location.

4. Missing ``gmake`` path

   Vivado requires a ``gmake`` executable which does not exist on Ubuntu. It is necessary to create a symbolic link to the regular ``make`` executable.

   .. code-block:: shell-session

      $ sudo ln -s /usr/bin/make /usr/bin/gmake

=============
Build process
=============

Go to your preferred development directory and clone the Red Pitaya repository from GitHub.
The choice of specific branches or tags is up to the user.

.. code-block:: shell-session

   git clone https://github.com/RedPitaya/RedPitaya.git
   cd RedPitaya

An example script ``settings.sh`` is provided for setting all necessary environment variables.
The script assumes some default tool install paths, so it might need editing if install paths other than the ones described above were used.

.. code-block:: shell-session

   $ . settings.sh

Prepare a download cache for various source tarballs.
This is an optional step which will speedup the build process by avoiding downloads for all but the first build.
There is a default cache path defined in the ``settings.sh`` script, you can edit it and avoid a rebuild the next time.

.. code-block:: shell-session

   mkdir -p dl
   export DL=$PWD/dl

Download the ARM Ubuntu root environment (usually the latest) from Red Pitaya download servers.
You can also create your own root environment following instructions in :ref:`OS image build instructions <os>`.
Correct file permissions are required for ``schroot`` to work properly.

.. code-block:: shell-session

   wget http://downloads.redpitaya.com/downloads/redpitaya_ubuntu_12-48-45_22-maj-2017.tar.gz
   sudo chown root:root redpitaya_ubuntu_12-48-45_22-maj-2017.tar.gz
   sudo chmod 664 redpitaya_ubuntu_12-48-45_22-maj-2017.tar.gz

Create schroot configuration file ``/etc/schroot/chroot.d/red-pitaya-ubuntu.conf``.
Replace the tarball path stub with the absolute path of the previously downloaded image.
Replace user names with a comma separeted list of users whom should be able to compile Red Pitaya.

.. code-block:: none

   [red-pitaya-ubuntu]
   description=Red Pitaya Debian/Ubuntu OS image
   type=file
   file=absolute-path-to-red-pitaya-ubuntu.tar.gz
   users=comma-seperated-list-of-users-with-access-permissions
   root-users=comma-seperated-list-of-users-with-root-access-permissions
   root-groups=root
   profile=desktop
   personality=linux
   preserve-environment=true

To build everything a few ``make`` steps are required.

.. code-block:: shell-session

   make -f Makefile.x86
   schroot -c red-pitaya-ubuntu <<- EOL_CHROOT
   make
   EOL_CHROOT
   make -f Makefile.x86 zip

To get an itteractive ARM shell do.

.. code-block:: shell-session

   schroot -c red-pitaya-ubuntu

=======================
Partial rebuild process
=======================

The next components can be built separately.

* FPGA + device tree
* u-Boot
* Linux kernel
* Debian/Ubuntu OS
* API
* SCPI server
* free applications

-----------
Base system
-----------

Here *base system* represents everything before Linux user space.

To be able to compile FPGA and cross compile *base system* software, it is necessary to setup the Vivado FPGA tools and ARM SDK.


.. code-block:: shell-session

   $ . settings.sh

On some systems (including Ubuntu 16.04) the library setup provided by Vivado conflicts with default system libraries.
To avoid this, disable library overrides specified by Vivado.


.. code-block:: shell-session

   $ export LD_LIBRARY_PATH=""

After building the base system it can be installed into the directory later used to create the FAT filesystem compressed image.


.. code-block:: shell-session

   $ make -f Makefile.x86 install

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FPGA and device tree sources
~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. code-block:: shell-session

   $ make -f Makefile.x86 fpga

Detailed instructions are provided for `building the FPGA <fpga/README.md#build-process>`_
including some `device tree details <fpga/README.md#device-tree>`_.

--------------------------------------
Device Tree compiler + overlay patches
--------------------------------------

Download the Device Tree compiler with overlay patches from Pantelis Antoniou.
Compile and install it.
Otherwise a binary is available in ``tools/dtc``.

.. code-block:: shell-session

   $ sudo apt-get install flex bison
   $ git clone git@github.com:pantoniou/dtc.git
   $ cd dtc
   $ git checkout overlays
   $ make
   $ sudo make install PREFIX=/usr

~~~~~~
U-boot
~~~~~~

To build the U-Boot binary and boot scripts (used to select between booting into Buildroot or Debian/Ubuntu):

.. code-block:: shell-session

   make -f Makefile.x86 u-boot

The build process downloads the Xilinx version of U-Boot sources from Github, applies patches and starts the build process.
Patches are available in the ``patches/`` directory.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Linux kernel and device tree binaries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To build a Linux image:

.. code-block:: shell-session

   make -f Makefile.x86 linux
   make -f Makefile.x86 linux-install
   make -f Makefile.x86 devicetree
   make -f Makefile.x86 devicetree-install

The build process downloads the Xilinx version of Linux sources from Github, applies patches and starts the build process.
Patches are available in the ``patches/`` directory.

~~~~~~~~~
Boot file
~~~~~~~~~

The created boot file contains FSBL, FPGA bitstream and U-Boot binary.

.. code-block:: shell-session

   make -f Makefile.x86 boot

----------------
Linux user space
----------------

~~~~~~~~~~~~~~~~
Debian/Ubuntu OS
~~~~~~~~~~~~~~~~

`Debian/Ubuntu OS instructions <OS/debian/README.md>`_ are detailed elsewhere.

~~~
API
~~~

To compile the API run:

.. code-block:: shell-session

   make api

The output of this process is the Red Pitaya ``librp.so`` library in ``api/lib`` directory.
The header file for the API is ``redpitaya/rp.h`` and can be found in ``api/includes``.
You can install it on Red Pitaya by copying it there:

.. code-block:: shell-session

   scp api/lib/librp.so root@192.168.0.100:/opt/redpitaya/lib/

~~~~~~~~~~~
SCPI server
~~~~~~~~~~~

Scpi server README can be found `here <scpi-server/README.md>`_.

To compile the server run:

.. code-block:: shell-session

   make api

The compiled executable is ``scpi-server/scpi-server``.
You can install it on Red Pitaya by copying it there:

.. code-block:: shell-session

   scp scpi-server/scpi-server root@192.168.0.100:/opt/redpitaya/bin/

~~~~~~~~~~~~~~~~~
Free applications
~~~~~~~~~~~~~~~~~

To build free applications, follow the instructions given at `<apps-free/README.md>`_ file.
