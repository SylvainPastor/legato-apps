
.PHONY: all server client
all: server client

server:
	mkapp -t localhost server.adef

run_server: server
	_build_server/localhost/app/server/staging/read-only/bin/server
	
client:
	mkapp -t localhost client.adef

run_client: client
	_build_client/localhost/app/client/staging/read-only/bin/client

clean:
	rm -rf _build_* *.update
