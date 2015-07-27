/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInteractorStyleGame.h
  Author:     Casper van Leeuwen

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkInteractorStyleGame - interactive manipulation of the camera
// .SECTION Description

#ifndef vtkInteractorStyleGame_h
#define vtkInteractorStyleGame_h

#include "vtkInteractorStyle.h"
#include <time.h>
#include "GamepadHandler.h"

class VTK_EXPORT vtkInteractorStyleGame : public vtkInteractorStyle
{
public:
  vtkTypeMacro(vtkInteractorStyleGame,vtkInteractorStyle);

  static vtkInteractorStyleGame *New();  
  void PrintSelf(ostream& os, vtkIndent indent);
  clock_t t;
  enum direction_t{ MOVE_RIGHT, MOVE_LEFT , MOVE_FORWARD, MOVE_BACKWARD};
  struct movement_t{ bool forward; bool backward; bool left; bool right;} movement;
  struct deltaMovement_t{ double x; double y;} mousedt, gamepaddt;
  struct motionfactor_t{double x; double y;} gamepadSpeed, keyboardSpeed;

  virtual void SetModelProp3D(vtkProp3D *prop);

  //struct flyState_t{bool flying; } flyState;
  // Description:
  // Event bindings controlling the effects of pressing mouse buttons
  // or moving the mouse.
  virtual void OnMouseMove();
  virtual void OnKeyPress() ;
  virtual void OnKeyRelease();
  virtual void OnTimer();
  virtual void OnChar();

  // These methods for the different interactions in different modes
  // are overridden in subclasses to perform the correct motion. Since
  // they are called by OnTimer, they do not have mouse coord parameters
  // (use interactor's GetEventPosition and GetLastEventPosition)
  virtual void CameraYaw(double dt);
  virtual void CameraPitch(double dt);
  virtual void CameraRoll(double dt);
  virtual void Pan(double dt);
  virtual void MoveToFocalPoint(double dt);
  virtual void HandleKeys(std::string key, bool down);
  virtual void handleGamepadState(gp_state* gpst);
  virtual void Rotate(double dt);
  virtual void Fly(double dt);
  virtual void FlyTo(double dt, double* destination, double* viewDir);
  virtual void Up(double dt);
  virtual void ModelRotate(double dt);

protected:
  vtkInteractorStyleGame();
  ~vtkInteractorStyleGame();
  bool turntableMode;
  bool modeButtonDown;       // Wether key used for switching mode is still pressed
  bool keyPressedDown;
  double maxSpeed;
  double gamepadLookSpeed;
  double mouseLookSpeed;
  double gamepadRoll;
  double keyboardRoll;
  bool advancedSettings;
  bool rotate;
  bool flying;
  int flyto;
  vtkProp3D* modelProp3D;
  double modelRotateSpeed;
  double *modelCenter;
  double modelRotation; // Around world Y axis

private:
  vtkInteractorStyleGame(const vtkInteractorStyleGame&);  // Not implemented.
  void operator=(const vtkInteractorStyleGame&);  // Not implemented.
  GamepadHandler* gamepad;
};

#endif