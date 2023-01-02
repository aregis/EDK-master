FROM hueivdocker.ddns.nl-ein116.lan.intra.lighting.com:5000/hiva-hse:latest

USER root
RUN apt-get update && apt-get install -y \
		apt-transport-https \
		ca-certificates \
		gnupg \
	&& apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF \
	&& echo "deb https://download.mono-project.com/repo/ubuntu stable-bionic main" > /etc/apt/sources.list.d/mono-official-stable.list \
	&& apt-get update && apt-get install -y \
		build-essential \
		cmake \
		dirmngr \
		git \
		openjdk-8-jdk \
		swig \
		mono-complete \
	&& rm -rf /var/lib/apt/lists/* && \
	mkdir /workspace

USER jenkins