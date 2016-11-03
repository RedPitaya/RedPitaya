SSH and Console(USB) connection
###############################

SSH connection can be established using standard SSH clients such as OpenSSH (Linux, OS X) or PuTTy (Windows). Access information for SSH connection:

    - Username: root
    - Password: root
    
Connection examples will be given for Windows, Linux and OS X users separately.

Windows users
=============

For this example, PuTTy tool was used on Windows XP and Windows 7 Starter OS. Run PuTTy and enter the Red Pitaya’s IP address to »Host Name (or IP address)« field as shown in figure below.

.. figure:: 445px-PuTTy_connection_settings.png

    Figure: PuTTy SSH connection settings.
    
If you attempt to connect to Red Pitaya for the first time, a security alert will pop-up asking you to confirm the connection. At this time, the ssh-key will be added to the registry in your computer. Command prompt pops-up after login is successful (see figure below).

.. figure:: 445px-Win_putty_logged.png

    Figure: SSH connection via PuTTy

Linux users
===========

Start Terminal and type:

.. code-block:: shell-session

   user@ubuntu:~$ ssh root@192.168.1.100

   root@192.168.1.100's password: root

   Red Pitaya GNU/Linux/Ecosystem version 0.90-299

   redpitaya>
    
OS X users
==========

.. code-block:: shell-session
  
   Run XOS terminal: Launchpad → Other → Terminal and type:
   
   localhost:~ user$ ssh root@192.168.1.100
   
   root@10.0.3.249's password: root
   
   Red Pitaya GNU/Linux/Ecosystem version 0.90-299
   
   redpitaya>
    
