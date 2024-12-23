FROM ubuntu:latest
WORKDIR /usr/local/speedway
# Copy in the source code
COPY . .
# Install the application dependencies
RUN apt update && apt -y dist-upgrade && apt -y install gcc \
    binutils bzip2 flex python3 perl make grep unzip \
    gawk subversion libz-dev libc-dev rsync pip \
    libncurses5-dev libncursesw5-dev git swig wget file
RUN  make clean && ./scripts/feeds update -a && ./scripts/feeds install -a \
     && git restore .config && make -j2


EXPOSE 3000

