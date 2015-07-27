/*=========================================================================
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInteractorStyleGame.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkInteractorStyleGame.h"

#include "vtkCamera.h"
#include "vtkCallbackCommand.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkXOpenGLRenderWindow.h"
#include "vtkRenderer.h"
#include <vtkSmartPointer.h>
#include <math.h>
#include <vtkTransform.h>

vtkStandardNewMacro(vtkInteractorStyleGame);

//----------------------------------------------------------------------------
vtkInteractorStyleGame::vtkInteractorStyleGame()
{
  this->maxSpeed = 1;
  this->gamepadLookSpeed = 20;
  this->mouseLookSpeed = 45;
  this->keyPressedDown = false;
  this->t = clock();
  this->mousedt.x = 0;
  this->mousedt.y = 0;
  this->gamepaddt.x = 0;
  this->gamepaddt.y = 0;
  this->gamepad = new GamepadHandler();
  this->gamepadSpeed.x = 0;
  this->gamepadSpeed.y = 0;
  this->keyboardSpeed.x = 0;
  this->keyboardSpeed.y = 0;
  this->gamepadRoll = 0;
  this->keyboardRoll = 0;
  this->advancedSettings = false;
  this->rotate = false;
  this->flying = false;
  this->turntableMode = false;
  this->modeButtonDown = false;
  this->modelProp3D = NULL;
  this->modelRotation = 0.0;
  this->modelRotateSpeed = 0.0;
}

//----------------------------------------------------------------------------
vtkInteractorStyleGame::~vtkInteractorStyleGame()
{
  delete this->gamepad;
}

void vtkInteractorStyleGame::SetModelProp3D(vtkProp3D *prop)
{
    this->modelProp3D = prop;
    this->modelCenter = this->modelProp3D->GetCenter();
}


//----------------------------------------------------------------------------
void vtkInteractorStyleGame::OnMouseMove()
{
  if(this->CurrentRenderer == NULL){
    return;
  }

  vtkRenderWindowInteractor *rwi = this->Interactor;
  vtkXOpenGLRenderWindow *rw = static_cast<vtkXOpenGLRenderWindow *>(rwi->GetRenderWindow());
  int *size = rw->GetSize();
  int *eventPos = rwi->GetEventPosition();

  Window Win = rw->GetWindowId();
  Display* Disp = rw->GetDisplayId();

  if(eventPos[0] < size[0] && eventPos[1] < size[1]){
    mousedt.x += rwi->GetEventPosition()[0] - roundl(size[0]/2);
    mousedt.y += (rwi->GetEventPosition()[1] +1) - roundl(size[1]/2);
  }
  else{
    mousedt.x = 0;
    mousedt.y = 0;
  }

  // Warp mouse to center of screen to grap the mouse
  XWarpPointer(Disp, Win, Win, 0,0,size[0],size[1], roundl(size[0]/2), roundl(size[1]/2));
}

void vtkInteractorStyleGame::OnKeyPress()
{
  // Get the keypress
  vtkRenderWindowInteractor *rwi = this->Interactor;
  std::string key = rwi->GetKeySym();
  this->HandleKeys(key, true);
  this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
}


void vtkInteractorStyleGame::OnKeyRelease()
{
  // Get the keypress
  vtkRenderWindowInteractor *rwi = this->Interactor;
  std::string key = rwi->GetKeySym();
  std::cout << key << std::endl;
  this->HandleKeys(key, false);
  this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
}

void vtkInteractorStyleGame::HandleKeys(std::string key, bool down)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if (key == "w")
    this->keyboardSpeed.y = down ? this->maxSpeed: 0.0;
  else if (key == "a")
    this->keyboardSpeed.x = down ? -this->maxSpeed : 0.0;
  else if (key == "s")
    this->keyboardSpeed.y = down ? -this->maxSpeed : 0.0;
  else if (key == "d")
    this->keyboardSpeed.x = down ? this->maxSpeed : 0.0;
  else if (key == "q")
    this->keyboardRoll = down ? this->gamepadLookSpeed: 0.0;
  else if (key == "e")
    this->keyboardRoll = down ? -this->gamepadLookSpeed: 0.0;
  else if (key== "bracketright")
    this->maxSpeed = down ? std::min(++this->maxSpeed, 200.0) : this->maxSpeed;
  else if (key== "bracketleft")
    this->maxSpeed = down ? std::max(--this->maxSpeed, 0.0) : this->maxSpeed;
  else if (key == "Escape")
    rwi->ExitCallback();
  else if (key == "KP_5")
    this->advancedSettings = down ? this->advancedSettings : !this->advancedSettings;
  else if (key == "KP_Subtract" && this->advancedSettings)
    this->CurrentRenderer->GetActiveCamera()->SetViewAngle(this->CurrentRenderer->GetActiveCamera()->GetViewAngle()-1);
  else if (key == "KP_Add" && this->advancedSettings)
    this->CurrentRenderer->GetActiveCamera()->SetViewAngle(this->CurrentRenderer->GetActiveCamera()->GetViewAngle()+1);
}

void vtkInteractorStyleGame::OnChar()
{
  vtkRenderWindowInteractor *rwi = this->Interactor;

  switch (rwi->GetKeyCode())
    {
    case 'u' :
    case 'U' :
      rwi->UserCallback();
      break;

    case '3' :
      if (rwi->GetRenderWindow()->GetStereoRender())
        {
        rwi->GetRenderWindow()->StereoRenderOff();
        }
      else
        {
        rwi->GetRenderWindow()->StereoRenderOn();
        }
      rwi->Render();
      break;
    }
}

// ----------------------------------------------------------------------------
// Description:
// Timer set on each render step. Handle mouse, keyboard and gamepad movement and move the camera accordingly.
void vtkInteractorStyleGame::OnTimer()
{
    vtkRenderWindowInteractor *rwi = this->Interactor;
    vtkXOpenGLRenderWindow *rw = static_cast<vtkXOpenGLRenderWindow *>(rwi->GetRenderWindow());
    int *size = rw->GetSize();
    Window Win = rw->GetWindowId();
    Display* Disp = rw->GetDisplayId();

    if (this->gamepad->IsActive())
    {
        // Get updated gamepad state
        this->handleGamepadState(this->gamepad->getGamepadState());
    }

    double dt = ((double)(clock() - t))/CLOCKS_PER_SEC;
    t = clock();

    if(this->gamepadSpeed.y != 0 || this->keyboardSpeed.y != 0)
        this->MoveToFocalPoint(dt);
    if(this->gamepadSpeed.x != 0 || this->keyboardSpeed.x != 0)
        this->Pan(dt);

    if(this->gamepadRoll != 0 || this->keyboardRoll != 0)
        this->CameraRoll(dt);
    if(this->gamepaddt.x !=0)
        //this->ModelRotate(dt);
        this->CameraYaw(dt);
    if(this->gamepaddt.y !=0)
        this->Up(dt);
        //this->CameraPitch(dt);

    if(this->flying)
        this->Fly(dt);

    if (this->modelRotateSpeed != 0)
    {
        //printf("modelRotateSpeed = %g\n", this->modelRotateSpeed);
        this->ModelRotate(dt);
    }

    //if(this->rotate)
        //this->Rotate(dt);

    mousedt.x = 0;
    mousedt.y = 0;
    XWarpPointer(Disp, Win, Win, 0,0,size[0],size[1], roundl(size[0]/2), roundl(size[1]/2));
}

//----------------------------------------------------------------------------
// Discription:
// Handles all the gamepad interaction and translates it to movement speed and looking speed
void vtkInteractorStyleGame::handleGamepadState(gp_state* gpst)
{
    // Button 2 and 3 on the gamepad control the speed of the movement
    //if(gpst->button[2]) this->maxSpeed =std::min(++this->maxSpeed, 200.0);
    //if(gpst->button[1]) this->maxSpeed =std::max(--this->maxSpeed, 0.0);

    // All four buttons on the front pressed -> exit
    if (gpst->button[4] && gpst->button[5] && gpst->button[6] && gpst->button[7])
    {
        // Exit
        std::cout << "Magic 4-finger salute pressed, exiting!" << std::endl;
        this->Interactor->ExitCallback();
    }

    // Button 9: mode switch
    if (gpst->button[8])
    {
      if (!this->modeButtonDown)
      {
        this->turntableMode = !this->turntableMode;
        printf("Switched to %s mode\n", this->turntableMode ? "turntable" : "game");
        this->modeButtonDown = true;
      }
    }
    else
      this->modeButtonDown = false;

    if (this->turntableMode)
    {
      // Left analog stick movement which controls the movement speed.
      this->gamepadSpeed.x = (((double)gpst->axis[0])/32767)*this->maxSpeed;
      this->gamepadSpeed.y = -(((double)gpst->axis[1])/32767)*this->maxSpeed;

      // Right analog stick movement controls
    }
    else
    {
      // Game mode (strafe, fly, ...)
      if(gpst->button[0]){
         this->flyto = 1;
         this->flying = true;
      }
      if(gpst->button[1]) {
        this->flyto = 2;
        this->flying = true;
      }
      if(gpst->button[2]) {
        this->flyto = 3;
        this->flying = true;
      }
      if(gpst->button[3]){
        this->flyto = 4;
        this->flying = true;
      }

      if(gpst->button[9])
          this->rotate = true;
      else
          this->rotate = false;

      // Left analog stick: movement which controls the movement speed.
      this->gamepadSpeed.x = (((double)gpst->axis[0])/32767)*this->maxSpeed;
      this->gamepadSpeed.y = -(((double)gpst->axis[1])/32767)*this->maxSpeed;

      // Right analog stick: controls the looking speed
      this->gamepaddt.x = -(((double)gpst->axis[2])/32767);
      this->gamepaddt.y = -(((double)gpst->axis[3])/32767);

      // Overwrite analog stick if arrows are pushed
      if(gpst->axis[4]){
        this->gamepadSpeed.x = (((double)gpst->axis[5])/32767) * this->maxSpeed;
      }
      else if(gpst->axis[5]){
        this->gamepadSpeed.x = -(((double)gpst->axis[6])/32767) * this->maxSpeed;
      }
      else
        this->modelRotateSpeed = 0.0;

      // The shoulder buttons control the roll of the camera
      if (gpst->button[4] && gpst->button[5])
          this->modelRotateSpeed = 0;
      else if (gpst->button[4])
          this->modelRotateSpeed = -this->maxSpeed;
      else if(gpst->button[5])
          this->modelRotateSpeed = this->maxSpeed;
      else
          this->modelRotateSpeed = 0;
    }

    //vtkRenderWindowInteractor *rwi = this->Interactor;
    //vtkXOpenGLRenderWindow *rw = static_cast<vtkXOpenGLRenderWindow *>(rwi->GetRenderWindow());
    //if(gpst->button[9]) setFullScreen(rw);
}

void vtkInteractorStyleGame::ModelRotate(double dt)
{
  if (this->CurrentRenderer == NULL){
    return;
  }

  //double speed = -100 * gamepaddt.x * dt;

  // World axis (pivot is origin)
  //this->modelProp3D->RotateWXYZ(speed, 0, 1, 0);
  // Local model axis
  //this->modelProp3D->RotateY(speed);

  //this->modelRotation += speed;
  this->modelRotation += this->modelRotateSpeed * 3;

  vtkSmartPointer<vtkTransform> xform = vtkSmartPointer<vtkTransform>::New();
  xform->Translate(this->modelCenter[0], this->modelCenter[1], this->modelCenter[2]);
  xform->RotateY(this->modelRotation);
  xform->Translate(-this->modelCenter[0], -this->modelCenter[1], -this->modelCenter[2]);

  this->modelProp3D->SetUserTransform(xform.Get());
}

//----------------------------------------------------------------------------
// Discription:
// Called every timestep from ontimer to yaw the camera based on the mouse, keyboard and gamepad movement.
void vtkInteractorStyleGame::CameraYaw(double dt)
{
  if (this->CurrentRenderer == NULL){
    return;
  }

  int *size = this->CurrentRenderer->GetRenderWindow()->GetSize();

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double delta_Yaw = -this->mouseLookSpeed / size[0];
  double mouseyaw = mousedt.x * delta_Yaw;

  double gamepadYaw = gamepaddt.x*this->gamepadLookSpeed*dt;

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->Yaw(mouseyaw + gamepadYaw);
  camera->OrthogonalizeViewUp();

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
}

//----------------------------------------------------------------------------
// Discription:
// Called every timestep from ontimer to pitch the camera based on the mouse, keyboard and gamepad movement.
void vtkInteractorStyleGame::CameraPitch(double dt)
{
  if (this->CurrentRenderer == NULL)
  {
    return;
  }

  int *size = this->CurrentRenderer->GetRenderWindow()->GetSize();

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double delta_Pitch = this->mouseLookSpeed / size[1];
  double mousePitch = mousedt.y * delta_Pitch;

  double gamepadPitch = gamepaddt.y*this->gamepadLookSpeed*dt;

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->Pitch(mousePitch + gamepadPitch);
  camera->OrthogonalizeViewUp();

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
}

//----------------------------------------------------------------------------
// Discription:
// Called every timestep from ontimer to roll the camera based on the mouse, keyboard and gamepad movement.
void vtkInteractorStyleGame::CameraRoll(double dt)
{
  if (this->CurrentRenderer == NULL)
  {
    return;
  }

  double roll = (keyboardRoll + gamepadRoll)*dt;

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->Roll(roll);
  camera->OrthogonalizeViewUp();

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
}

//----------------------------------------------------------------------------
// Discription:
// Called every timestep from ontimer to pan the camera based on the mouse, keyboard and gamepad movement.
void vtkInteractorStyleGame::Pan(double dt)
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double viewFocus[3], viewPoint[3], viewUp[3];
  double motionVector[3];
  double dirOfProjection[3];
  double motiondelta = 0;

  // Calculate the focal depth since we'll be using it a lot

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

  double speed = keyboardSpeed.x + gamepadSpeed.x;
  speed = speed > this->maxSpeed ? this->maxSpeed : speed < -this->maxSpeed ? -this->maxSpeed : speed;

  motiondelta=dt*speed;

  camera->GetDirectionOfProjection(dirOfProjection);
  camera->GetFocalPoint(viewFocus);
  camera->GetPosition(viewPoint);
  camera->GetViewUp(viewUp);

  vtkMath::Cross(dirOfProjection, viewUp, motionVector);
  vtkMath::Normalize(motionVector);

  camera->SetFocalPoint((motionVector[0]*motiondelta) + viewFocus[0],
                        (motionVector[1]*motiondelta) + viewFocus[1],
                        (motionVector[2]*motiondelta) + viewFocus[2]);

  camera->SetPosition((motionVector[0]*motiondelta) + viewPoint[0],
                      (motionVector[1]*motiondelta) + viewPoint[1],
                      (motionVector[2]*motiondelta) + viewPoint[2]);

  if (rwi->GetLightFollowCamera())
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

  if (this->AutoAdjustCameraClippingRange)
    {
    this->CurrentRenderer->ResetCameraClippingRange();
    }
}

//----------------------------------------------------------------------------
// Discription:
// Called every timestep from ontimer to move the camera in the direction of projection based on the mouse, keyboard and gamepad movement.
void vtkInteractorStyleGame::MoveToFocalPoint(double dt)
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double viewFocus[3], viewPoint[3], viewUp[3];
  double dirOfProjection[3];
  double motiondelta = 0;

  // Calculate the focal depth since we'll be using it a lot

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

  double speed = keyboardSpeed.y +gamepadSpeed.y;
  speed = speed > this->maxSpeed ? this->maxSpeed : speed < -this->maxSpeed ? -this->maxSpeed : speed;

  motiondelta = dt*speed;
  // XXX always seems to be 0, 0, -1
  camera->GetDirectionOfProjection(dirOfProjection);
  camera->GetFocalPoint(viewFocus);
  camera->GetPosition(viewPoint);
  camera->GetViewUp(viewUp);

    //printf("viewpoint = %f, %f, %f\n", viewPoint[0], viewPoint[1], viewPoint[2]);
    //printf("viewfocus = %f, %f, %f\n", viewFocus[0], viewFocus[1], viewFocus[2]);
    //printf("dirOfProjection = %f, %f, %f\n", dirOfProjection[0], dirOfProjection[1], dirOfProjection[2]);

  camera->SetFocalPoint((dirOfProjection[0]*motiondelta) + viewFocus[0],
                        (dirOfProjection[1]*motiondelta) + viewFocus[1],
                        (dirOfProjection[2]*motiondelta) + viewFocus[2]);

  camera->SetPosition((dirOfProjection[0]*motiondelta) + viewPoint[0],
                      (dirOfProjection[1]*motiondelta) + viewPoint[1],
                      (dirOfProjection[2]*motiondelta) + viewPoint[2]);

  if (rwi->GetLightFollowCamera())
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

  if (this->AutoAdjustCameraClippingRange)
    {
    this->CurrentRenderer->ResetCameraClippingRange();
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleGame::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "MaxSpeed: " << this->maxSpeed << "\n";
}

//----------------------------------------------------------------------------
void vtkInteractorStyleGame::Rotate(double dt)
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  int dx = 100*dt;

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->Azimuth(dx);
  camera->OrthogonalizeViewUp();

  if (this->AutoAdjustCameraClippingRange)
    {
    this->CurrentRenderer->ResetCameraClippingRange();
    }

  if (rwi->GetLightFollowCamera())
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
}

void vtkInteractorStyleGame::Up(double dt)
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

  double viewFocus[3], viewPoint[3];

  double speed = gamepaddt.y;
  speed = speed > this->maxSpeed ? this->maxSpeed : speed < -this->maxSpeed ? -this->maxSpeed : speed;

  double dy = speed*dt;

  camera->GetFocalPoint(viewFocus);
  camera->GetPosition(viewPoint);

  camera->SetFocalPoint(viewFocus[0], viewFocus[1] + dy, viewFocus[2]);
  camera->SetPosition(viewPoint[0], viewPoint[1] + dy, viewPoint[2]);

  if (rwi->GetLightFollowCamera())
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

  if (this->AutoAdjustCameraClippingRange)
    {
    this->CurrentRenderer->ResetCameraClippingRange();
    }
}

void vtkInteractorStyleGame::Fly(double dt)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  double dest[3];
  double viewdir[3];
  switch(this->flyto){
    case 1:
      dest[0] = 0;
      dest[1] = 0;
      dest[2] = 0;
      viewdir[0] = 1;
      viewdir[1] = 0;
      viewdir[2] = 0;
      this->FlyTo(dt, dest, viewdir);
    break;
    case 2:
      dest[0] = 0;
      dest[1] = 1;
      dest[2] = 0;
      viewdir[0] = 0;
      viewdir[1] = -1;
      viewdir[2] = 0;
      this->FlyTo(dt, dest, viewdir);
    break;
    case 3:
      dest[0] = 0;
      dest[1] = 0;
      dest[2] = 1;
      viewdir[0] = 0;
      viewdir[1] = 0;
      viewdir[2] = -1;
      this->FlyTo(dt, dest, viewdir);
    break;
    case 4:
      dest[0] = 1;
      dest[1] = 0;
      dest[2] = 0;
      viewdir[0] = -1;
      viewdir[1] = 0;
      viewdir[2] = 0;
      this->FlyTo(dt, dest, viewdir);
    break;
    default:;
  }
}

void vtkInteractorStyleGame::FlyTo(double dt, double* destination, double* viewDir)
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double focalPoint[3], camposition[3], viewUp[3];
  double dirOfProjection[3];
  double motionvector[3];
  double motiondelta = dt;
  bool transdone = false;
  bool rotdone = false;

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->GetDirectionOfProjection(dirOfProjection);
  camera->GetFocalPoint(focalPoint);
  camera->GetPosition(camposition);
  camera->GetViewUp(viewUp);

  motionvector[0] = destination[0] -  camposition[0];
  motionvector[1] = destination[1] -  camposition[1];
  motionvector[2] = destination[2] -  camposition[2];

  if(vtkMath::Norm(motionvector) > 0.1)
  {
    vtkMath::Normalize(motionvector);

    camera->SetFocalPoint((motionvector[0]*motiondelta/2) + focalPoint[0],
                          (motionvector[1]*motiondelta/2) + focalPoint[1],
                          (motionvector[2]*motiondelta/2) + focalPoint[2]);

    camera->SetPosition((motionvector[0]*motiondelta/2) + camposition[0],
                        (motionvector[1]*motiondelta/2) + camposition[1],
                        (motionvector[2]*motiondelta/2) + camposition[2]);

    camera->OrthogonalizeViewUp();
  } else {
    transdone = true;
  }

  camera->GetDirectionOfProjection(dirOfProjection);
  camera->GetFocalPoint(focalPoint);
  camera->GetPosition(camposition);
  camera->GetViewUp(viewUp);

  double axis[3];
  vtkMath::Cross(dirOfProjection, viewDir, axis);

  if(vtkMath::Norm(axis) > 0.1)
  {
    vtkMath::Normalize(axis);
    vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
    // translate the camera to the origin,
    // rotate about axis,
    // translate back again
    trans->Translate(+camposition[0],+camposition[1],+camposition[2]);
    trans->RotateWXYZ(motiondelta*50,axis);
    trans->Translate(-camposition[0],-camposition[1],-camposition[2]);

    trans->TransformPoint(focalPoint,focalPoint);
    camera->SetFocalPoint(focalPoint);
    camera->OrthogonalizeViewUp();
  }
  else{
    rotdone = true;
  }

  this->flying = !(transdone & rotdone);

  if (rwi->GetLightFollowCamera())
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
}

