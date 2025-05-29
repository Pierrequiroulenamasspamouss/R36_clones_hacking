#!/bin/sh
set -e

# Remember original directory (where script and dropbearmulti are)
BASE_DIR="$(dirname "$0")"
BASE_DIR="$(cd "$BASE_DIR" && pwd)"

# Load kernel modules
modprobe libcomposite
modprobe usb_f_rndis

# Setup USB gadget in gadget configfs dir
GADGET_DIR=/sys/kernel/config/usb_gadget/g1
mkdir -p "$GADGET_DIR"
cd "$GADGET_DIR"

echo 0x1d6b > idVendor
echo 0x0104 > idProduct

mkdir -p strings/0x409
echo "12345678" > strings/0x409/serialnumber
echo "Linux" > strings/0x409/manufacturer
echo "R36 emulation console" > strings/0x409/product

mkdir -p configs/c.1
echo 100 > configs/c.1/MaxPower

mkdir -p functions/rndis.usb0
ln -sf functions/rndis.usb0 configs/c.1/

echo ff300000.usb > UDC

ifconfig usb0 192.168.7.2 netmask 255.255.255.0 up

# Back to original directory to run dropbearmulti
cd "$BASE_DIR"

if [ ! -x ./dropbearmulti ]; then
  echo "Error: dropbearmulti binary not found or not executable in $BASE_DIR"
  exit 1
fi

KEYS="dropbear_rsa_host_key dropbear_dss_host_key dropbear_ecdsa_host_key dropbear_ed25519_host_key"
for key in $KEYS; do
  if [ ! -f "$key" ]; then
    echo "Generating $key ..."
    case "$key" in
      dropbear_rsa_host_key) TYPE=rsa ;;
      dropbear_dss_host_key) TYPE=dss ;;
      dropbear_ecdsa_host_key) TYPE=ecdsa ;;
      dropbear_ed25519_host_key) TYPE=ed25519 ;;
    esac
    ./dropbearmulti dropbearkey -t "$TYPE" -f "$key"
  else
    echo "$key already exists, skipping generation."
  fi
done

echo "Starting Dropbear SSH server..."
./dropbearmulti dropbear -p 22 \
  -r dropbear_rsa_host_key \
  -r dropbear_dss_host_key \
  -r dropbear_ecdsa_host_key \
  -r dropbear_ed25519_host_key \
  -F -E
