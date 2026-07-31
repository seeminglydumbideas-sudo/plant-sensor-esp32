#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// Power & Sleep Configuration
// =============================================================================

// Boot count threshold after which the sensor switches to long deep sleep
// -> used to get more measurements during a period after firmware update or reboot.
// -> multiply by 2 min to know how long before switching to long deep sleep
#define LONG_SLEEP_BOOT_THRESHOLD  20

// Duration of long deep sleep in seconds 
// -> 12 hours ATM. should be challenged with battery usage.
#define LONG_SLEEP_DURATION_SEC    (12 * 60 * 60)

#endif // CONFIG_H
