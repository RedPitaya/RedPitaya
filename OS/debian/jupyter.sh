################################################################################
# Authors:
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
# Python package manager
apt-get -y install python3-dev python3-pip

pip3 install --upgrade pip
pip3 install setuptools
pip3 install jupyter
EOF_CHROOT

# Running Jupyter
# jupyter notebook --no-browser --ip='*'

