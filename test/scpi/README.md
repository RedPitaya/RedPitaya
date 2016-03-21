# SPCI

SCPI server should be started using its systemd service. It is suggested to also stop the Nginx service (web applications) to avoid interference, since both have access to the hardware.
```bash
systemctl stop redpitaya_nginx
systemctl start redpitaya_scpi
```

