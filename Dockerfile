FROM ubuntu:latest

WORKDIR /home/speedway
# Copy in the source code
COPY . .

EXPOSE 3000

