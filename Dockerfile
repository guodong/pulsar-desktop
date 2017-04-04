FROM cloudwarelabs/xorg:latest
MAINTAINER guodong <gd@tongjo.com>
RUN apt-get update
RUN apt-get install -y gcc git libwebp-dev libwebsockets-dev libx11-dev libxdamage-dev libxtst-dev
COPY pulsar /usr/local/bin/pulsar
COPY start.sh /usr/local/bin/start.sh
EXPOSE 5678
ENV DISPLAY :0
CMD start.sh
