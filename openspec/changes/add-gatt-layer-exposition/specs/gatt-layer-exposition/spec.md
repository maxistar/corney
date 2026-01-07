## ADDED Requirements
### Requirement: Expose active layer number over GATT
The firmware SHALL expose the currently active layer number as a readable GATT characteristic.

#### Scenario: Read current layer
- **WHEN** a BLE client reads the layer characteristic
- **THEN** the firmware returns the active layer number as an unsigned integer

### Requirement: Notify on layer changes
The firmware SHALL send GATT notifications for the layer characteristic when the active layer changes and a client has subscribed.

#### Scenario: Layer change notification
- **WHEN** a BLE client subscribes to notifications and the active layer changes
- **THEN** the firmware sends a notification containing the new active layer number
