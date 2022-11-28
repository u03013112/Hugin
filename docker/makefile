OS=ubuntu
DOCKERFILE=dockerfile.${OS}
TAG=${OS}v1
ACCOUNT=u03013112
IMAGE=${ACCOUNT}/hugin_build:${TAG}

all:
	docker build -f ${DOCKERFILE} -t ${IMAGE} .
push:
	docker push ${IMAGE} 