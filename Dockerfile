FROM ubuntu:latest
WORKDIR /home/speedway
COPY . .
# Install the application dependencies
RUN apt update && apt -y dist-upgrade && apt -y install gcc \
    binutils bzip2 flex python3 perl make grep unzip \
    gawk subversion libz-dev libc-dev rsync pip sudo \
    libncurses5-dev libncursesw5-dev git swig wget file
CMD ["/home/speedway/start.sh"]

EXPOSE 3000
