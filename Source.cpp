#include <GLFW/glfw3.h>

#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

#define TEST_POLLING_RATE 2 //0 = don't. 1 = old mode, instant console output, creates issues with simultaneous keypresses. 2 = new mode, console output batched until end.
//currently, keyboard polling and mouse polling are mixed together, so only move your keyboard or your mouse, but not both simultaneously

using frame_clock = std::chrono::steady_clock; //high resolution clock uses an unfortunate realtime clock on Linux https://stackoverflow.com/a/41203433/
using float_millisecond = std::chrono::duration<float, std::ratio<1, 1000>>;
GLFWwindow* window;
std::vector<frame_clock::time_point> input_times;
std::vector<char> input_key;
std::vector<std::string> input_action;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
		  input_times.push_back(frame_clock::now());
      input_action.push_back("+");
      input_key.push_back(key);
    }
  if (action == 0) {
	    input_times.push_back(frame_clock::now());
      input_action.push_back("-");
      input_key.push_back(key);
    }
}

void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos) {
	if (TEST_POLLING_RATE == 1) {
		static frame_clock::time_point previous_time = frame_clock::now();
		auto now = frame_clock::now();
		std::cout << (float_millisecond(now - previous_time).count()) << ' ';
		previous_time = now;
	}
	else if (TEST_POLLING_RATE == 2) {
		input_times.push_back(frame_clock::now());
		input_action.push_back("-> ");
		input_key.push_back('m');
	}
}

void input_loop() {
	while (!glfwWindowShouldClose(window)) {
		if (0) {
			//pollevents option
			glfwPollEvents();
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (!TEST_POLLING_RATE)
				std::this_thread::yield(); //causes a whole bunch of CPU usage! when compared to sleeping or WaitEvents
		}
		else
			glfwWaitEvents(); //this is actually quite good. not sure if it's as good as constant polling

		if (TEST_POLLING_RATE == 2) {
			for (int x = 1; x < input_times.size(); ++x) {
				float result = float_millisecond(input_times[x] - input_times[x - 1]).count();
				//if (result > 2 && result < 100) //for getting only the interesting timepoints
					std::cout << result << ' ' << input_action[x] << input_key[x] <<  '\n';
			}
			if (input_times.size() > 1) {
				input_times = { input_times.back() };
				input_key = { input_key.back() };
        input_action = { input_action.back() };
			}
		}
	}
}

int main() {
	glfwInit();

	window = glfwCreateWindow(800, 600, "input test", nullptr, nullptr);

	if (window == NULL) {
		std::cout << ("Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);
	glfwSetCursorPosCallback(window, mouse_cursor_callback);
	glfwSetKeyCallback(window, key_callback);


	input_loop();

	glfwTerminate();
	return 0;
}
