FROM cloudwarelabs/xfce4-min:latest
MAINTAINER guodong <gd@tongjo.com>
RUN apt-get update
RUN apt-get install -y libwebp-dev libx11-dev libxdamage-dev libxtst-dev
COPY pulsar /usr/local/bin/pulsar
COPY libwebsockets.so.10 /usr/lib/
COPY pulsar.desktop ~/.config/autostart/
EXPOSE 5678
ENV DISPLAY :0
