from wyliodrin import *

from time import *

pinMode (16, 1)


print('Example 1 - LED blinker')
while True:
  digitalWrite (16, 1);
  sleep ((500)/1000.0)
  digitalWrite (16, 1);
  sleep ((500)/1000.0)
