.PHONY: all clean log_reset check

all: critical hook.so process

hook.so: hook.c
	${CC} -shared -fPIC -o hook.so hook.c -ldl

check:
	$(MAKE) clean
	$(MAKE) all
	for f in $(wildcard ./tests/*); do \
		echo "=== Running $$f ==="; \
		./run.pl "$$f"; \
	done

clean:
	rm -f critical hook.so process

log_reset:
	rm -f log.txt
