######################
General considerations
######################

#. follow best practices
#. no default passwords
#. no hardcoded passwords
#. user settings similar to Ubuntu (use ``sudo``, disabled ``root`` user)
#. avoid running applications as root
#. HW access requires group membership

Main issues:
#. backward compatibility
#. Fat32 does not provide necessary file attributes

###########################
Hardware access permissions
###########################

#####
Users
#####

=====================================
System users for running applications
=====================================

==========================
``redpitaya`` default user
==========================

=============
``root`` user
=============

Aftert the first boot the ``root`` user does not have a password
and is therefore disabled.
Setting ``root`` user password enables access to the account:

.. code-block:: shell-session

   $ sudo passwd root
