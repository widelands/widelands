SUBDIRS= src wffcreate wwfcreate wtfcreate

EXTRA_DIST=pics fonts worlds maps

EXTRADATADIRS=pics/ fonts/ worlds/ maps/
EXTRADATA=pics/*.bmp fonts/*.wff worlds/*.wwf maps/*.swd

datainstalldirs:
	   for dir in $(EXTRADATADIRS) ; do \
				          $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/$$dir ; \
							    done


install-data-local: datainstalldirs
	        for i in $(EXTRADATA); do \
						         $(INSTALL_DATA) $$i $(DESTDIR)$(pkgdatadir)/$$i; \
									        done

uninstall-am:
		  rm -rf $(pkgdatadir)

