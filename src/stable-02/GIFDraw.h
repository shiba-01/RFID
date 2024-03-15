//
// Created by lenovo on 1/23/2024.
//

#ifndef RFID_GIFDRAW_H
#define RFID_GIFDRAW_H

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 480
#define BUFFER_SIZE 256            // Optimum is >= GIF width or integral division of width

#ifdef USE_DMA
extern uint16_t usTemp[2][BUFFER_SIZE]; // Global to support DMA use
#else
extern uint16_t usTemp[1][BUFFER_SIZE];    // Global to support DMA use
#endif
extern bool dmaBuf;

#endif //RFID_GIFDRAW_H
