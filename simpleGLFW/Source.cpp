// ============================================================================
//	Includes
// ============================================================================

#include <stdio.h>
#include <stdlib.h>					// malloc(), free()
#include <gl\glew.h>
#include <AR/config.h>
#include <AR/video.h>
//#include <AR/param.h>			// arParamDisp()
#include <AR/ar.h>
#include <AR/gsub_lite.h>

#include <string.h>

#include <GL/glfw3.h>

#ifdef _DEBUG
#define AR_EXT "d.lib"
#else
#define AR_EXT ".lib"
#endif
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "libAR" AR_EXT)
#pragma comment(lib, "libARgsub_lite" AR_EXT)
#pragma comment(lib, "libARvideo" AR_EXT)

char *cparam_name = "camera_para.dat";
char *vconf = "WDM_camera_flipV.xml";
char *patt_name = "markerB.pat";



static ARParam		gARTCparam;

typedef struct {
	//    SDL_Texture     *background;
	//    SDL_Window      *window;
	//    SDL_Renderer    *renderer;
	//    SDL_GLContext   *context;


	int patternId;
	double patternCenter[2];
	double patternSize;
	double patternTransform[4][4];
	unsigned short patternFound;

	int threshold;
	GLint videoPBO[1];
	GLint videoWidth, videoHeight;
	unsigned char* videoImagePtr;
} AppState;


static int setupCamera(const char *cparam_name,
	char *vconf,
	ARParam *cparam,
	AppState* appState)
{
	ARParam			wparam;

	// Open the video path.
	if (arVideoOpen(vconf) < 0) {
		fprintf(stderr, "setupCamera(): Unable to open connection to camera.\n");
		return 0;
	}

	// Find the size of the window.
	if (arVideoInqSize(&appState->videoWidth, &appState->videoHeight) < 0) return 0;
	fprintf(stdout, "Camera image size (x,y) = (%d,%d)\n", appState->videoWidth, appState->videoHeight);

	// Load the camera parameters, resize for the window and init.
	if (arParamLoad(cparam_name, 1, &wparam) < 0) {
		fprintf(stderr, "setupCamera(): Error loading parameter file %s for camera.\n", cparam_name);
		return 0;
	}


	arParamChangeSize(&wparam, appState->videoWidth, appState->videoHeight, cparam);
	fprintf(stdout, "*** Camera Parameter ***\n");
	arParamDisp(cparam);

	arInitCparam(cparam);


	if (arVideoCapStart() != 0) {
		fprintf(stderr, "setupCamera(): Unable to begin camera data capture.\n");
		return 0;
	}
	return 1;
}



// Something to look at, draw a rotating colour cube.
static void DrawCube(void)
{
	// Colour cube data.
	static GLuint polyList = 0;
	float fSize = 0.5f;
	long f, i;
	const GLfloat cube_vertices[8][3] = {
		{ 1.0, 1.0, 1.0 }, { 1.0, -1.0, 1.0 }, { -1.0, -1.0, 1.0 }, { -1.0, 1.0, 1.0 },
		{ 1.0, 1.0, -1.0 }, { 1.0, -1.0, -1.0 }, { -1.0, -1.0, -1.0 }, { -1.0, 1.0, -1.0 } };
	const GLfloat cube_vertex_colors[8][3] = {
		{ 1.0, 1.0, 1.0 }, { 1.0, 1.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 1.0, 1.0 },
		{ 1.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } };
	GLint cube_num_faces = 6;
	const short cube_faces[6][4] = {
		{ 3, 2, 1, 0 }, { 2, 3, 7, 6 }, { 0, 1, 5, 4 }, { 3, 0, 4, 7 }, { 1, 2, 6, 5 }, { 4, 5, 6, 7 } };

	if (!polyList) {

		polyList++;

		//        printf("Render!");
		//        polyList = glGenLists (1);
		//        glNewList(polyList, GL_COMPILE);
		glBegin(GL_QUADS);
		for (f = 0; f < cube_num_faces; f++)
		for (i = 0; i < 4; i++) {
			glColor3f(cube_vertex_colors[cube_faces[f][i]][0], cube_vertex_colors[cube_faces[f][i]][1], cube_vertex_colors[cube_faces[f][i]][2]);
			glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
		}
		glEnd();
		glColor3f(0.0, 0.0, 0.0);
		for (f = 0; f < cube_num_faces; f++) {
			glBegin(GL_LINE_LOOP);
			for (i = 0; i < 4; i++)
				glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
			glEnd();
		}
		//        glEndList ();
	}

	glPushMatrix(); // Save world coordinate system.
	glTranslatef(0.0, 0.0, 0.5); // Place base of cube on marker surface.
	//    glRotatef(gDrawRotateAngle, 0.0, 0.0, 1.0); // Rotate about z axis.
	//    glDisable(GL_LIGHTING);	// Just use colours.
	//    glCallList(polyList);	// Draw the cube.
	glPopMatrix();	// Restore world coordinate system.

	//    glCallList(polyList);	// Draw the cube.

}

static int renderScene(AppState* state)
{

	if (state->patternFound) {


		GLdouble projectionMatrix[16];
		GLdouble modelViewMatrix[16];

		// Projection transformation.
		arglCameraFrustumRH(&gARTCparam, 0.1, 1000.0, projectionMatrix);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(projectionMatrix);

		glMatrixMode(GL_MODELVIEW);


		// Calculate the camera position relative to the marker.
		// Replace VIEW_SCALEFACTOR with 1.0 to make one drawing unit equal to 1.0 ARToolKit units (usually millimeters).
		arglCameraViewRH(state->patternTransform, modelViewMatrix, 1.0);
		glLoadMatrixd(modelViewMatrix);

		// All lighting and geometry to be drawn relative to the marker goes here.
		DrawCube();


	} // gPatt_found


	{
		GLuint gle = glGetError();
		if (gle) {
			fprintf(stderr, "GL error 0x%x\n");
		}
	}

}

void renderBackgroundImage(AppState* state)
{

	static const GLfloat squareVertices[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, 1.0f,
	};

	static const GLfloat textureVertices[] = {
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
	};

	float width, height;

	float posX = 0, posY = 0;


	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0.0f, state->videoWidth, state->videoHeight, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();


	glActiveTexture(GL_TEXTURE0);

	//    /* this accounts for Texture2D vs TextureRectangle */
	//    glBindTexture(GL_TEXTURE_2D,state->videoTexture);

	//    glBegin(GL_QUADS);
	//    glTexCoord2f(0, 0);
	//    glVertex2f(posX,posY);
	//    glTexCoord2f(1*width, 0);
	//    glVertex2f(posX + width, posY);
	//    glTexCoord2f(1*width, 1*height);
	//    glVertex2f(posX + width, posY + height);
	//    glTexCoord2f(0, 1*height);
	//    glVertex2f(posX, posY + height);
	//    glEnd();


	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	//        if (isDepthTestOn) glEnable(GL_DEPTH_TEST);
	//        if (isLightingOn) glEnable(GL_LIGHTING);

}



static void myKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}



static void re(GLFWwindow* window, AppState* state)
{
	float ratio;
	int width, height;

	glfwGetFramebufferSize(window, &width, &height);

	ratio = width / (float)height;
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
	glBegin(GL_TRIANGLES);
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(-0.6f, -0.4f, 0.f);
	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.6f, -0.4f, 0.f);
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.6f, 0.f);
	glEnd();

	glfwSwapBuffers(window);
}


static int updateTrackerAndVideoBackground(GLFWwindow* window, AppState* state)
{

	ARMarkerInfo    *marker_info;					// Pointer to array holding the details of detected markers.
	int             marker_num;						// Count of number of markers detected.
	int             j, k;

	// Grab a video frame.
	if ((state->videoImagePtr = arVideoGetImage()) != NULL) {

		//        gCallCountMarkerDetect++; // Increment ARToolKit FPS counter.

		// Detect the markers in the video frame.
		if (arDetectMarker(state->videoImagePtr,
			state->threshold,
			&marker_info,
			&marker_num) < 0) {
			exit(-1);
		}

		glDrawBuffer(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers for new frame.

		renderBackgroundImage(state);

		// Check through the marker_info array for highest confidence
		// visible marker matching our preferred pattern.
		k = -1;
		for (j = 0; j < marker_num; j++) {
			if (marker_info[j].id == state->patternId) {
				if (k == -1) k = j; // First marker detected.
				else if (marker_info[j].cf > marker_info[k].cf) k = j; // Higher confidence marker detected.
			}
		}

		if (k != -1) {
			// Get the transformation between the marker and the real camera into gPatt_trans.
			arGetTransMat(&(marker_info[k]),
				state->patternCenter,
				state->patternSize,
				state->patternTransform);
			state->patternFound = 1;
		}
		else {
			state->patternFound = 0;
		}


		//        renderScene(state);


		fprintf(stdout, "Pattern found %d\n", state->patternFound);

	}

}


static int setupMarker(const char *patt_name, int *patt_id)
{
	// Loading only 1 pattern in this example.
	if ((*patt_id = arLoadPatt(patt_name)) < 0) {
		fprintf(stderr, "setupMarker(): pattern load error !!\n");
		return 0;
	}

	return 1;
}

int main(void)
{
	GLFWwindow* window;
	AppState appState;


	memset(&appState, 0, sizeof(AppState));


	/* Video Camera Setup */
	if (!setupCamera(cparam_name, vconf, &gARTCparam, &appState)) {
		fprintf(stderr, "main(): Unable to set up AR camera.\n");
		return -1;
	}

	/* Load marker(s). */
	if (!setupMarker(patt_name, &appState.patternId)) {
		fprintf(stderr, "main(): Unable to set up AR marker.\n");
		return -1;
	}

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "ARToolKit + GLFW", NULL, NULL);


	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, myKeyCallback);

	if (window) {

		/* Make the window's context current */
		glfwMakeContextCurrent(window);

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */

			updateTrackerAndVideoBackground(window, &appState);


			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();

			switch (glfwGetKey(window, GLFW_RELEASE)) {
			case GLFW_KEY_ESCAPE:

				fprintf(stdout, "Escape\n");

				glfwDestroyWindow(window);
				break;

			case 0:
				break;

			default:
				fprintf(stdout, "Unhandled Key\n");
			}
		}

	}

	glfwTerminate();

	return 0;
}