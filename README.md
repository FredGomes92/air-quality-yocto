# Air Quality Embedded Linux Project

This project provides an embedded Linux solution to measure air quality using the Yocto Project (Styhead).

## Features

- Sensor integration for AM2330 (digital temperature and humidity sensor) and Plantover PMS5003 sensor (particles matter sensor)
- Web-app creation for visualization of the data from the sensors

## Getting Started

### Prerequisites

- Linux host machine (recommended: Ubuntu 20.04+)

### Clone the Repository


```
git clone --recursive git@github.com:FredGomes92/air-quality-yocto.git
cd air-quality-yocto
```


### Setup and Build

Run the setup script to initialize and setup bitbake:

```
./setup.sh
```

Edit the `conf/local.conf` with the following machine and distribution info:

```
MACHINE ?= "raspberrypi4"
DISTRO ?= "air-quality"
```

Build: ```bitbake core-image-minimal ```

## Setup

 ![ Set-up - sensors + raspberrypi](images/rpi.jpg)

 All data visualization:

 ![ web-app - all data](images/webapp.jpg)

 Individual data visualization:

<p style="display: flex; gap: 5px;">
  <img src="images/temp.jpg" alt="Temperature Data" style="width: 50%;">
  <img src="images/pms.jpg" alt="PMS Data" style="width: 50%;">
</p>



