smet2html: smet2html.c
	gcc -Wall -o smet2html smet2html.c

clean:
	rm smet2html 2>/dev/null || /bin/true
	rm build-stamp 2>/dev/null || /bin/true
	rm debian/files 2>/dev/null || /bin/true
	rm debian/substvars 2>/dev/null || /bin/true
	rm -rf debian/tmp 2>/dev/null || /bin/true

install: smet2html
	/usr/bin/install -c -p smet2html /usr/local/bin/smet2html

install-debpkgbuild: smet2html
	mkdir debian/tmp/ 2>/dev/null || /bin/true
	mkdir debian/tmp/usr/ 2>/dev/null || /bin/true
	mkdir debian/tmp/usr/bin/ 2>/dev/null || /bin/true
	/usr/bin/install -c -p smet2html debian/tmp/usr/bin/smet2html

dist: clean
	cd .. && tar --exclude=CVS -cf /tmp/smet2html-0.1.tar smet2html/*
	gzip /tmp/smet2html-0.1.tar
	mv /tmp/smet2html-0.1.tar.gz .

