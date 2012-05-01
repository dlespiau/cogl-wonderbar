ACLOCAL_AMFLAGS = -I build ${ACLOCAL_FLAGS}

noinst_LTLIBRARIES = librply.la

librply_la_SOURCES = rply/rply.c rply/rply.h
librply_la_CFLAGS  = $(GLIB_CFLAGS)
librply_la_LIBADD  = $(GLIB_LIBS)

bin_PROGRAMS = wonderbar

wonderbar_SOURCES = 		\
	main.c 			\
	mash-data-loader.c	\
	mash-data-loader.h	\
	mash-data.c		\
	mash-data.h		\
	mash-ply-loader.c	\
	mash-ply-loader.h	\
	$(NULL)

wonderbar_CFLAGS  = $(WARNING_CFLAGS) $(CLUTTER_CFLAGS)
wonderbar_LDADD   = librply.la $(CLUTTER_LIBS)
