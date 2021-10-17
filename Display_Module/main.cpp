
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <map>

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
	#include <unistd.h>
	#include <sys/time.h>
#elif defined(WIN32)
	#include <Windows.h>
	#include <tchar.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
	#include <unistd.h>
	#include <sys/time.h>
#endif


#include "Camera.hpp"
#include "Ground.hpp"
#include "KeyManager.hpp"

#include "Shape.hpp"
#include "Vehicle.hpp"
#include "MyVehicle.hpp"

#include "Messages.hpp"
#include "HUD.hpp"

/**************************** Your code begin *****************************/
#include <SMStructs.h>
#include <SMObject.h>

void drawGPS();
void drawString(const char* str);
void selectFont(int size, int charset, const char* face);
void drawLaser();
/**************************** Your code end *****************************/

void display();
void reshape(int width, int height);
void idle();

void keydown(unsigned char key, int x, int y);
void keyup(unsigned char key, int x, int y);
void special_keydown(int keycode, int x, int y);
void special_keyup(int keycode, int x, int y);

void mouse(int button, int state, int x, int y);
void dragged(int x, int y);
void motion(int x, int y);

using namespace std;
using namespace scos;

// Used to store the previous mouse location so we
//   can calculate relative mouse movement.
int prev_mouse_x = -1;
int prev_mouse_y = -1;

// vehicle control related variables
Vehicle * vehicle = NULL;
double speed = 0;
double steering = 0;

//ProcessManagement* ProcessManagementPtr = NULL;
//SM_Laser* SM_LaserPtr = NULL;
//SM_GPS* SM_GPSPtr = NULL;
//SM_VehicleControl* SM_VehicleControlPtr = NULL;
//int cnt_ProcessManagement = 0;
//int MAX_WAITBEAT = 100;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
//int _tmain(int argc, _TCHAR* argv[]) {
int main(int argc, char ** argv) {



	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - GL");

	Camera::get()->setWindowDimensions(WINDOW_WIDTH, WINDOW_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(special_keydown);
	glutSpecialUpFunc(special_keyup);

	glutMouseFunc(mouse);
	glutMotionFunc(dragged);
	glutPassiveMotionFunc(motion);


	/**************************** Your code begin *****************************/

	///* Open existed SharedMemory Object, no need to create a new one.*/
	//SMObject ProcessManagementObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	//ProcessManagementObj.SMAccess();// Open the existed handle of SMO
	//ProcessManagementPtr = (ProcessManagement*)ProcessManagementObj.pData;

	//SMObject SM_LaserObj(_TEXT("SM_Laser"), sizeof(SM_Laser));
	//SM_LaserObj.SMAccess();
	//SM_LaserPtr = (SM_Laser*)SM_LaserObj.pData;

	//SMObject SM_GPSObj(_TEXT("SM_GPS"), sizeof(SM_GPS));
	//SM_GPSObj.SMAccess();
	//SM_GPSPtr = (SM_GPS*)SM_GPSObj.pData;

	//SMObject SM_VehicleControlObj(_TEXT("SM_VehicleControl"), sizeof(SM_VehicleControl));
	//SM_VehicleControlObj.SMAccess();
	//SM_VehicleControlPtr = (SM_VehicleControl*)SM_VehicleControlObj.pData;

		
	/***************************** Your code end *****************************/


	// -------------------------------------------------------------------------
	// Please uncomment the following line of code and replace 'MyVehicle'
	//   with the name of the class you want to show as the current 
	//   custom vehicle.
	// -------------------------------------------------------------------------
	vehicle = new MyVehicle();


	glutMainLoop();

	if (vehicle != NULL) {
		delete vehicle;
	}

	return 0;
}

/**************************** Your code begin *****************************/
bool heartbeats()
{
	//if (ProcessManagementPtr->Heartbeat.Flags.Camera == 0b0)
	//{
	//	/* Heart beats */
	//	ProcessManagementPtr->Heartbeat.Flags.Camera == 0b1;
	//	cnt_ProcessManagement = 0;
	//}
	//else
	//{
	//	if (cnt_ProcessManagement++ > MAX_WAITBEAT)
	//	{
	//		ProcessManagementPtr->Shutdown.Flags.Camera = 0b1;
	//		return false;
	//	}
	//}
	return true;
}

void drawGPS()
{
	double northing = vehicle->getX() + 90000000;
	double easting = vehicle->getZ() + 500000;
	unsigned int height = vehicle->getY();

	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	vehicle->positionInGL();
	char* str = NULL;
	str = new char[30];
	sprintf(str, "(%.2f, %.2f, %d)", northing-90000000, easting - 500000,height);
	glRasterPos3f(0, 1.0, 0);// car: xOz plane, z: height
	selectFont(20, ANSI_CHARSET, "Times New Roman");
	drawString(str);
	glPopMatrix();
	delete[]str;
}

// ASCII字符总共只有0到127，一共128种字符
#define MAX_CHAR       128
void drawString(const char* str)
{
	static int isFirstCall = 1;
	static GLuint lists;

	if (isFirstCall) { // 如果是第一次调用，执行初始化
						// 为每一个ASCII字符产生一个显示列表
		isFirstCall = 0;

		// 申请MAX_CHAR个连续的显示列表编号
		lists = glGenLists(MAX_CHAR);

		// 把每个字符的绘制命令都装到对应的显示列表中
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	// 调用每个字符对应的显示列表，绘制每个字符
	for (; *str != '\0'; ++str)
		glCallList(lists + *str);
}

void selectFont(int size, int charset, const char* face) {
	HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
	HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	DeleteObject(hOldFont);
}

void drawLaser()
{
	
	glPushMatrix();
	vehicle->positionInGL();
	//glTranslated(0.5, 0, 0);
	glColor3f(1.0, 0, .5);
	glBegin(GL_LINES);

	glVertex3f(0, 1.0, 0);

	glVertex3f(1000/1000, 1.0, 1000/1000);
	glEnd();
	glPopMatrix();
}
//void drawGPS(double northing, double easting, double height)
//{
//
//
//	//glPushMatrix	 https://docs.microsoft.com/zh-cn/windows/win32/opengl/glpushmatrix
//	//glTranslated   https://docs.microsoft.com/zh-cn/windows/win32/opengl/gltranslated
//	Camera::get()->switchTo2DDrawing();
//	int Half_winWidth = (Camera::get()->getWindowWidth()) * .5;
//	//int Half_winHeight = (Camera::get()->getWindowHeight()) * .5;
// 
//	glPushMatrix();
//	glTranslatef(Half_winWidth, 0, 0);// 移动到正中心
//	//glDisable(GL_LIGHTING);
//	
//	//绘制中心线
//	glColor3f(.5, .5, .5);
//	glLineStipple(1, 0x0F0F);
//	glEnable(GL_LINE_STIPPLE);
//	glBegin(GL_LINES);
//	
//	glVertex2f(0, 0);
//	glVertex2f(0, 60);
//	glEnd();
//	glDisable(GL_LINE_STIPPLE);
//	glPopMatrix();
//	Camera::get()->switchTo3DDrawing();
//
//	//Camera::get()->switchTo2DDrawing();
//	//int winWidthOff = (Camera::get()->getWindowWidth() - 800) * .5;
//	//if (winWidthOff < 0)
//	//	winWidthOff = 0;
//
//	//if (vehicle) {
//	//	glColor3f(0, 1, 0);
//	//	HUD::DrawGauge(winWidthOff, 480, 210, -1, 1, vehicle->getSpeed(), "Speed");
//	//}
//
//	//Camera::get()->switchTo3DDrawing();
//
//
//}


/***************************** Your code end *****************************/

void display() {
	// -------------------------------------------------------------------------
	//  This method is the main draw routine. 
	// -------------------------------------------------------------------------

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(Camera::get()->isPursuitMode() && vehicle != NULL) {
		double x = vehicle->getX(), y = vehicle->getY(), z = vehicle->getZ();
		double dx = cos(vehicle->getRotation() * 3.141592765 / 180.0);
		double dy = sin(vehicle->getRotation() * 3.141592765 / 180.0);
		Camera::get()->setDestPos(x + (-3 * dx), y + 7, z + (-3 * dy));
		Camera::get()->setDestDir(dx, -1, dy);
	}
	Camera::get()->updateLocation();
	Camera::get()->setLookAt();

	Ground::draw();
	
	// draw my vehicle

	/**************************** Your code begin *****************************/
	//vehicle->setX(SM_GPSPtr->northing);
	//vehicle->setZ(SM_GPSPtr->easting);
	//vehicle->setY(SM_GPSPtr->height);
	/**************************** Your code end *****************************/

	if (vehicle != NULL) {
		vehicle->draw();
	}


	// draw HUD
	HUD::Draw();

	drawGPS();
	drawLaser();
	glutSwapBuffers();
};

void reshape(int width, int height) {

	Camera::get()->setWindowDimensions(width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
};

double getTime()
{
#if defined(WIN32)
	LARGE_INTEGER freqli;
	LARGE_INTEGER li;
	if(QueryPerformanceCounter(&li) && QueryPerformanceFrequency(&freqli)) {
		return double(li.QuadPart) / double(freqli.QuadPart);
	}
	else {
		static ULONGLONG start = GetTickCount64();
		return (GetTickCount64() - start) / 1000.0;
	}
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec + (t.tv_usec / 1000000.0);
#endif
}

void idle() {
	///**************************** Your code begin *****************************/
	//if (!heartbeats()) exit(0);// Heatbeats failed, exit the whole process.
	///**************************** Your code end *****************************/



	if (KeyManager::get()->isAsciiKeyPressed('a')) {
		Camera::get()->strafeLeft();
	}

	if (KeyManager::get()->isAsciiKeyPressed('c')) {
		Camera::get()->strafeDown();
	}

	if (KeyManager::get()->isAsciiKeyPressed('d')) {
		Camera::get()->strafeRight();
	}

	if (KeyManager::get()->isAsciiKeyPressed('s')) {
		Camera::get()->moveBackward();
	}

	if (KeyManager::get()->isAsciiKeyPressed('w')) {
		Camera::get()->moveForward();
	}

	if (KeyManager::get()->isAsciiKeyPressed(' ')) {
		Camera::get()->strafeUp();
	}

	speed = 0;
	steering = 0;

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_LEFT)) {
		steering = Vehicle::MAX_LEFT_STEERING_DEGS * -1;   
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_RIGHT)) {
		steering = Vehicle::MAX_RIGHT_STEERING_DEGS * -1;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_UP)) {
		speed = Vehicle::MAX_FORWARD_SPEED_MPS;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_DOWN)) {
		speed = Vehicle::MAX_BACKWARD_SPEED_MPS;
	}

	const float sleep_time_between_frames_in_seconds = 0.025;

	static double previousTime = getTime();
	const double currTime = getTime();
	const double elapsedTime = currTime - previousTime;
	previousTime = currTime;

	// do a simulation step
	if (vehicle != NULL) {
		vehicle->update(speed, steering, elapsedTime);
	}

	display();

#ifdef _WIN32 
	Sleep(sleep_time_between_frames_in_seconds * 1000);
#else
	usleep(sleep_time_between_frames_in_seconds * 1e6);
#endif
};

void keydown(unsigned char key, int x, int y) {

	// keys that will be held down for extended periods of time will be handled
	//   in the idle function
	KeyManager::get()->asciiKeyPressed(key);

	// keys that react ocne when pressed rather than need to be held down
	//   can be handles normally, like this...
	switch (key) {
	case 27: // ESC key
		exit(0);
		break;      
	case '0':
		Camera::get()->jumpToOrigin();
		break;
	case 'p':
		Camera::get()->togglePursuitMode();
		break;
	}

};

void keyup(unsigned char key, int x, int y) {
	KeyManager::get()->asciiKeyReleased(key);
};

void special_keydown(int keycode, int x, int y) {

	KeyManager::get()->specialKeyPressed(keycode);

};

void special_keyup(int keycode, int x, int y) {  
	KeyManager::get()->specialKeyReleased(keycode);  
};

void mouse(int button, int state, int x, int y) {

};

void dragged(int x, int y) {

	if (prev_mouse_x >= 0) {

		int dx = x - prev_mouse_x;
		int dy = y - prev_mouse_y;

		Camera::get()->mouseRotateCamera(dx, dy);
	}

	prev_mouse_x = x;
	prev_mouse_y = y;
};

void motion(int x, int y) {

	prev_mouse_x = x;
	prev_mouse_y = y;
};