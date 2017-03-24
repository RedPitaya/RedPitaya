# Always prefer setuptools over distutils
from setuptools import setup, find_packages
# To use a consistent encoding
from codecs import open
from os import path

here = path.abspath(path.dirname(__file__))

# Get the long description from the README file
with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='redpitaya',

    # Versions should comply with PEP440.  For a discussion on single-sourcing
    # the version across setup.py and the project code, see
    # https://packaging.python.org/en/latest/single_source_version.html
    version='0.98',

    description='Red Pitaya Python drivers and Jupyter applications',
    long_description=long_description,

    # The project's main homepage.
    url='https://github.com/RedPitaya/RedPitaya',

    # Author details
    author='Red Pitaya',
    author_email='info@redpitaya.com',

    # Choose your license
    license='BSD',

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 3 - Alpha',

        'Intended Audience :: Developers',
        'Topic :: Software Development :: Drivers',

        'License :: OSI Approved :: MIT License',

        'Programming Language :: Python :: 3.5',
    ],

    keywords='oscilloscope generator',
    packages=['redpitaya'],
    install_requires=['numpy', 'scipy', 'bokeh', 'IPython', 'ipywidgets'],
    package_data={
        'redpitaya': ['drv/mercury.dts'],
    },
)
