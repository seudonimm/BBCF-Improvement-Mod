#pragma once
#include "IWindow.h"

class ScrWindow : public IWindow
{
public:
	ScrWindow(const std::string& windowTitle, bool windowClosable,
		ImGuiWindowFlags windowFlags = 0)
		: IWindow(windowTitle, windowClosable, windowFlags) {}
	~ScrWindow() override = default;

protected:
	void Draw() override;

private:
	void DrawStatesSection();


	bool m_showDemoWindow = false;
};