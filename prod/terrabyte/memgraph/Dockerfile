FROM ubuntu:22.04

RUN apt update && apt-get install -y wget curl python3 libpython3.10

# Install Memgraph
RUN wget https://download.memgraph.com/memgraph/v2.17.0/ubuntu-22.04/memgraph_2.17.0-1_amd64.deb && \
    dpkg -i memgraph_2.17.0-1_amd64.deb && \
    apt-get install -y -f && \
    rm memgraph_2.17.0-1_amd64.deb

# # Set vm.max_map_count
# RUN echo "vm.max_map_count=262144" >> /etc/sysctl.conf
#WORKDIR /home/user
CMD ["tail", "-f", "/dev/null"]