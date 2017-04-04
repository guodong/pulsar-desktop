FROM cloudwarelabs/xorg:latest
MAINTAINER guodong <gd@tongjo.com>
RUN apt-get update
RUN apt-get install -y gcc git libwebp-dev libwebsockets-dev libx11-dev libxdamage-dev libxtst-dev
RUN cd /usr/src && git clone https://github.com/guodong/pulsar-desktop.git && cd pulsar-desktop
RUN build.sh
EXPOSE 5678
CMD pulsar
