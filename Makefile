.PHONY: clean

all:
	make -C cdrone docker
	make -C gobase docker

clean:
	make -C cdrone clean
	make -C gobase clean
