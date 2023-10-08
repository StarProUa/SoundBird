#include <iostream>
#include "GLFW/glfw3.h"
#include "application.h"
#include <cmath>

int box[8]  = {0,50, 0,0, 50,50, 50,0};
float sprite[8] = {0.f,0.f, 0.f,1.f, 1.f,0.f, 1.f,1.f};

class Game : public Application
{
	float rms;
	float point = 0;
public:
	Game() : Application(640, 480, "SoundBird")
	{

	}
	~Game() override
	{

	}
private:
	void render() override
	{
		point += rms * delta * micSentivity;
		glPushMatrix();
		glColor3f(1, 0, 0);
		glPointSize(10);
		glTranslatef(640 / 2, (480 / 2) + point, 0);

		PaintTexture({1, 1, 1}, GL_TRIANGLE_STRIP, 0, 4, box, sprite, 1);

		point += -micSentivity * delta;

		std::cout << point << std::endl;
		glPopMatrix();
	}
	void keyboard(int &key, int &action) override
	{
		if(key == Key::Escape && action == State::Press) exit();
	}
	void microphone(const float *data) override
	{
		rms = RMS(data);
	}
	void mouse(int &button, int &action, int x, int y) override
	{

	}
};

int main()
{
	Game SoundBird;
	SoundBird.Run();
}
