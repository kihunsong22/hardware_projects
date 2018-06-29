
  TCPIP_EXAMPLE
  -------------

This example shows some possibilities of the tcpip stack used for the
weatherstation. It is a device which you can connect to in the same
way as to the stand-alone RS232-enabled weatherstation, in fact it runs
on the same FPGA project as that example.

Dialup uses 115200 baud, user 'user', password 'password'

Functionality included:

- You can browse to http://embedded.test

- You can FTP to embedded.test
  users: 'user', 'root', 'test' with their passwords 'user', 'root', 'test'

- You can TELNET to embedded.test
  users: 'user', 'root', 'test' with their passwords 'user', 'root', 'test'

- If you press any button on the keypad the device will connect to an
  outside SMTP server and send a mail to you.

  IMPORTANT: this will only work after some defines in main.c have been
  changed to reflect your setup, and some routing for your workstation
  and network has been set up. Read main.c for more info.

You can use tools\stackconf.exe to read the tcpip-settings file general.ini,
and optianally modify things and regenerate the source-files to reflect your
changes.


