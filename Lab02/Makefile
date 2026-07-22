.PHONY: all clean check_raft check_bracha check

all: process

check_raft: PROTOCOL=RAFT
check_bracha: PROTOCOL=BRACHA
check_raft check_bracha: check

check:
	$(MAKE) clean
	$(MAKE) all
	make -C ../Lab01 hook.so
	for f in $(wildcard ./tests/*); do \
		echo "=== Running $$f with $(PROTOCOL) ==="; \
		env PROTOCOL="$(PROTOCOL)" ./run.pl "$$f"; \
	done

clean:
	rm -f process
