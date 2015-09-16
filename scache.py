#!/usr/bin/env python

# Example of how to use scache in an application

from subprocess import check_output as ucall
import os

scache_path = os.path.dirname(os.path.realpath(__file__)) + '/scache'

def scache(timeout=300): return ucall([scache_path, str(timeout)])

def get_passphrase():
  try:
    pw = scache()
  except:
    scache(0)
    try:
      pw = scache()
    except:
      return None
    pass
  return pw[:-1]   

if __name__=='__main__':
  pwd1 = get_passphrase()
  pwd2 = get_passphrase()
  print "Your pass phrase is", len(pwd2), "characters long."
  pass
