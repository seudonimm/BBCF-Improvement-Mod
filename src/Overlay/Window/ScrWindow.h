#pragma once
#include "IWindow.h"
#include <vector>
#include "Game/Scr/ScrStateReader.h"

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
	void DrawPlaybackSection();


	bool m_showDemoWindow = false;
	void* p2_old_char_data = NULL;
	char fpath[1200] = "fpath";
	std::vector<scrState*> gap_register{};
	std::vector<scrState*> wakeup_register{};
};