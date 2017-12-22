.PHONY: clean

all:
	make -C cdrone docker
	make -C gobase docker

pkg:
	make -C cdrone pkg

clean:
	make -C cdrone clean
	make -C gobase clean
