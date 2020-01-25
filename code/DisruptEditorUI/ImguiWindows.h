#pragma once

namespace UI {
	class Window {
	public:
		virtual ~Window() = 0;

		virtual void draw() = 0;
	};

	void displayTopMenu();
	void displayTempWindows();
	void displayWindows();

#define DEFINE_WINDOW(x) void display ##x ();
#include "Windows.def"
#undef DEFINE_WINDOW
}
