FROM ubuntu:xenial

RUN apt-get update && apt-get -y install cmake git build-essential vim python clang
RUN apt-get -y remove gcc g++
RUN mkdir /src
RUN cd /src && git clone http://llvm.org/git/llvm.git
RUN cd /src/llvm/tools && git clone http://llvm.org/git/clang.git
RUN mkdir /src/llvm/build 
RUN sed -i -e s/Hexagon/Lanai/ /src/llvm/CMakeLists.txt
RUN cd /src/llvm/build && cmake -DLLVM_TARGETS_TO_BUILD="X86;Lanai" ..
RUN cd /src/llvm/build && make -j 3
RUN cd /src/llvm/build && make install

WORKDIR /mnt/
