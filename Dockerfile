FROM ubuntu:latest

ENV IDF_PATH=/stm32-qemu-project/ESP8266_RTOS_SDK

RUN apt-get update && \
  apt-get install -y git make flex bison gperf libncurses5-dev libncursesw5-dev python3 pip

RUN ln -s /usr/bin/python3 /usr/bin/python

COPY . stm32-qemu-project/

RUN apt install python3-virtualenv -y

RUN /stm32-qemu-project/ESP8266_RTOS_SDK/install.sh

COPY ./install.sh ./install.sh

ENTRYPOINT ["bash", "install.sh"]
