TSXS?=tsxs

%.so: %.c
	$(TSXS) -c $< -o $@

all: test_remap.so

install: all
	$(TSXS) -i -o test_remap.so

clean:
	rm -f *.lo *.so
