

#include "App.h"
#include <iostream>
using namespace std;

G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    (void)argc; (void)argv;
    GApp::Settings settings(argc, argv);

    // Change the window and other startup parameters by modifying the
    // settings class.  For example:
    settings.window.width       = 1280; 
    settings.window.height      = 720;

    return App(settings).run();
}

App::App(const GApp::Settings& settings) : GApp(settings) {
    renderDevice->setColorClearValue(Color3(0.2));
}

void App::onInit() {
    // Turn off the developer HUD
    createDeveloperHUD();
    debugWindow->setVisible(false);
    developerWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);
    showRenderingStats = false;
    // Set up the camera in a good position to see the entire field
    activeCamera()->setPosition(Vector3(0,60,70));
    activeCamera()->lookAt(Vector3(0,0,10), Vector3(0,1,0));
    activeCamera()->setFieldOfViewAngleDegrees(90);
    activeCamera()->setFarPlaneZ(-1000);
    // Add some simple lighting to the scene
    Array<Light::Ref> lights;
    lights.append(Light::directional("light1", Vector3(1,0.4,1), Color3(0.5)));
    lights.append(Light::directional("light2", Vector3(1,0.4,-1), Color3(0.5)));
    lights.append(Light::directional("light3", Vector3(-1,0.4,-1), Color3(0.5)));
    lights.append(Light::directional("light4", Vector3(-1,0.4,1), Color3(0.5)));
    Light::bindFixedFunction(renderDevice, lights, Color3(0.2,0.2,0.2));
    // Initialize the car
    carSize = Vector3(3.5, 3, 4);
    carRadius = 2.5;
    carMaxSpeed = 30;
    carMaxAcceleration = 150;
    carPosition = Point3(0, carSize.y/2, 45);
    carVelocity = Vector3(0,0,0);
	carFlying = false;
	//Initialize the ball
	ballRadius = 2;
	ballPosition = Point3(0,10,0);
	float random = Random().uniform(0,1.0) * pi();
	ballVelocity = Vector3(25 * cos(random),10,25 * sin(random));
}

void App::onUserInput(UserInput *input) {
    inputDirection = Vector3(0,0,0);
    if (input->keyDown(GKey::LEFT) && !carFlying)
        inputDirection.x--;
    if (input->keyDown(GKey::RIGHT) && !carFlying)
        inputDirection.x++;
    if (input->keyDown(GKey::UP) && !carFlying)
        inputDirection.z--;
    if (input->keyDown(GKey::DOWN) && !carFlying)
        inputDirection.z++;
	if (input->keyPressed(GKey('z'))){
		if(!carFlying){
			carVelocity.y += 20;
			carFlying = true;
			flyingDirection = inputDirection;
		}
	}
	if(carFlying){
		inputDirection = flyingDirection;
	}

	
    if (inputDirection.magnitude() > 0)
        inputDirection = inputDirection.unit();
    if (input->keyDown(GKey::LSHIFT) || input->keyDown(GKey::RSHIFT))
        inputDirection *= 2;

    // This returns true if the SPACEBAR was pressed
    if (input->keyPressed(GKey(' '))) {
		ballPosition = Point3(0,10,0);
		float random = Random().uniform(0,1.0) * pi();
		ballVelocity = Vector3(25.0 * cos(random),10,25.0 * sin(random));
        // This is where you "kick-off" the ball from the center of the pitch.

    }
}

void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    // An oversimplified dynamics model for the car
	/*
    Vector3 carTargetVel = carMaxSpeed*inputDirection;
    Vector3 dv = carTargetVel - carVelocity;
    if (dv.magnitude()/rdt > carMaxAcceleration)
        dv = dv.unit()*carMaxAcceleration*rdt;
	*/
	//Prior equation broken up to allow for gravitational acceleration
	//not limited by MaxSpeed or MaxAcceleration
	Vector2 carVel = Vector2(carVelocity.x,carVelocity.z);
	Vector2 inputDir = Vector2(inputDirection.x,inputDirection.z);
	Vector2 carTargetVel = carMaxSpeed*inputDir;
	Vector2 dv2 = carTargetVel - carVel;
	float magnitude = sqrt(square(dv2.x) + square(dv2.y));
	if(magnitude/rdt > carMaxAcceleration){
		dv2 = dv2.direction() * carMaxAcceleration * rdt;
	}
	carVelocity += Vector3(0,-0.5,0);
	carVelocity.x += dv2.x;
	carVelocity.z += dv2.y;
    //carVelocity += dv;
    carPosition += carVelocity*rdt;
	//dynamic model for ball
	ballVelocity += Vector3(0,-0.5,0);
	ballPosition += ballVelocity*rdt;
	//Ball/Goal collision detection
	if(ballPosition.x > -10 && ballPosition.x < 10){
		if(ballPosition.y > 0 && ballPosition.y < 10){
			if(ballPosition.z > 50 - ballRadius){
				cout << "GOAL -- You scored!";
				ballPosition = Point3(0,10,0);
				float random = Random().uniform() * pi();
				ballVelocity = Vector3(25.0 * cos(random),10,25.0 * sin(random));
			}
			else if(ballPosition.z < -50 + ballRadius){
				cout << "GOAL -- Opponent scored!";
				ballPosition = Point3(0,10,0);
				float random = Random().uniform() * pi();
				ballVelocity = Vector3(25.0 * cos(random),10,25.0 * sin(random));
			}
		}
	}
	//collision detection for ball/wall, ball/ground
	if(ballPosition.x - ballRadius < -40){
		ballPosition.x = ballRadius + -40;
		ballVelocity.x *= -1.0;
		ballVelocity.x *= .9;
		ballVelocity.z *= .9;
	}
	else if(ballPosition.x + ballRadius > 40){
		ballPosition.x = 40 - ballRadius;
		ballVelocity.x *= -1.0;
		ballVelocity *= .9;
		ballVelocity.x *= .9;
		ballVelocity.z *= .9;
	}
	if(ballPosition.y - ballRadius < 0){
		ballPosition.y = ballRadius;
		ballVelocity.y *= -1.0;
		ballVelocity.y *= .9;
		ballVelocity.x *= 0.99;
		ballVelocity.z *= 0.99;
	}
	if(ballPosition.z - ballRadius < -50){
		ballPosition.z = ballRadius + -50;
		ballVelocity.z *= -1.0;
		ballVelocity *= .9;
		ballVelocity.x *= .9;
		ballVelocity.z *= .9;
	}
	else if(ballPosition.z + ballRadius > 50){
		ballPosition.z = 50 - ballRadius;
		ballVelocity.z *= -1.0;
		ballVelocity *= .9;
		ballVelocity.x *= .9;
		ballVelocity.z *= .9;
	}
    // Handle ball/wall, car/wall, and ball/car collisions here
	//car/wall collision detection
	if(carPosition.x - (carSize.x / 2) < -40){
		carPosition.x = -40 + (carSize.x / 2);
		carVelocity.x *= -1.0;
	}
	else if(carPosition.x + (carSize.x / 2) > 40){
		carPosition.x = 40 - (carSize.x / 2);
		carVelocity.x *= -1.0;
	}
	if((carPosition.y - (carSize.y / 2) - 0.21) < 0){
		carPosition.y = carSize.y / 2 + 0.21;
		carVelocity.y = 0;
		carFlying = false;
	}
	if(carPosition.z - (carSize.z / 2) < -50){
		carPosition.z = -50 + (carSize.z / 2);
		carVelocity.z *= -1.0;
	}
	else if(carPosition.z + (carSize.z / 2) > 50){
		carPosition.z = 50 - (carSize.z / 2);
		carVelocity.z *= -1.0;
	}
	
	//Car/Ball collision physics, close enough
	float distance = sqrt(square(carPosition.x - ballPosition.x) + square(carPosition.y - ballPosition.y) + square(carPosition.z - ballPosition.z));
	if(distance < carRadius + ballRadius){
		
		//Vector3 oppoDir = ballVelocity * -0.01;
		Vector3 distanceVector = ballPosition - carPosition;
		Vector3 velocityDiff = ballVelocity - carVelocity;
		float distance = sqrt(square(carPosition.x - ballPosition.x) + square(carPosition.y - ballPosition.y) + square(carPosition.z - ballPosition.z));
		float rightSide = velocityDiff.dot(distanceVector) / (square(abs(distance)));
		Vector3 result = ballVelocity - (distanceVector * rightSide);
		ballVelocity = result * 1.1;
		ballVelocity.y += 1;
		//shifting ball outside of car after calculations, breaks collisions even more
		/*
		while(distance < carRadius + ballRadius){
			ballPosition -= oppoDir;
			distance = sqrt(square(carPosition.x - ballPosition.x) + square(carPosition.y - ballPosition.y) + square(carPosition.z - ballPosition.z));
		}
		*/
	}
}

void App::onGraphics3D(RenderDevice *rd, Array<Surface::Ref> &surface3D) {
    rd->clear();
    rd->setCullFace(CullFace::NONE); // turn off "back-face culling"    
    // Draw the car as a box
    Draw::box(Box(carPosition - carSize/2, carPosition + carSize/2),
              rd, Color3(0.2,0.4,0.8), Color3::white());
	//car shadow
	rd->setColor(Color3(0.2));
	rd->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
	float shadowScale;
	shadowScale = (15 - carPosition.y) / 15;
	if(shadowScale < 0){
		shadowScale = 0;
	}
	for(float i = 0; i < 2 * pi(); i += pi()/360.0f){
		float x = carPosition.x + carSize.x * 0.9 * shadowScale * sin(i);
		float z = carPosition.z + carSize.z * 0.9 * shadowScale * cos(i);
		rd->sendVertex(Point3(x,0.2,z));
	}
	rd->endPrimitive();
	//draw the ball
	rd->setColor(Color3(1));
	Sphere sphere(ballPosition,ballRadius);
	Draw::sphere(sphere, rd, Color3(0.5,0.5,0.5),Color3(0.5,0.5,0.5));
	//ball shadow
	rd->setColor(Color3(0.2));
	rd->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
	shadowScale = (20 - ballPosition.y) / 20;
	if(shadowScale < 0){
		shadowScale = 0;
	}
	for(float i = 0; i < 2 * pi(); i += pi()/360.0f){
		float x = ballPosition.x + ballRadius * shadowScale * sin(i);
		float z = ballPosition.z + ballRadius * shadowScale * cos(i);
		rd->sendVertex(Point3(x,0.1,z));
	}
	rd->endPrimitive();
    // Draw the ground as a quadrilateral
    rd->setColor(Color3(0.3,0.6,0.2));
    rd->setNormal(Vector3(0,1,0)); // what normal to use for lighting
    rd->beginPrimitive(PrimitiveType::QUADS);
    rd->sendVertex(Point3(-50,0,-60));
    rd->sendVertex(Point3(50,0,-60));
    rd->sendVertex(Point3(50,0,60));
    rd->sendVertex(Point3(-50,0,60));
    rd->endPrimitive();
    // Draw the borders of the playing area
    rd->setColor(Color3(1));
    rd->beginPrimitive(PrimitiveType::LINE_STRIP);
    rd->sendVertex(Point3(-40,0,-50));
    rd->sendVertex(Point3(40,0,-50));
    rd->sendVertex(Point3(40,0,50));
    rd->sendVertex(Point3(-40,0,50));
    rd->sendVertex(Point3(-40,0,-50));
    rd->endPrimitive();
    rd->beginPrimitive(PrimitiveType::LINES);
    rd->sendVertex(Point3(-40,0,-50));
    rd->sendVertex(Point3(-40,60,-50));
    rd->sendVertex(Point3(40,0,-50));
    rd->sendVertex(Point3(40,60,-50));
    rd->sendVertex(Point3(40,0,50));
    rd->sendVertex(Point3(40,60,50));
    rd->sendVertex(Point3(-40,0,50));
    rd->sendVertex(Point3(-40,60,50));
    rd->endPrimitive();
	

    // Draw the rest of the pitch markings, the goals, and the ball here
	//penalty boxes
	rd->beginPrimitive(PrimitiveType::LINE_STRIP);
	rd->sendVertex(Point3(-20,0,-50));
    rd->sendVertex(Point3(20,0,-50));
    rd->sendVertex(Point3(20,0,-33.5));
    rd->sendVertex(Point3(-20,0,-33.5));
    rd->sendVertex(Point3(-20,0,-50));
    rd->endPrimitive();
	rd->beginPrimitive(PrimitiveType::LINE_STRIP);
	rd->sendVertex(Point3(-20,0,50));
    rd->sendVertex(Point3(20,0,50));
    rd->sendVertex(Point3(20,0,33.5));
    rd->sendVertex(Point3(-20,0,33.5));
    rd->sendVertex(Point3(-20,0,50));
    rd->endPrimitive();
	//midfield line & circle
	rd->beginPrimitive(PrimitiveType::LINE_STRIP);
	rd->sendVertex(Point3(40,0,0));
	rd->sendVertex(Point3(-40,0,0));
	rd->endPrimitive();
	rd->beginPrimitive(PrimitiveType::LINE_STRIP);
	for(float i = 0; i < 2 * pi(); i += pi()/360.0f){
		float x = 9.15 * sin(i);
		float z = 9.15 * cos(i);
		rd->sendVertex(Point3(x,0,z));
	}
	rd->endPrimitive();
	//goals
	rd->setColor(Color3(0.0,0.0,0.9));
	rd->beginPrimitive(PrimitiveType::LINE_STRIP);
	rd->sendVertex(Point3(-10,0,50));
	rd->sendVertex(Point3(10,0,50));
	rd->sendVertex(Point3(10,10,50));
	rd->sendVertex(Point3(-10,10,50));
	rd->sendVertex(Point3(-10,0,50));
	rd->endPrimitive();
	for(int i = -9; i < 10; i++){
		rd->beginPrimitive(PrimitiveType::LINE_STRIP);
		rd->sendVertex(Point3(i,0,50));
		rd->sendVertex(Point3(i,10,50));
		rd->endPrimitive();
	}
	for(int i = 9; i > 0; i--){
		rd->beginPrimitive(PrimitiveType::LINE_STRIP);
		rd->sendVertex(Point3(-10,i,50));
		rd->sendVertex(Point3(10,i,50));
		rd->endPrimitive();
	}
	rd->setColor(Color3(1,0.6,0));
	rd->beginPrimitive(PrimitiveType::LINE_STRIP);
	rd->sendVertex(Point3(-10,0,-50));
	rd->sendVertex(Point3(10,0,-50));
	rd->sendVertex(Point3(10,10,-50));
	rd->sendVertex(Point3(-10,10,-50));
	rd->sendVertex(Point3(-10,0,-50));
	rd->endPrimitive();
	for(int i = -9; i < 10; i++){
		rd->beginPrimitive(PrimitiveType::LINE_STRIP);
		rd->sendVertex(Point3(i,0,-50));
		rd->sendVertex(Point3(i,10,-50));
		rd->endPrimitive();
	}
	for(int i = 9; i > 0; i--){
		rd->beginPrimitive(PrimitiveType::LINE_STRIP);
		rd->sendVertex(Point3(-10,i,-50));
		rd->sendVertex(Point3(10,i,-50));
		rd->endPrimitive();
	}
	
}
