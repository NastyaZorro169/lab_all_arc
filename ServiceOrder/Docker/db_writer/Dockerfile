FROM ubuntu
ENV TZ=Europe/Moscow
RUN apt-get clean && apt-get update -y 
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata git sudo build-essential autoconf libtool libssl-dev zlib1g-dev librdkafka-dev pkg-config cmake gcc git g++ mysql-client libmysqlclient-dev libboost-all-dev libssl-dev && apt-get clean
RUN git clone -b main https://github.com/pocoproject/poco.git && cd poco && mkdir cmake-build && cd cmake-build && cmake .. && make && sudo make install && cd .. && cd ..
RUN git clone https://github.com/tdv/redis-cpp.git && cd redis-cpp && mkdir build && cd build && cmake .. && make && sudo make install && cd .. && cd .. 
RUN git clone https://github.com/edenhill/librdkafka.git && cd librdkafka && ./configure && make && sudo make install && cd 
RUN git clone https://github.com/mfontanini/cppkafka && cd cppkafka && mkdir build && cd build && cmake .. && make && sudo make install && cd 
ARG build3=$RANDOM
RUN git clone https://github.com/NastyaZorro169/labs_arc.git
RUN sudo ldconfig
RUN cd labs_arc/ServiceOrder/db_writer && mkdir build && cd build && cmake  .. && make
ENTRYPOINT [ "labs_arc/ServiceOrder/db_writer/build/db_writer" ]