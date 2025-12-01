#!/bin/bash
# Replace with your actual systemd service name
service_name="redpitaya_scpi.service"
if systemctl is-active --quiet "$service_name"; then
  echo "running"
  exit 0 # Service is active (success)
else
  echo "stopped"
  exit 1 # Service is not active (failure)
fi
