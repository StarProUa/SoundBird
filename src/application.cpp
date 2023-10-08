#include "application.h"

#include "GLFW/glfw3.h"
#include "portaudio.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>

#define WAIT 0.1

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned *texture;

void Paint(Color color, unsigned int &&Mode, int &&array_first, int &&array_size, int *array)
{
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glVertexPointer(2, GL_INT, 0, array);
	glEnableClientState(GL_VERTEX_ARRAY);

	glColor3f(color.r, color.g, color.b);
	glDrawArrays(Mode, array_first, array_size);

	glDisableClientState(GL_VERTEX_ARRAY);
}

void PaintTexture(Color color, unsigned int &&Mode, int array_first, int array_size, int *array, float *texture_array, unsigned int id)
{
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_INT, 0, array);
	glTexCoordPointer(2, GL_FLOAT, 0, texture_array);

	glColor4f(color.r, color.g, color.b, 1);
	glBindTexture(GL_TEXTURE_2D, texture[id]);
	glDrawArrays(Mode, array_first, array_size);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

Application::Application(const int &width, const int &height, const char *title) : wWin(width), hWin(height)
{
	try
	{
		micSentivity = 400;

		initGLFW(title);
		initTexture();
		initPortAudio();
	}
	catch(const std::runtime_error &exp)
	{
		std::cerr << exp.what() << std::endl;

		shutdown();

		std::exit(-1);
	}
}

void Application::exit()
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Application::Run()
{
	double lastUpdateTime = 0, lastFrameTime = 0;
	double sleepTime = WAIT;

	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		double now = glfwGetTime();
		delta = now - lastUpdateTime;

		glfwPollEvents();

		if((sleepTime -= delta) <= 0)
		{
			clockUnit = true;
			sleepTime = WAIT;
		}

		render();
		glfwSwapBuffers(window);

		clockUnit = false;

		lastUpdateTime = now;
	}

	Pa_StopStream(audio);
	Pa_CloseStream(audio);

	Pa_Terminate();
}

void Application::shutdown()
{
	Pa_StopStream(audio);
	Pa_CloseStream(audio);

	Pa_Terminate();

	glfwDestroyWindow(window);

	int amount = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &amount);

	for(int i = 0; i < amount; i++)
	{
		glDeleteTextures(1, &texture[i]);
	}

	glfwTerminate();
}

void Application::initTexture()
{
	struct Texture
	{
		int h_image, w_image, ch;
		unsigned char *data;
	};

	std::vector<Texture> tx;

	std::ifstream file("..\\assets\\sprites.don", std::ios::binary);

	if(!file.is_open()) throw std::runtime_error("Error to opening texture file");

	std::cout << "Start read file" << std::endl;

	while(1)
	{
		char skip;
		if(!file.read(&skip, sizeof(skip))) break;

		char name[256];
		if(!file.read(name, sizeof(name))) break;

		long size;
		if(!file.read(reinterpret_cast<char *>(&size), sizeof(size))) break;

		char data[size];
		file.read(data, sizeof(data));

		Texture temp;
		temp.data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(data), sizeof(data),
										  &temp.w_image, &temp.h_image, &temp.ch, 0);

		tx.push_back(temp);
	}

	std::cout << "End read file" << std::endl;

	file.close();

	for(unsigned i = 0; i < tx.size(); i++)
	{
		if(tx[i].data == NULL) throw std::runtime_error("FAILED TO INIT TEXTURE - " + std::to_string(i));
	}

	std::cout << "End check data" << std::endl;

	texture = new unsigned[tx.size()];

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	glGenTextures(tx.size(), texture);

	std::cout << "Enable gl Texture arg" << std::endl;

	for(unsigned i = 0; i < tx.size(); i++)
	{
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, (tx[i].ch == 4) ? GL_RGBA : GL_RGB,
						 tx[i].w_image, tx[i].h_image, 0, (tx[i].ch == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, tx[i].data);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(tx[i].data);
	}

	std::cout << "End settings texture" << std::endl;

	glDisable(GL_TEXTURE_2D);
}

void Application::initGLFW(const char *title)
{
	if(!glfwInit()) throw std::runtime_error("Failed to init glfw");

	window = glfwCreateWindow(wWin, hWin, title, nullptr, nullptr);

	if(!window) throw std::runtime_error("Error to create window");

	glfwMakeContextCurrent(window);

	glClearColor(1, 1, 1, 0);
	glOrtho(0, wWin, 0, hWin, 0, 1);

	glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height)
							  {
									glViewport(0, 0, width, height);
							  });

	glfwSetKeyCallback(window, keyboardCallback);

	glfwSetMouseButtonCallback(window, mouseCallback);

	glfwSetWindowUserPointer(window, this);
}

void Application::initPortAudio()
{
	PaError err;

	err = Pa_Initialize();
	if(err != paNoError) throw std::runtime_error("Failed to init PortAudio: " + std::string(Pa_GetErrorText(err)));

	PaStreamParameters inputParameters;
	inputParameters.device = Pa_GetDefaultInputDevice();

	const PaDeviceInfo *device = Pa_GetDeviceInfo(inputParameters.device);

	err = Pa_OpenDefaultStream(&audio, 2, 0, paFloat32, device->defaultSampleRate, 256, audioCallback, this);
	if(err != paNoError) throw std::runtime_error("Error to create thread: " + std::string(Pa_GetErrorText(err)));

	err = Pa_StartStream(audio);
	if(err != paNoError) throw std::runtime_error("Error startup thread: " + std::string(Pa_GetErrorText(err)));
}

Application::~Application()
{
	shutdown();
}

void Application::keyboardCallback(GLFWwindow *window, int key, int scode, int action, int smode)
{
	Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
	app->keyboard(key, action);
}

void Application::mouseCallback(GLFWwindow *window, int button, int action, int smode)
{
	double x, y;

	glfwGetCursorPos(window, &x, &y);

	Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
	app->mouse(button, action, x, app->hWin - y);
}

int Application::audioCallback(const void *input, void *output, unsigned long frameCount,
							   const PaStreamCallbackTimeInfo *timeInfo, unsigned long statusFlags,
							   void *userData)
{
	Application *app = static_cast<Application *>(userData);
	const float *mic = static_cast<const float *>(input);

	app->microphone(mic);

	return paContinue;
}

float RMS(const float *data)
{
	float rms = 1;
	float alpha = 0.1;
	for(int i = 0; i < 256; i++)
	{
		rms = (1 - alpha) * rms + alpha * (data[i] * data[i]);
	}


	return std::sqrt(rms / 256) * micSentivity;
}
