FROM ubuntu:14.04
LABEL maintainer="Binbin Zhang"

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    libopenblas-dev \
    git \
    python3-dev \
    python3-pip \
    swig \
    libmysqlclient-dev \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install setuptools tornado==5.0.2 mysqlclient

RUN git clone --branch master --recursive https://github.com/robin1001/xdecoder.git

RUN cd xdecoder \
    && make server \
    && make clean

WORKDIR xdecoder
ADD exp exp

RUN pwd && ls

EXPOSE 80

CMD ["python3", "server/main.py", "server/config.json"]
