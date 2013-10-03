
ACLOCAL_AMFLAGS = -I m4

moddir = @APACHE_MODULEDIR@
mod_LTLIBRARIES = mod_hoedown.la

mod_hoedown_la_SOURCES = \
	mod_hoedown.c \
	hoedown/src/autolink.c \
	hoedown/src/buffer.c \
	hoedown/src/escape.c \
	hoedown/src/html.c \
	hoedown/src/html_blocks.c \
	hoedown/src/html_smartypants.c \
	hoedown/src/markdown.c \
	hoedown/src/stack.c

mod_hoedown_la_CFLAGS = @APACHE_CFLAGS@ @APACHE_INCLUDES@ @CURL_CFLAGS@
mod_hoedown_la_CPPFLAGS = @APACHE_CPPFLAGS@ @APACHE_INCLUDES@ @CURL_CPPFLAGS@
mod_hoedown_la_LDFLAGS = -avoid-version -module @APACHE_LDFLAGS@ @CURL_LDFLAGS@
mod_hoedown_la_LIBS = @APACHE_LIBS@ @CURL_LIBS@
