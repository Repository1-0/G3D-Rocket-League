

#ifndef App_h
#define App_h

#include <G3D/G3DAll.h>

class App : public GApp {
public:
    
    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit();
    virtual void onUserInput(UserInput *uinput); 
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt);
    virtual void onGraphics3D(RenderDevice* rd, Array<Surface::Ref> &surface);

protected:

    Point3 carPosition;
    Vector3 carVelocity;
    Vector3 carSize;
	bool carFlying;
    float carRadius;
    float carMaxSpeed, carMaxAcceleration;

	Point3 ballPosition;
	Vector3 ballVelocity;
	Vector3 ballSize;
	float ballRadius;
	float ballMaxSpeed;

    Vector3 inputDirection;
	Vector3 flyingDirection;
};

#endif
