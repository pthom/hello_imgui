FROM gitpod/workspace-full-vnc
RUN sudo apt-get update
RUN sudo apt install -y libglfw3-dev
RUN sudo rm -rf /var/lib/apt/lists/*

# Install emscripten toolchain
# RUN cd ~ \
#     && git clone https://github.com/emscripten-core/emsdk.git \
#     && cd emsdk \
#     && git pull \
#     && ./emsdk install latest \
#     && ./emsdk activate latest