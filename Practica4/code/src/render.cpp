#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <cassert>
#include <vector>
#include <iostream>
#include <string>
#include <imgui\imgui.h>

#include "GL_framework.h"

#define DEBUG_LOG false

extern bool loadOBJ(const char * path, std::vector < glm::vec3 > & out_vertices, std::vector < glm::vec2 > & out_uvs, std::vector < glm::vec3 > & out_normals);


///////// Forward declarations
namespace ImGui {
	void Render();
}

namespace RenderVars {
	const float FOV = glm::radians(65.f);
	const float zNear = 0.2f;
	const float zFar = 500.f;										//Far plane (how far the camera can see)
	const float cameraSeparation = 10.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP = _modelView;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, 0.f, -20.f };							//Change the '-50f' if camera has to start further or closer
	float rota[2] = { 0.f, 0.f };
	void setPosition(glm::vec3 newPos) {
		panv[0] = newPos.x;
		panv[1] = newPos.y;
		panv[2] = newPos.z;
	}
	glm::vec3 getPosition() {
		return glm::vec3(panv[0], panv[1], panv[2]);
	}
	int cameraMode = 0;
	

	void resetRotation() {
		rota[0] = 0;
		rota[1] = 0;
	}
}
namespace RV = RenderVars;

//Custom namespaces
namespace Lights {

}

namespace Scene {
	int drawingMethod = 0;

	void renderUI() {
		int* option = new int();
		ImGui::Begin("Parameters");
		ImGui::Separator();
		ImGui::RadioButton("Looped", option, 0);
		ImGui::RadioButton("Instanced", option, 1);
		ImGui::RadioButton("MultiDrawInstanced", option, 2);
		ImGui::End();
	}

	}


namespace models3D {
	struct model
	{
	public:
		std::vector< glm::vec3 > vertices;
		std::vector< glm::vec2 > uvs;
		std::vector< glm::vec3 > normals;
		glm::vec3 color;
	    glm::mat4 objMat = glm::mat4(1.f);

		GLuint vao;
		GLuint vbo[2];					//2: Vertex positions and normals
		GLuint shaders[3];
		GLuint program, flatProgram;
	};

	model create(std::string modelName, glm::vec3 position, float scale, glm::vec3 initColor = glm::vec3(0.7f, 0.65f, 0.34f));
	void cleanup(model aModel);
	void draw(model aModel);
	void drawFlat(model aModel);
	model dolphin, tuna, golden_fish, whale,sun;
}


////////////////


void GLResize(int width, int height) {
	glViewport(0, 0, width, height);
	if(height != 0) RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	else RV::_projection = glm::perspective(RV::FOV, 0.f, RV::zNear, RV::zFar);
}

void GLmousecb(MouseEvent ev) {
	if(RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) {
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch(ev.button) {
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	} else {
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}

void GLinit(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(RV::FOV, (float)width/(float)height, RV::zNear, RV::zFar);

	models3D::golden_fish = models3D::create("models/golden_fish.obj",	glm::vec3(0, 0, 0),	1.f, glm::vec3(0.97f,0.53f,0.23f));
	models3D::whale = models3D::create("models/whale.obj", glm::vec3(0, 0, 0), 1.f, glm::vec3(0.1f, 0.0f, 0.1f));
	//models3D::tuna = models3D::create(	"models/tuna.obj",	glm::vec3(0, 0, 0),	0.003f, glm::vec3(1.f,0.0f,1.f));

	//models3D::sun = models3D::create("models/sun.obj",	glm::vec3(0, 0, 0), 0.3f, glm::vec3(0));
	//models3D::provaModel = models3D::create("treeTriangulated.obj", glm::vec3(0, 0, 0), 0.2f);
}

void GLcleanup() {
	/*models3D::cleanup(models3D::dolphin);
	models3D::cleanup(models3D::tuna);*/
	//models3D::cleanup(models3D::sun);
	models3D::cleanup(models3D::whale);
	models3D::cleanup(models3D::golden_fish);
}

void GLrender(double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Scene::renderUI();


	RV::_modelView = glm::mat4();
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));
	RV::_MVP = RV::_projection * RV::_modelView;





	float modelSeparation = 0.2f;

	switch (Scene::drawingMethod)
	{
	case 0:												//dibuixar usant el m�tode antic
		for (int x = 0; x < 200; x++)
		{
				//models3D::whale.objMat = glm::translate(glm::mat4(), glm::vec3(0, 0, -20));
				models3D::drawFlat(models3D::whale);
				models3D::drawFlat(models3D::golden_fish);

				//models3D::tuna.objMat = glm::translate(glm::mat4(), glm::vec3(0,0,-20));
				//models3D::drawFlat(models3D::tuna);
				//models3D::drawFlat(models3D::dolphin);



		}
		break;
	case 1:												//dibuixar instanciant

		break;
	case 2:												//dibuixar MultiDrawIndirect

		break;
	default:
		break;
	}

	ImGui::Render();
}


//////////////////////////////////// COMPILE AND LINK
GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name="") {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
void linkProgram(GLuint program) {
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}


//////////////////////////////////////////////// LOADED OBJECTS
namespace models3D {

	const char* models3D_vertShader =
		"#version 330\n\
		in vec3 in_Position;\n\
		in vec3 in_Normal;\n\
		uniform mat4 objMat;\n\
		uniform mat4 mv_Mat;\n\
		uniform mat4 mvpMat;\n\
		void main() {\n\
			gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);											\n\
			//vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);											\n\
																											\n\
		}";

	const char* models3D_fragShader =
		"#version 330\n\
		out vec4 out_Color;\n\
		uniform vec4 color;\n\
		in float lightDifuse;\n\
		void main() {\n\
			out_Color = vec4(color.xyz*0.5+color.xyz*lightDifuse, 1.0 );\n\
		}";

	//For drawing objects with a simple flat color.
	const char* models3D_flatShader =
		"#version 330										\n\
		out vec4 out_Color;									\n\
		uniform vec4 color;									\n\
		void main() {										\n\
			out_Color = vec4(color.xyz, 1.0 );				\n\
		}";

	//Can draw either with phong difuse shading or with toon shading
	const char* models3D_toonShader =
		"#version 330										\n\
		in vec4 vert_Normal;								\n\
		out vec4 out_Color;									\n\
		uniform mat4 mv_Mat;								\n\
		uniform vec4 color;									\n\
		uniform vec3 ambientColor;							\n\
		uniform float ambientStrenght;						\n\
		uniform float light1Brightnes;						\n\
		uniform float lightBulbBrightnes;					\n\
		uniform vec3 lightColor1;							\n\
		uniform vec3 lightColor2;							\n\
		uniform vec3 lightColor3;							\n\
		uniform int toonShaderOption;						\n\
		in float lightDifuse;								\n\
		in float lightDifuse2;								\n\
		in float lightDifuse3;								\n\
															\n\
		float toonValue(in float difuse){					\n\
			if(difuse < 0.2) {																												\n\
				return 0.;																													\n\
			} else if(difuse > 0.2 && difuse < 0.4) {																						\n\
				return 0.2;																													\n\
			} else if(difuse > 0.4 && difuse < 0.5) {																						\n\
				return 0.4;																													\n\
			} else {																														\n\
				return 1.;																													\n\
			}																																\n\
		}																																	\n\
		void main() {																														\n\
			float toonDifuse1 = 0;			// Sun																							\n\
			float toonDifuse2 = 0;			// Moon																							\n\
			float toonDifuse3 = 0;			// Light bulb																					\n\
			if(toonShaderOption == 0) {																										\n\
				toonDifuse1 = lightDifuse*light1Brightnes;																					\n\
				toonDifuse2 = lightDifuse2*1800;																							\n\
				toonDifuse3 = lightDifuse3*lightBulbBrightnes;																				\n\
			} else if(toonShaderOption == 1) { 																								\n\
 				toonDifuse1 = toonValue(lightDifuse*light1Brightnes);																		\n\
				toonDifuse3 = toonValue(lightDifuse3*lightBulbBrightnes);																	\n\
			} else if(toonShaderOption == 2) { 																								\n\
				toonDifuse1 = toonValue(lightDifuse*light1Brightnes);																		\n\
				toonDifuse2 = toonValue(lightDifuse2*1800);																					\n\
			} else if(toonShaderOption == 3) { 																								\n\
				toonDifuse2 = toonValue(lightDifuse2*1800);																					\n\
				toonDifuse3 = toonValue(lightDifuse3*lightBulbBrightnes);																	\n\
			}																																\n\
			out_Color = vec4(color.xyz*lightColor1*toonDifuse1 + color.xyz*lightColor2*toonDifuse2  + color.xyz*lightColor3*toonDifuse3, 1.0 ); \n\
			if(toonShaderOption == 0){			//Ambient light only available when toon shading is disabled								\n\
				out_Color += vec4(color.xyz*ambientStrenght*ambientColor,0.0);																\n\
			}																																\n\
		}";

	model create(std::string modelName, glm::vec3 position, float scale, glm::vec3 initColor) {
		model newModel;
		newModel.color = initColor;
		newModel.objMat = glm::scale(glm::mat4(), glm::vec3(scale));
		//TODO: change pos

		bool res = loadOBJ(modelName.c_str(), newModel.vertices, newModel.uvs, newModel.normals);

		if (DEBUG_LOG) //Posar a true si es vol debugar els vertexs
		{
			for (int i = 0; i < newModel.vertices.size(); i++)
			{
				std::cout << "vert" << i << ": " << newModel.vertices.at(i).x << "/" << newModel.vertices.at(i).y << "/" << newModel.vertices.at(i).z << std::endl;
			}
		}
		if (res)
			std::cout << "Model "+ modelName+" was loaded succesfully \n";
		else
			std::cout << "ERROR AT LOADING OBJECT \n";

		//Create LoadedObject program
		glGenVertexArrays(1, &newModel.vao);
		glBindVertexArray(newModel.vao);
		glGenBuffers(2, newModel.vbo);		//------- Object vertexs and normals

		glBindBuffer(GL_ARRAY_BUFFER, newModel.vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, newModel.vertices.size() * sizeof(glm::vec3), &newModel.vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, newModel.vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, newModel.normals.size() * sizeof(glm::vec3), &newModel.normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		newModel.shaders[0] = compileShader(models3D_vertShader, GL_VERTEX_SHADER, "objectVert");
		newModel.shaders[1] = compileShader(models3D_toonShader, GL_FRAGMENT_SHADER, "objectToonFrag");
		newModel.shaders[2] = compileShader(models3D_flatShader, GL_FRAGMENT_SHADER, "flatFrag");

		newModel.program = glCreateProgram();
		glAttachShader(newModel.program, newModel.shaders[0]);
		glAttachShader(newModel.program, newModel.shaders[1]);
		glBindAttribLocation(newModel.program, 0, "in_Position");
		glBindAttribLocation(newModel.program, 1, "in_Normal");
		linkProgram(newModel.program);

		//Flat shader program
		newModel.flatProgram = glCreateProgram();
		glAttachShader(newModel.flatProgram, newModel.shaders[0]);
		glAttachShader(newModel.flatProgram, newModel.shaders[2]);
		glBindAttribLocation(newModel.flatProgram, 0, "in_Position");
		glBindAttribLocation(newModel.flatProgram, 1, "in_Normal");
		linkProgram(newModel.flatProgram);


		return newModel;
	}

	void cleanup(model aModel) {
		glDeleteBuffers(2, aModel.vbo);
		glDeleteVertexArrays(1, &aModel.vao);

		glDeleteProgram(aModel.program);
		glDeleteProgram(aModel.flatProgram);
		glDeleteShader(aModel.shaders[0]);
		glDeleteShader(aModel.shaders[1]);
		glDeleteShader(aModel.shaders[2]);
	}

	void draw(model aModel) {
		/*glBindVertexArray(aModel.vao);
		glUseProgram(aModel.program);
		//Model
		glUniformMatrix4fv(glGetUniformLocation(aModel.program, "objMat"), 1, GL_FALSE, glm::value_ptr(aModel.objMat));
		glUniformMatrix4fv(glGetUniformLocation(aModel.program, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(aModel.program, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(aModel.program, "color"), aModel.color.r, aModel.color.g, aModel.color.b, 1.f); //lightColor1
		//Lights
		glUniform3f(glGetUniformLocation(aModel.program, "lightColor1"), models3D::sun.color.r,		models3D::sun.color.g,		models3D::sun.color.b);
		glUniform3f(glGetUniformLocation(aModel.program, "ambientColor"), Lights::currentAmbientColor.r, Lights::currentAmbientColor.g, Lights::currentAmbientColor.b);
		glUniform4f(glGetUniformLocation(aModel.program, "light1Pos"), Lights::sunPosition.x, Lights::sunPosition.y, Lights::sunPosition.z, 1.f);						//Sun
		glUniform4f(glGetUniformLocation(aModel.program, "light2Pos"), Lights::moonPosition.x, Lights::moonPosition.y, Lights::moonPosition.z, 1.f);					//Moon
		glUniform4f(glGetUniformLocation(aModel.program, "light3Pos"), Lights::lightBulbPosition.x, Lights::lightBulbPosition.y, Lights::lightBulbPosition.z, 1.f);		//Light bulb
		glUniform1f(glGetUniformLocation(aModel.program, "ambientStrenght"), Lights::AMBIENT_LIGHT_STRENGHT);
		glUniform1f(glGetUniformLocation(aModel.program, "light1Brightnes"), Lights::currentSunBrightness);
		glUniform1f(glGetUniformLocation(aModel.program, "lightBulbBrightnes"), Lights::LIGHT_BULB_BRIGHTNESS);
		glUniform1i(glGetUniformLocation(aModel.program, "toonShaderOption"), Lights::toonShaderState);


		glDrawArrays(GL_TRIANGLES, 0, aModel.vertices.size());

		glUseProgram(0);
		glBindVertexArray(0);*/
	}
	void drawFlat(model aModel) {
		glBindVertexArray(aModel.vao);
		glUseProgram(aModel.flatProgram);
		glUniformMatrix4fv(glGetUniformLocation(aModel.flatProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(aModel.objMat));
		glUniformMatrix4fv(glGetUniformLocation(aModel.flatProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(aModel.flatProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(aModel.flatProgram, "color"), aModel.color.x, aModel.color.y, aModel.color.z, 1.f);
		glDrawArrays(GL_TRIANGLES, 0, aModel.vertices.size());

		glUseProgram(0);
		glBindVertexArray(0);
	}
}