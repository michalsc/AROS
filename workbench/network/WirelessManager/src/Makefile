SUBDIRS=ap common crypto drivers eapol_auth eapol_supp eap_common eap_peer eap_server l2_packet radius rsn_supp tls utils wps

all:
	for d in $(SUBDIRS); do [ -d $$d ] && $(MAKE) -C $$d; done

clean:
	for d in $(SUBDIRS); do [ -d $$d ] && $(MAKE) -C $$d clean; done

install:
	for d in $(SUBDIRS); do [ -d $$d ] && $(MAKE) -C $$d install; done
