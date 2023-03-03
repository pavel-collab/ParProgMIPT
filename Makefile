all:
	mpic++ HelloWorld.c -o HelloWorld
	mpic++ Sum.c -o Sum
	mpic++ RoundSending.c -o RoundSending

HelloWorld:
	mpic++ HelloWorld.c -o HelloWorld

Sum:
	mpic++ Sum.c -o Sum

RoundSending:
	mpic++ RoundSending.c -o RoundSending

exponent:
	mpic++ exponent.c -o exponent

clear:
	rm HelloWorld Sum RoundSending exponent