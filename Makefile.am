SUBDIRS= src wffcreate wwfcreate

EXTRA_DIST=pics fonts

EXTRADATADIRS=pics/ fonts/ worlds/
EXTRADATA=pics/*.bmp fonts/*.wff worlds/*.wwf

datainstalldirs:
	   for dir in $(EXTRADATADIRS) ; do \
				          $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/$$dir ; \
							    done


install-data-local: datainstalldirs
	        for i in $(EXTRADATA); do \
						         $(INSTALL_DATA) $$i $(DESTDIR)$(pkgdatadir)/$$i; \
									        done

