SUBDIRS= src

EXTRA_DIST=pics

EXTRA_DIST=pics 

EXTRADATADIRS=pics/
EXTRADATA=pics/*.bmp

datainstalldirs:
	   for dir in $(EXTRADATADIRS) ; do \
				          $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/$$dir ; \
							    done


install-data-local: datainstalldirs
	        for i in $(EXTRADATA); do \
						         $(INSTALL_DATA) $$i $(DESTDIR)$(pkgdatadir)/$$i; \
									        done

