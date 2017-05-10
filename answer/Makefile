Obj = answer_tcp_2.o

answer_tcp_2:$(Obj)
	gcc -o answer_tcp_2 $(Obj)
answer_tcp_2.o:answer_tcp_2.c
	gcc -c answer_tcp_2.c
.PHONY:clean

clean:
	rm $(Obj)
install:
	./answer_tcp_2
