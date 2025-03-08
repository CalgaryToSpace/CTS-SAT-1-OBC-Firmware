#include <stdbool.h>

#define SENTENCE_LEN    67

enum Capture_Status{Transmit_Success, Wrong_input, Capture_Failure};


      /**
     * Transmits ASCII telecommand based on input
     * @param lighting - lighting should be a *lower case* char
     * 			d - daylight ambient light
     * 			m - medium ambient light
     * 			n - night ambient light
     * 			s - solar sail contrast and light
     */
    enum Capture_Status CAM_Capture_Image(bool enable_flash, uint8_t lighting_mode);