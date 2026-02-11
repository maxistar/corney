# GATT layer number exposition

This document defines the custom GATT service and characteristic used to expose the active layer number.

## UUIDs
- Service UUID: `715d81e1-377d-4f26-a678-a506675d99ec`
- Characteristic UUID (Layer Number): `e87c518a-f323-4dd7-9dd5-0991add1c01b`

## Data format
- Value: unsigned 8-bit integer
- Endianness: not applicable (single byte)
- Semantics: zero-based active layer number, representing the highest active layer

## Properties
- Read
- Notify
