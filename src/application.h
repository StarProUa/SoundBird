#ifndef APP_H
#define APP_H

struct GLFWwindow;
struct PaStreamCallbackTimeInfo;

enum Key
{
	Space = 32,
	Escape = 256,
};

enum State
{
	Press = 1,
};

enum mouseButton
{
	Left = 0,
	Right,
	Middle,
};

struct Color
{
	int r, g, b;
};

inline float micSentivity;

inline bool clockUnit;
inline float delta;

class Application
{
	GLFWwindow *window;
	void *audio;

	int wWin, hWin; 

protected:
	Application(const int &width, const int &height, const char *title);

	virtual void keyboard(int &key, int &action) = 0;
	virtual void mouse(int &button, int &action, int x, int y) = 0;
	virtual void microphone(const float *data) = 0;

	virtual void render() = 0;

	void exit();

	virtual ~Application();

public:
	void Run();

private:
	void shutdown();

	void initTexture();
	void initGLFW(const char *title);
	void initPortAudio();

	static void keyboardCallback(GLFWwindow *window, int key, int scode, int action, int smode);
	static void mouseCallback(GLFWwindow *window, int button, int action, int smode);
	static int audioCallback(const void *input, void *output, unsigned long frameCount,
					  const PaStreamCallbackTimeInfo* timeInfo,
					  unsigned long statusFlags, void *userData );
};

void PaintTexture(Color color, unsigned int &&Mode, int array_first, int array_size, int *array, float *texture_array, unsigned int id);
float RMS(const float *data);

#endif // APP_H
