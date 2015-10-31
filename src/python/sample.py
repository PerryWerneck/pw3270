#!/usr/bin/python
#-*- coding: utf-8

import py3270

print "Teste extensão pw3270"

print py3270.Revision()

term = py3270.Terminal("")

print "Using pw3270 version " + term.Version() + " revision " + term.Revision()

term.Connect("tn3270://zos.efglobe.com:telnet",10);

print term.IsConnected()
print term.IsReady()

print term.GetStringAt(14,19,38)







