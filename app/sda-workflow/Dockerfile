FROM fishnet/fishnet:latest
RUN apt-get update && apt-get install -y \
    libssl-dev \
    nodejs\
    python3 \
    python3-pip \
    python3-venv\
    && apt-get clean
# Create venv for installation of toil
RUN python3 -m venv /opt/venv 
# Install toil
RUN /opt/venv/bin/pip install toil[cwl]
# Add toil to path
ENV PATH="/opt/venv/bin:$PATH"
# Copy cfg files in user directory
# RUN cp -r /fishnet/prod/cfg /root
# RUN mkdir -p /root/jobs
# RUN mkdir -p /root/working
WORKDIR /data
CMD ["tail", "-f", "/dev/null"]
