Obj = expoint.o

expoint:$(Obj)
	gcc -o expoint $(Obj)
expoint.o:expoint.c
	gcc -c expoint.c
.PHONY:clean

clean:
	rm $(Obj)
