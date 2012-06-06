noinst_LTLIBRARIES = librply.la

experimental_cflags = 				\
	-DCOGL_ENABLE_EXPERIMENTAL_API		\
	-DCOGL_ENABLE_EXPERIMENTAL_2_0_API	\
	$(NULL)

librply_la_SOURCES = rply/rply.c rply/rply.h
librply_la_CFLAGS  = $(GLIB_CFLAGS)
librply_la_LIBADD  = $(GLIB_LIBS)

bin_PROGRAMS = wonderbar

wonderbar_SOURCES = 			\
	es-main.c 			\
	es-main.h			\
	es-entity.c			\
	es-entity.h			\
	es-util.c			\
	es-util.h			\
	mash-data-loader.c		\
	mash-data-loader.h		\
	mash-data.c			\
	mash-data.h			\
	mash-ply-loader.c		\
	mash-ply-loader.h		\
	components/es-animation-clip.c	\
	components/es-animation-clip.h	\
	components/es-camera.c		\
	components/es-camera.h		\
	components/es-light.c		\
	components/es-light.h		\
	components/es-mesh-renderer.c	\
	components/es-mesh-renderer.h	\
	$(NULL)

wonderbar_CFLAGS  = 		\
	-Isrc			\
	$(WARNING_CFLAGS) 	\
	$(GLIB_CFLAGS)		\
	$(SDL_CFLAGS) 		\
	$(COGL_CFLAGS)		\
	$(experimental_cflags)	\
	$(NULL)

wonderbar_LDADD   = librply.la $(GLIB_LIBS) $(SDL_LIBS) $(COGL_LIBS)
