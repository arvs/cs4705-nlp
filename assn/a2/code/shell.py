try:
	from IPython.core import ipapi
except ImportError:
	from IPython import ipapi

def import_all(module):
	ipapi.get().ex('from %s import *' % module)

def import_x(module, submodules):
	if not isinstance(submodules, str):
		submodules = (',').join(submodules)
	ipapi.get().ex('from %s import %s' % (module, submodules))

import_x("parser", "Parser")
ipapi.get().ex('p = Parser("../train/rare.dat")')
