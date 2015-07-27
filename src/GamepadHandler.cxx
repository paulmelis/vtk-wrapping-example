/*
Gamepad handling class

Copyright (C) 2015, SURFsara 
Author: Casper van Leeuwen
*/
#include "GamepadHandler.h"

GamepadHandler::GamepadHandler() : gamepadID(0), gamepadEv(0), gamepadState(0), version(0), axes(0), buttons(0), thread(0), reading(false)
{
    this->openDevice(); // Find and setup IO
    this->startReading(); // Read IO in thread
}

GamepadHandler::~GamepadHandler()
{
    if (gamepadID > 0) 
    {
        this->reading = false;
        pthread_join(thread, 0);
        std::cout << "pthread_join() returned" << std::endl;
        // XXX closing the device file seems to leave the joystick state
        // incorrect (with buttons 5-8 still pressed) at next run, causing
        // us to exit right away
        //close(this->gamepadID);
    }
    delete gamepadState;
    delete gamepadEv;
    gamepadID = 0;
}

// ----------------------------------------------------------------------------
// Description:
// This functions open the device associated with the gamepad
// and setup the gamepad state.
void GamepadHandler::openDevice()
{
    this->gamepadEv = new gp_event(); // gp event struct {time, value, type, number}
    this->gamepadState = new gp_state(); // gp event struct {buttons and axis}
    this->gamepadID = open(JOYSTICK_DEV, O_RDONLY | O_NONBLOCK);
    
    if (this->gamepadID == 0)        
    {
        std::cout << "WARNING: gamepad device could not be opened!" << std::endl;
        return;
    }
        
    ioctl(this->gamepadID, JSIOCGNAME(256), this->name);
    ioctl(this->gamepadID, JSIOCGVERSION, &(this->version));
    ioctl(this->gamepadID, JSIOCGAXES, &(this->axes));
    ioctl(this->gamepadID, JSIOCGBUTTONS, &(this->buttons));
    
    std::cout << "Gamepad detected" << std::endl;
    std::cout << "   Name: " << this->name << std::endl;
    std::cout << "Version: " << this->version << std::endl;
    std::cout << "   Axes: " << (int)this->axes << std::endl;
    std::cout << "Buttons: " << (int)this->buttons << std::endl;
    
    this->gamepadState->axis.reserve(this->axes);
    this->gamepadState->button.reserve(this->buttons);
    
    std::cout << "Printing gamepad state with " << gamepadState->button.capacity() << " buttons and " << gamepadState->axis.capacity() << " axes: " << std::endl;
}

// ----------------------------------------------------------------------------
// Description:
// Start the reading of the gamepad asynchronously in another thread
void GamepadHandler::startReading()
{
    if (this->gamepadID == 0)
    {
        std::cout << "No gamepad present..." << std::endl;
        return;
    }
    
    pthread_create(&(this->thread), 0, &GamepadHandler::readEvents, this); 
    this->reading = true;
}

// ----------------------------------------------------------------------------
// Description:
// Continues loop reading gamepad state until gamepad is deleted
void* GamepadHandler::readEvents(void *obj) 
{
    int bytes;
    
    GamepadHandler* gp =  reinterpret_cast<GamepadHandler *>(obj);
    while(gp->reading)
    {
        bytes = read(gp->gamepadID, gp->gamepadEv, sizeof(*(gp->gamepadEv)));
        if (bytes > 0) 
        {
            gp->gamepadEv->type &= ~JS_EVENT_INIT;
            if (gp->gamepadEv->type & JS_EVENT_BUTTON)
                gp->gamepadState->button[gp->gamepadEv->number] = gp->gamepadEv->value;
            if (gp->gamepadEv->type & JS_EVENT_AXIS)
                gp->gamepadState->axis[gp->gamepadEv->number] = gp->gamepadEv->value;
        }
        
        usleep(10000);
    }
    
    std::cout << "GamepadHandler::readEvents() done" << std::endl;
    
    // Clear out any events left
    bytes = read(gp->gamepadID, gp->gamepadEv, sizeof(*(gp->gamepadEv)));
    while (bytes > 0)
    {
        bytes = read(gp->gamepadID, gp->gamepadEv, sizeof(*(gp->gamepadEv)));
    }
    
    std::cout << "GamepadHandler::readEvents() returned" << std::endl;
}

// ----------------------------------------------------------------------------
// Description:
// Get gamepad state
gp_state* GamepadHandler::getGamepadState()
{
    return this->gamepadState;
}

bool GamepadHandler::IsActive()
{
    return this->reading;
}
