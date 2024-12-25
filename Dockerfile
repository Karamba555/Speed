FROM ubuntu:latest
WORKDIR /home/speedway
# Copy in the source code
COPY . . 
# Install the application dependencies
RUN apt update && apt -y dist-upgrade && apt -y install gcc \
    binutils bzip2 flex python3 perl make grep unzip \
    gawk subversion libz-dev libc-dev rsync pip sudo \
    libncurses5-dev libncursesw5-dev git swig wget file
RUN  make clean 
RUN sudo ./scripts/feeds update -a 
RUN sudo ./scripts/feeds install -a 
RUN git restore .config 
RUN make -j2


EXPOSE 3000
