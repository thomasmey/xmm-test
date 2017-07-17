
objs = xmm-test.o

xmmtester: $(objs)
	cc -o xmmtester $(objs) -lpthread -lm 

xmm-test.o:
