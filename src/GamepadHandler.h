#ifndef __GAMEPADHANDLER_H__
#define __GAMEPADHANDLER_H__

/*
Gamepad handling class

Copyright (C) 2015, SURFsara 
Author: Casper van Leeuwen
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <linux/joystick.h>
#include <vector>

#define JOYSTICK_DEV "/dev/input/js0"
#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

struct gp_event {
    __u32 time;     /* event timestamp in milliseconds */
    __s16 value;    /* value */
    __u8 type;      /* event type */
    __u8 number;    /* axis/button number */
};

struct gp_state{
    std::vector<signed short> button;
    std::vector<signed short> axis;
};

class GamepadHandler {
public:
    GamepadHandler();
    ~GamepadHandler();
    void openDevice();
    void startReading();
    gp_state* getGamepadState();
    bool IsActive();

protected:

private:
    pthread_t thread;
    int gamepadID;
    gp_event* gamepadEv;
    gp_state* gamepadState;
    __u32 version;
    __u8 axes;
    __u8 buttons;
    char name[256];
    bool reading;
    static void* readEvents(void * obj);
};

#endif