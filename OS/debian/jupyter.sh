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
# Python package manager
apt-get -y install python3-dev python3-cffi python3-wheel python3-setuptools python3-pip python3-zmq python3-jinja2 python3-pygments python3-six python3-html5lib python3-terminado python3-decorator python3-ptyprocess python3-pexpect python3-simplegeneric python3-wcwidth python3-pickleshare python3-bleach python3-mistune python3-jsonschema
apt-get -y install python3-numpy python3-matplotlib

pip3 install --upgrade pip
pip3 install jupyter
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

# create configuration directory
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
# copy notebook examples
###############################################################################

ln -s /opt/redpitaya/jupyter/experiments   $ROOT_DIR/home/jupyter/experiments
ln -s /opt/redpitaya/jupyter/examples      $ROOT_DIR/home/jupyter/examples
ln -s /opt/redpitaya/jupyter/welcome.ipynb $ROOT_DIR/home/jupyter/welcome.ipynb
