.PHONY=all clean
DOCKER_PATH=${GOPATH}/src/github.com/docker/docker-credential-helpers
POSTMAN_URL="https://dl.pstmn.io/download/latest/linux64"

all: docker-credential-pass Postman

docker-credential-pass:
	go get github.com/docker/docker-credential-helpers || true
	make -C $(DOCKER_PATH) pass
	mv $(DOCKER_PATH)/bin/docker-credential-pass .

Postman:
	wget -O postman_linux64.tar.gz $(POSTMAN_URL)
	tar -xf postman_linux64.tar.gz
	rm -f postman_linux64.tar.gz
	touch Postman/log.txt

clean:
	rm -f docker-credential-pass
	rm -rf Postman
