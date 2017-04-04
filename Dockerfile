FROM cloudwarelabs/xorg:latest
MAINTAINER guodong <gd@tongjo.com>
RUN apt-get update
RUN apt-get install -y libwebp-dev libwebsockets-dev libx11-dev libxdamage-dev libxtst-dev dictionaries-common
RUN /usr/share/debconf/fix_db.pl && dpkg-reconfigure dictionaries-common
RUN apt-get install -y gnome-themes-standard xfce4
RUN sed -i '2s/.*/gtk-theme-name = Adwaita/' /etc/gtk-3.0/settings.ini
COPY pulsar /usr/local/bin/pulsar
COPY start.sh /usr/local/bin/start.sh
EXPOSE 5678
ENV DISPLAY :0
CMD start.sh
