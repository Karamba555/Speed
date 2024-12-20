#!/bin/sh
openssl s_client -showcerts -servername odis.pluslab.pl -connect odis.pluslab.pl:443 2>/dev/null | openssl x509 -inform pem -noout -modulus |openssl md5 >/tmp/ssl


