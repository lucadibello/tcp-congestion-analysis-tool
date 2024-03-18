CFLAGS ?=-Wall -Werror -g

.PHONY: all
all: client

.PHONY: clean
clean:
	rm client client.*

.PHONY: client-tests
client-tests: all
	@./tests/client_cli.py
	@./tests/client.py

.PHONY: check
check: client-tests
