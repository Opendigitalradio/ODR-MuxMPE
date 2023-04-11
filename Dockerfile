# Build odr-audioenc
FROM ubuntu:22.04 AS builder
ARG  URL_BASE=https://github.com/Opendigitalradio
ARG  SOFTWARE=ODR-MuxMPE/archive/refs/tags
ARG  VERSION=v1.0.0
ENV  DEBIAN_FRONTEND=noninteractive

ENV LANG en_US.UTF-8  
ENV LANGUAGE en_US:en  
ENV LC_ALL en_US.UTF-8

## Update system
RUN  apt-get update \
     && apt-get upgrade --yes

## Install build packages
RUN  apt-get install --yes \
          autoconf \
          build-essential \
          curl \
          pkg-config \
          git \
          cmake

## Install OatPP
RUN git clone --depth=1 https://github.com/oatpp/oatpp \
&& cd oatpp \
&& mkdir build \
&& cd build \
&& cmake -DOATPP_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release .. \
&& make install -j 6

RUN git clone --depth=1 https://github.com/oatpp/oatpp-swagger \
&& cd oatpp-swagger \
&& mkdir build \
&& cd build \
&& cmake -DOATPP_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release .. \
&& make install -j 6

RUN apt-get --no-install-recommends -y install \
  g++ dpkg-dev git doxygen tclsh pkg-config cmake libssl-dev build-essential ca-certificates dos2unix graphviz curl locales libcppunit-dev libcurl4 libcurl4-openssl-dev git-lfs g++ tar zip doxygen graphviz linux-libc-dev libedit-dev libusb-1.0-0-dev dpkg-dev python3 default-jdk libsrt-openssl-dev libsrt1.4-openssl && \
  update-alternatives --install /usr/bin/python python /usr/bin/python3 2 && \
  sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen && locale-gen


#RUN git clone https://github.com/Haivision/srt.git
#WORKDIR srt
#RUN ./configure && make -j8 && make install

RUN git clone https://github.com/tsduck/tsduck.git && cd tsduck && \
make -j10 install NOPCSC=1 NOTEST=1 NODEKTEC=1 NOHIDES=1 NOVATEK=1 SYSPREFIX=/usr/local

#Build
RUN apt-get --no-install-recommends -y install libboost-system*
COPY . /build
WORKDIR /build
RUN  ./bootstrap.sh && ./configure && make -j8 && make install


# Build the final image
FROM ubuntu:22.04
ENV  DEBIAN_FRONTEND=noninteractive
## Update system
RUN  apt-get update \
     && apt-get upgrade --yes

## Copy objects built in the builder phase
COPY --from=builder /usr/local/bin/ /usr/local/bin/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libedit.so.2* /usr/lib/x86_64-linux-gnu/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libbsd.so.0* /usr/lib/x86_64-linux-gnu/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libsrt.* /usr/lib/x86_64-linux-gnu/
COPY --from=builder /usr/local/lib/libtsduck.so /usr/local/lib/
COPY --from=builder /usr/local/lib/tsduck /usr/local/lib/
COPY --from=builder /usr/local/share/tsduck /usr/local/share/tsduck
COPY --from=builder /usr/local/etc/security/console.perms.d/80-tsduck.perms /usr/local/etc/security/console.perms.d/
COPY --from=builder /usr/local/etc/udev/rules.d/80-tsduck.rules /usr/local/etc/udev/rules.d/
COPY --from=builder /usr/local/include/oatpp-1.3.0 /usr/local/include/oatpp-1.3.0
COPY --from=builder /usr/local/lib/oatpp-1.3.0 /usr/local/lib/oatpp-1.3.0

## Install production libraries
RUN  apt-get install --yes \
          libboost-system1.74.0 \
          libcurl4 \
          libzmq5 \
          tzdata \
     && rm -rf /var/lib/apt/lists/*



EXPOSE 9001-9016
EXPOSE 9201
EXPOSE 8000
ENTRYPOINT ["odr-muxmpe"]
LABEL org.opencontainers.image.vendor="Open Digital Radio" 
LABEL org.opencontainers.image.description="MPE Multiplexer" 
LABEL org.opencontainers.image.authors="andy.mace@mediauk.net" 
