FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Зависимости сборки + wxWidgets (GTK3)
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        git \
        wget \
        ca-certificates \
        pkg-config \
        libwxgtk3.2-dev \
        libwxgtk-webview3.2-dev \
        libgtk-3-dev \
        libglib2.0-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /devel

# Клонирование публичного репозитория
RUN git clone --depth=1 \
        https://github.com/LinCSP/video2doc.git \
        ./video2doc

# Создаём директорию сборки
RUN mkdir -p /devel/video2doc-build

COPY ./build.sh /devel/
RUN chmod u+x /devel/build.sh

WORKDIR /devel
CMD ["/devel/build.sh"]
