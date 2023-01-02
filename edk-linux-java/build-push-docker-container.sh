VERSION=$(git show -s --format=%h)
IMAGE_NAME="edk_linux_java_builder"
IMAGE_TAG="hueivdocker.ddns.nl-ein116.lan.intra.lighting.com:5000/$IMAGE_NAME:latest"
sudo docker pull hueivdocker.ddns.nl-ein116.lan.intra.lighting.com:5000/hiva-hse:latest
sudo docker build -t $IMAGE_NAME:$VERSION .
sudo docker tag $IMAGE_NAME:$VERSION $IMAGE_TAG
sudo docker push $IMAGE_TAG
