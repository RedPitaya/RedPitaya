# -*- coding: utf-8 -*-
import os

from distutils.core import setup
from distutils.command.build import build
from distutils.command.install import install

from PyRedPitaya import __version__

# If we want to overrid the build processe
# For example, to compile de libmonitor and install it
# Install libmonitor only for redpitaya, i.e. when 

build_dir = "monitor/"

def compile_libmonitor():
    cwd = os.getcwd() # get current directory
    try:
        os.chdir(build_dir)
        os.system("make clean")
        os.system("make all")
    finally:
        os.chdir(cwd)

def install_libmonitor(prefix=''):
    cwd = os.getcwd() # get current directory
    try:
        os.chdir(build_dir)
        os.system("make install INSTALL_DIR={prefix}".format(prefix=prefix))
    finally:
        os.chdir(cwd)


class lib_build(build):
    def run(self):
        compile_libmonitor()
        build.run(self)

class lib_install(install):
    def run(self):
        compile_libmonitor()
        install_libmonitor(self.prefix)
#        install.run(self)


cmdclass = {}
cmdclass['lib_build'] = lib_build
cmdclass['lib_install'] = lib_install

setup(name='PyRedPitaya',
      version=__version__,
      description='Python utilities for redpitaya',
      author=u'Pierre Cladé',
      author_email='pierre.clade@upmc.fr',
      packages=['PyRedPitaya', 'PyRedPitaya.enum'],
      cmdclass=cmdclass, 
     )
