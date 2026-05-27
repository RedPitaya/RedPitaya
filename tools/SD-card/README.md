# Install Phoronix test suite

Install dependancies
```bash
apt-get update
apt-get upgrade
apt-get install php5-cli php5-gd php5-json
apt-get -f install # if last command continue with error
```

Downloading and installing the Phoronix test suite
```bash
wget http://phoronix-test-suite.com/releases/repo/pts.debian/files/phoronix-test-suite_5.8.0_all.deb
dpkg install phoronix-test-suite_5.8.0_all.deb
```

Install IOzone Filesystem Benchmark
```bash
phoronix-test-suite install iozone
```

# Running tests

Run `iozone` test, select: 1 1 3 and enter test descriptions.
```bash
phoronix-test-suite run iozone # select: 1 1 3 and enter test descriptions
```
