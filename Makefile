##
# SCCPp 0.1
# 
sccpp: sccpp.c
	$(CC) $< -o $@

clean:
	rm -f sccpp

install: sccpp
	install -d "/usr/bin"
	install -m 755 sccpp "/usr/bin"

uninstall:
	rm -rf "/usr/bin/sccpp"
