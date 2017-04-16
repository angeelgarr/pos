Obj = kazlist.o

kazlist:kazlist.o
	gcc -o $@ $^
list.o:kazlist.c list.h
	gcc -c $^
.PHONY:clean

clean:
	rm $(Obj)
