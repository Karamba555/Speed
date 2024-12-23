FROM ubuntu:latest
WORKDIR /usr/local/speedway
# Copy in the source code
COPY . .
# Install the application dependencies
RUN apt update && apt -y dist-upgrade && apt -y install gcc \
    binutils bzip2 flex python3 perl make grep unzip \
    gawk subversion libz-dev libc-dev rsync pip \
    libncurses5-dev libncursesw5-dev git swig 
RUN make dirclean && make clean 
RUN ./scripts/feeds update -a 
RUN ./scripts/feeds install -a 
RUN git restore .config 
RUN make -j2

EXPOSE 3000

