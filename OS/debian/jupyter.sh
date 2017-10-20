################################################################################
# Authors:
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

###############################################################################
# install packages
###############################################################################

# Added by DM; 2017/10/17 to check ROOT_DIR setting
if [ $ROOT_DIR ]; then 
    echo ROOT_DIR is "$ROOT_DIR"
else
    echo Error: ROOT_DIR is not set
    echo exit with error
    exit
fi

chroot $ROOT_DIR <<- EOF_CHROOT
# Sigrok
apt-get -y install libsigrok libsigrokdecode sigrok-cli
# OWFS 1-wire library
# NOTE: for now do not install OWFS, and avoid another http/ftp server from running by default
#apt-get -y install owfs python-ow

# Python package manager, Jupyter dependencies
apt-get -y install python3-dev python3-cffi python3-wheel python3-setuptools python3-pip python3-zmq python3-jinja2 python3-pygments python3-six python3-html5lib python3-terminado python3-decorator python3-ptyprocess python3-pexpect python3-simplegeneric python3-wcwidth python3-pickleshare python3-bleach python3-mistune python3-jsonschema
# update PIP
pip3 install --upgrade pip

# Python numerical processing and plotting
apt-get -y install gfortran libopenblas-dev liblapack-dev python-dev
# APT
apt-get -y install python3-numpy python3-scipy python3-pandas
apt-get -y install python3-matplotlib
# # PIP (there appears to be a bug in how new numpy handles FPGA buffer mapping)
# pip3 install numpy
# pip3 install scipy
# pip3 install pandas
# apt-get -y install libpng-dev libfreetype6-dev
# #pip3 install pycairo
# pip3 install matplotlib

# Jupyter and ipywidgets
pip3 install jupyter
pip3 install ipywidgets
jupyter nbextension enable --system --py widgetsnbextension

# Jupyter declarative widgets
pip3 install jupyter_declarativewidgets
#jupyter declarativewidgets quick-setup --sys-prefix
jupyter declarativewidgets install
jupyter nbextension enable --sys-prefix --py --system declarativewidgets

# # Jupyter dashboards
# pip3 install jupyter_dashboards
# jupyter dashboards quick-setup --sys-prefix
# # TODO: not sure this step is needed
# jupyter nbextension enable --sys-prefix --py --system jupyter_dashboards
# # Jupyter dashboards dundlers
# pip3 install jupyter_dashboards_bundlers
# jupyter bundlerextension enable --sys-prefix --py --system dashboards_bundlers

# Jupyter dashboards server
# TODO: this is a rather large install mostly due to Node.js
# https://github.com/jupyter-incubator/dashboards_server
# TODO: this is disabled for now
#apt-get -y install npm
#npm install -g jupyter-dashboards-server

# http://bokeh.pydata.org/ interactive visualization library
pip3 install bokeh

# additional Python support for GPIO, LED, PWM, SPI, I2C, MMIO, Serial
# https://pypi.python.org/pypi/python-periphery
pip3 install python-periphery
pip3 install smbus2
pip3 install i2cdev

# support for VCD files
pip3 install pyvcd

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

mkdir $ROOT_DIR/home/jupyter/RedPitaya
git clone https://github.com/redpitaya/jupyter.git $ROOT_DIR/home/jupyter/RedPitaya

chroot $ROOT_DIR <<- EOF_CHROOT
pip3 install -e /home/jupyter/RedPitaya
EOF_CHROOT

mkdir $ROOT_DIR/home/jupyter/WhirlwindTourOfPython
git clone https://github.com/jakevdp/WhirlwindTourOfPython.git $ROOT_DIR/home/jupyter/WhirlwindTourOfPython
