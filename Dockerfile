FROM gcc:13 AS build
WORKDIR /src
COPY server.cpp lru_cache.h ./
RUN g++ -O2 -static server.cpp -o server

FROM debian:bookworm-slim
COPY --from=build /src/server /usr/local/bin/server
WORKDIR /data
EXPOSE 6380
CMD ["server"]
