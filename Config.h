#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// Power & Sleep Configuration
// =============================================================================

// Boot count threshold after which the sensor switches to long deep sleep
#define LONG_SLEEP_BOOT_THRESHOLD  20

// Duration of long deep sleep in seconds 
// -> initially 15 minutes during tests phase.
// -> 12 hours in prod ?
#define LONG_SLEEP_DURATION_SEC    (12 * 60 * 60)

#endif // CONFIG_H
