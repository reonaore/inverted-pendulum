# Firmware

## Requirements

* VScode
* Recommended plugins in workspace
  * Open command pallet and type `Show Recommended Extensions`

## How to build and write firm to device

* Create `data/wifi-config.json`. Example is described below

```jsonc
{
    "ssid": "<SSID>", // your local ssid
    "password": "<Password>" // your ssid's password
}
```

* Upload it by
  1. Open command pallet and type `Run task`
  2. Choose `Upload Filesystem Image`
* Build and upload
  1. Open command pallet and type `Run task`
  2. Choose `Upload`
* See [official site](https://docs.platformio.org/en/stable/integration/ide/vscode.html#quick-start) to know details

## APIs

This firm has APIs to control/monitor device.

* TODO
