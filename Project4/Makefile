src = *.c
hdr = *.h
dep = $(hdr) $(src)
bin = a.out

$(bin): $(dep)
	gcc -Wall -g $(src) -o $(bin);

all: $(bin)

clean:
	rm $(bin);

