External ADC clock
##################

ADC clock can be provided by:
    * On board 125MHz XO (default)
    * From external source / through extension connector (instructions provided bellow)

Schematic:

Remove:
- R37, R46
Add:
- R34 = 0R, R35 = 0R

.. image:: External_img1.png


Remove:
- FB11

.. image:: External_img2.png

Remove:
- 0R on C64, R24
Add:
- C64 = 100nF, C63 = 100nF,
- R36 = 100R

.. image:: External_img3.png

.. image:: External_shem.png
    :width: 70%	
