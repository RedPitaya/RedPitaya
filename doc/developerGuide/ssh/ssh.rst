.. _ssh:

##############
SSH connection
##############

Access information for SSH connection:

* Username: ``root``
* Password: ``root``
    
Connection instructions are available for:

* :ref:`Windows <windows>`,
* :ref:`Linux <linux>`,
* :ref:`macOS <macos>`.


.. _windows:

=======
Windows
=======

For this example, `PuTTy tool <http://www.putty.org/>`_
was used on Windows XP and Windows 7 Starter OS.
Run PuTTy and enter the Red Pitaya's IP address into
**Host Name (or IP address)** field.

.. figure:: 445px-PuTTy_connection_settings.png

   Figure: PuTTy SSH connection settings.
    
If you attempt to connect to Red Pitaya for the first time,
a security alert will pop-up asking you to confirm the connection.
At this time, the ssh-key will be added to the registry in your computer.
Command prompt pops-up after login is successful.

.. figure:: 445px-Win_putty_logged.png

   Figure: SSH connection via PuTTy


.. _linux:

=====
Linux
=====

Start Terminal and type (replace IP address with the right one):

.. code-block:: shell-session

   user@ubuntu:~$ ssh root@192.168.1.100
   root@192.168.1.100's password: root
   Red Pitaya GNU/Linux/Ecosystem version 0.90-299
   redpitaya>


.. _macos:

=====
macOS
=====

Run terminal **Launchpad → Other → Terminal** and type (replace IP address with the right one):

.. code-block:: shell-session
  
   localhost:~ user$ ssh root@192.168.1.100
   root@10.0.3.249's password: root
   Red Pitaya GNU/Linux/Ecosystem version 0.90-299
   redpitaya>
