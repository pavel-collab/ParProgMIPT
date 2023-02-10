all:
	mpic++ HelloWorld.c -o HelloWorld
	mpic++ Sum.c -o Sum

HelloWorld:
	mpic++ HelloWorld.c -o HelloWorld

Sum:
	mpic++ Sum.c -o Sum

clear:
	rm HelloWorld Sum