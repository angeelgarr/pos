Obj = list.o

list:list.o
	gcc -o list list.c
list.o:list.c
	gcc -c list.c
.PHONY:clean

clean:
	rm $(Obj)
