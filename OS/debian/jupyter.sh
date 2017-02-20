################################################################################
# Authors:
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

###############################################################################
# install packages
###############################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
# Sigrok
apt-get -y install libsigrok libsigrokdecode sigrok-cli
# OWFS 1-wire library
# NOTE: for now do not install OWFS, and avoid another http/ftp server from running by default
#apt-get -y install owfs python-ow

# Python package manager, Jupyter dependencies
apt-get -y install python3-dev python3-cffi python3-wheel python3-setuptools python3-pip python3-zmq python3-jinja2 python3-pygments python3-six python3-html5lib python3-terminado python3-decorator python3-ptyprocess python3-pexpect python3-simplegeneric python3-wcwidth python3-pickleshare python3-bleach python3-mistune python3-jsonschema
# Python numerical processing and plotting
apt-get -y install python3-numpy python3-scipy python3-pandas
apt-get -y install python3-matplotlib

pip3 install --upgrade pip
pip3 install jupyter
pip3 install ipywidgets
jupyter nbextension enable --py widgetsnbextension

# https://plot.ly/python/ for interactive graphs
pip3 install plotly

# http://bokeh.pydata.org/ interactive visualization library
pip3 install bokeh

# additional Python support for GPIO, LED, PWM, SPI, I2C, MMIO, Serial
# https://pypi.python.org/pypi/python-periphery
pip3 install python-periphery
pip3 install smbus2
pip3 install i2cdev

# UDEV support can be used to search for peripherals loaded using DT overlays
# https://pypi.python.org/pypi/pyudev
# https://pypi.python.org/pypi/pyfdt
pip3 install pyudev pyfdt
EOF_CHROOT

###############################################################################
# create user and add it into groups for HW access rights
###############################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
useradd -m -c "Jupyter notebook user" -s /bin/bash -G xdevcfg,uio,xadc,led,gpio,spi,i2c,eeprom,dialout,dma jupyter
EOF_CHROOT

###############################################################################
# systemd service
###############################################################################

# copy systemd service
install -v -m 664 -o root -D  $OVERLAY/etc/systemd/system/jupyter.service \
                             $ROOT_DIR/etc/systemd/system/jupyter.service

# create configuration directory for users root and jupyter
install -v -m 664 -o root -D  $OVERLAY/home/jupyter/.jupyter/jupyter_notebook_config.py \
                             $ROOT_DIR/root/.jupyter/jupyter_notebook_config.py
# let the owner be root, since the user should not change it easily
install -v -m 664 -o root -D  $OVERLAY/home/jupyter/.jupyter/jupyter_notebook_config.py \
                             $ROOT_DIR/home/jupyter/.jupyter/jupyter_notebook_config.py

chroot $ROOT_DIR <<- EOF_CHROOT
chown -v -R jupyter:jupyter /home/jupyter/.jupyter
EOF_CHROOT

chroot $ROOT_DIR <<- EOF_CHROOT
systemctl enable jupyter
EOF_CHROOT

###############################################################################
# copy/link notebook examples
###############################################################################

mkdir $ROOT_DIR/home/jupyter/WhirlwindTourOfPython
git clone https://github.com/jakevdp/WhirlwindTourOfPython.git $ROOT_DIR/home/jupyter/WhirlwindTourOfPython

ln -s /opt/redpitaya/jupyter/welcome.ipynb $ROOT_DIR/home/jupyter/welcome.ipynb
