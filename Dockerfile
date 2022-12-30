FROM gcc:latest as build
WORKDIR /busrpc-dev
COPY cmake/ ./cmake
COPY src/ ./src
COPY tests/ ./tests
COPY CMakeLists.txt .
RUN apt-get update && \
    apt-get install -y \
      cmake \
    && \
    mkdir build
WORKDIR /busrpc-dev/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DBUSRPC_BUILD_TESTS=ON -DBUSRPC_USE_STATIC_PROTOBUF=ON .. && \
    cmake --build . &&  \
    CTEST_OUTPUT_ON_FAILURE=ON cmake --build . --target test

FROM ubuntu:latest
RUN apt-get update && apt-get install -y protobuf-compiler
RUN groupadd -r busrpc && useradd -r -g busrpc busrpc
USER busrpc
WORKDIR /busrpc-dev
COPY --from=build /busrpc-dev/build/busrpc .
ENTRYPOINT ["./busrpc"]
