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

Exponent:
	mpic++ Exponent.c -o Exponent

BlockingSendResearch:
	mpic++ BlockingSendResearch.c -o BlockingSendResearch

clear:
	rm HelloWorld Sum RoundSending exponent