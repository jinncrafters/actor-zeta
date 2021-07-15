FROM ubuntu:bionic

RUN    apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y \
        apt-utils \
        gcc \ 
        g++ \
        clang \
        clang-format \
        clang-tidy \
        openssh-server \
        cmake \
        build-essential \
        gdb \
        gdbserver \
        rsync \
        vim

ADD . /code
WORKDIR /code

RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd
RUN sed -i 's/PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

EXPOSE 22 7777

RUN useradd -ms /bin/bash debugger
RUN echo 'debugger:pwd' | chpasswd

CMD ["/usr/sbin/sshd", "-D"]
