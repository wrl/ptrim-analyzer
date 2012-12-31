#!/usr/bin/env python

import subprocess
import time
import sys

top = "."
out = "build"

# change this stuff

APPNAME = "ptrim-analyzer"
VERSION = "0.1"

#
# dep checking functions
#

def check_jack(conf):
	conf.check_cc(
		mandatory=True,
		execute=True,

		lib="jack",
		header_name="jack/jack.h",
		uselib_store="JACK",

		msg="Checking for JACK")

#
# waf stuff
#

def options(opt):
	opt.load('compiler_c')

def configure(conf):
	# just for output prettifying
	# print() (as a function) ddoesn't work on python <2.7
	separator = lambda: sys.stdout.write("\n")

	separator()
	conf.load("compiler_c")
	conf.load("gnu_dirs")

	#
	# conf checks
	#

	separator()

	check_jack(conf)

	separator()

	#
	# setting defines, etc
	#

	conf.env.VERSION = VERSION
	#conf.env.append_unique("CFLAGS", ["-std=c99", "-Wall", "-Werror"])

def build(bld):
	bld.recurse("src")
