import platform
from sys import version_info as _swig_python_version_info

short_version=platform.python_version().split(".")
short_version=short_version[0]+'.'+short_version[1]

if short_version=="3.9":
    if __package__ or "." in __name__:
        from ._python_lib3_9 import *
    else:
        from _python_lib3_9 import *

if short_version=="3.10":
    if __package__ or "." in __name__:
        from ._python_lib3_10 import *
    else:
        from _python_lib3_10 import *

if short_version=="3.11":
    if __package__ or "." in __name__:
        from ._python_lib3_11 import *
    else:
        from _python_lib3_11 import *

if short_version=="3.12":
    if __package__ or "." in __name__:
        from ._python_lib3_12 import *
    else:
        from _python_lib3_12 import *

if short_version=="3.13":
    if __package__ or "." in __name__:
        from ._python_lib3_13 import *
    else:
        from _python_lib3_13 import *