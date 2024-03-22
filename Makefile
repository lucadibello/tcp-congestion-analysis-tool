CFLAGS ?=-Wall -Werror -g

.PHONY: all
all: server client

.PHONY: clean
clean:
	rm server client

.PHONY: server-tests
server-tests: all
	@./tests/server_cli.py
	@./tests/server.py

.PHONY: client-tests
client-tests: all
	@./tests/client_cli.py
	@./tests/client.py

.PHONY: check
check: server-tests client-tests
