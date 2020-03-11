# rwmem
Read or/and write memory via /dev/mem interface

# How to add
* Place this repository into somewhere in your Android tree
* Add below snippet into your device.mk in device/$(VENDOR)/(TARGET_DEVICE)/
```
# Replace TARGET_DEVICE into your device name
PRODUCT_PACKAGES += \                                                            
    rwmem.$(TARGET_DEVICE)
```
