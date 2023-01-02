CONTAINER="edk_linux_java_builder"
IMAGE="hueivdocker.ddns.nl-ein116.lan.intra.lighting.com:5000/$CONTAINER:latest"
sudo docker pull "$IMAGE"
sudo docker run --rm --name $CONTAINER -v $(pwd)/../:/workspace/ --workdir /workspace --user 1000:1000 $IMAGE /bin/bash -c "rm -rf build/ && mkdir build/ && cd build/ && cmake -DBUILD_WRAPPERS=ON .. && make"
