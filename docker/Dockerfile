FROM debian:stretch
MAINTAINER Freddie Rice

# install the dependencies 
RUN apt-get -y update \
	&& apt-get -y install build-essential curl gawk bison flex automake texinfo libtool libtool-bin cvs libncurses5-dev python-dev gperf help2man git wget cmake unzip pkg-config

# dumb-init
RUN wget -O /usr/local/bin/dumb-init https://github.com/Yelp/dumb-init/releases/download/v1.2.1/dumb-init_1.2.1_amd64 \
	&& chmod 0755 /usr/local/bin/dumb-init

# add the non-root user and build
RUN useradd --create-home --shell /bin/bash user
USER user
ENV PATH="${PATH}:/home/user/local/bin"
RUN cd /home/user \
	&& mkdir local \
	&& wget "http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.23.0.tar.xz" \
	&& tar xvf crosstool-ng-1.23.0.tar.xz \
	&& cd crosstool-ng-1.23.0 \
	&& ./configure --prefix=/home/user/local \
	&& make \
	&& make install \
	&& cd .. \
	&& rm -rf crosstool-ng-1.23.0 crosstool-ng-1.23.0.tar.xz

# build compiler
RUN cd /home/user && mkdir pi && cd pi \
	&& ct-ng armv8-rpi3-linux-gnueabihf \
	&& ct-ng build \
	&& cd .. \
	&& rm -rf pi \
	&& cd /home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/ \
	&& chmod 0755 . \
	&& mkdir -p etc lib sbin usr \
	&& chmod 0755 etc lib sbin usr \
	&& mkdir -p usr/bin usr/include usr/lib \
	&& chmod 0755 usr/bin usr/include usr/lib usr/share \
	&& cd /home/user
ENV PATH="${PATH}:/home/user/x-tools/armv8-rpi3-linux-gnueabihf/bin/"

# build protobuf for host and client
RUN cd /home/user \
	&& wget https://github.com/google/protobuf/releases/download/v3.5.0/protobuf-all-3.5.0.tar.gz \
	&& tar xvf protobuf-all-3.5.0.tar.gz \
	&& cd protobuf-3.5.0 \
	&& ./configure --prefix=/home/user/local/ \
	&& make -j4 \
	&& make install \
	&& cd ../ \
	&& rm -rf protobuf-3.5.0 \
	&& tar xvf protobuf-all-3.5.0.tar.gz \
	&& cd protobuf-3.5.0 \
	&& ./configure --host=armv8-rpi3-linux-gnueabihf CC=armv8-rpi3-linux-gnueabihf-gcc CXX=armv8-rpi3-linux-gnueabihf-g++ --with-protoc=/home/user/local/bin/protoc --prefix=/home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr \
	&& make -j4 \
	&& make install \
	&& cd ../ \
	&& rm -rf protobuf-3.5.0 protobuf-all-3.5.0.tar.gz

# build mmal libraries
RUN cd /home/user \
	&& git clone https://github.com/raspberrypi/userland.git userland \
	&& cd userland \
	&& git checkout 3cd60d45bc7c9d3ec8daee574bc99027cb1bab9e 
COPY armv8.cmake /home/user/userland/makefiles/cmake/toolchains/
RUN cd /home/user/userland \
	&& mkdir build \
	&& cd build \
	&& cmake -DCMAKE_TOOLCHAIN_FILE=../makefiles/cmake/toolchains/armv8.cmake -DCMAKE_BUILD_TYPE=Release ../ \
	&& make -j4
USER root
RUN cd /home/user/userland/build \
	&& make install \
	&& cd /home/user/ \
	&& rm -rf userland
USER user

# build opencv
RUN cd /home/user \
	&& wget https://github.com/opencv/opencv/archive/3.3.1.zip \
	&& unzip 3.3.1.zip
RUN cd /home/user \
	&& mkdir build \
	&& cd build \
	&& cmake -DCMAKE_CXX_COMPILER=`which armv8-rpi3-linux-gnueabihf-g++` -DCMAKE_C_COMPILER=`which armv8-rpi3-linux-gnueabihf-gcc` -DCMAKE_AR=`which armv8-rpi3-linux-gnueabihf-ar` -DCMAKE_TOOLCHAIN_FILE=../opencv-3.3.1/platforms/linux/arm-gnueabi.toolchain.cmake -DCMAKE_MAKE_PROGRAM:PATH=`which make` ../opencv-3.3.1 \
	&& make -j4 \
	&& make install \
	&& cp -r install/lib/* /home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr/lib/ \
	&& cp -r install/include/* /home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr/include \
	&& cd ../ \
	&& rm -rf 3.3.1.zip build opencv-3.3.1

# build zlib
RUN cd /home/user \
	&& wget http://zlib.net/zlib-1.2.11.tar.gz \
	&& tar xvf zlib-1.2.11.tar.gz \
	&& cd zlib-1.2.11 \
	&& CC=`which armv8-rpi3-linux-gnueabihf-gcc` ./configure --prefix=/home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr \
	&& make -j4 \
	&& make install \
	&& cd ../ \
	&& rm -rf zlib-1.2.11 zlib-1.2.11.tar.gz

# build openssl
RUN cd /home/user \
	&& wget https://www.openssl.org/source/openssl-1.0.2n.tar.gz \
	&& tar xvf openssl-1.0.2n.tar.gz \
	&& cd openssl-1.0.2n \
	&& ./Configure dist --prefix=/home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr \
	&& make CC=armv8-rpi3-linux-gnueabihf-gcc AR="armv8-rpi3-linux-gnueabihf-ar r" RANLIB=armv8-rpi3-linux-gnueabihf-ranlib \
	&& make install \
	&& cd ../ \
	&& rm -rf openssl-1.0.2n openssl-1.0.2n.tar.gz

# install spdlog
RUN cd /home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr/include \
	&& git clone https://github.com/gabime/spdlog.git spdlog_git \
	&& cd spdlog_git \
	&& git checkout v0.14.0 \
	&& cp -r include/spdlog ../ \
	&& cd ../ \
	&& rm -rf spdlog_git

# install golang
ENV PATH="${PATH}:/home/user/go/bin:/home/user/gopath/bin"
ENV GOPATH="/home/user/gopath"
RUN cd /home/user \
	&& wget https://redirector.gvt1.com/edgedl/go/go1.9.2.linux-amd64.tar.gz \
	&& tar xvf go1.9.2.linux-amd64.tar.gz \
	&& rm -rf go1.9.2.linux-amd64.tar.gz \
	&& mkdir -p gopath

# go gets
RUN cd /home/user \
	&& go get -u github.com/nsf/termbox-go \
	&& go get -u github.com/golang/protobuf/proto \
	&& go get -u github.com/golang/protobuf/protoc-gen-go

RUN cd /home/user \
	&& git clone https://github.com/libarchive/libarchive.git \
	&& cd libarchive \
	&& build/autogen.sh \
	&& ./configure --host=armv8-rpi3-linux-gnueabihf CC=armv8-rpi3-linux-gnueabihf-gcc --prefix=/home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr --without-openssl \
	&& make \
	&& make install

RUN cd /home/user \
	&& git clone https://github.com/vrpn/vrpn.git \
	&& mkdir vrpn/build \
	&& cd vrpn/build \
	&& CC=/home/user/x-tools/armv8-rpi3-linux-gnueabihf/bin/armv8-rpi3-linux-gnueabihf-gcc CXX=/home/user/x-tools/armv8-rpi3-linux-gnueabihf/bin/armv8-rpi3-linux-gnueabihf-g++ cmake -G "Unix Makefiles" ../ \
	&& make -j4 \
	&& mv quat/libquat.a *.a /home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr/lib/ \
	&& cp -r ../*.h /home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr/include/

USER root
RUN yes | apt-get install mercurial

USER user
RUN cd /home/user \
	&& hg clone https://bitbucket.org/eigen/eigen/ \
	&& cd eigen \
	&& mkdir build \
	&& cd build \
	&& CC=/home/user/x-tools/armv8-rpi3-linux-gnueabihf/bin/armv8-rpi3-linux-gnueabihf-gcc CXX=/home/user/x-tools/armv8-rpi3-linux-gnueabihf/bin/armv8-rpi3-linux-gnueabihf-g++ cmake -DCMAKE_INSTALL_PREFIX=/home/user/x-tools/armv8-rpi3-linux-gnueabihf/armv8-rpi3-linux-gnueabihf/sysroot/usr ../ \
	&& make install

WORKDIR /home/user/cdrone
ENTRYPOINT ["/usr/local/bin/dumb-init", "--"]
CMD ["bash", "-c", "make"]
