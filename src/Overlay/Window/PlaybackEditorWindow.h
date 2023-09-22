#pragma once
#include "IWindow.h"
#include "Game/Playbacks/PlaybackManager.h"
#include "Game/Playbacks/PlaybackSlot.h"
#include <string>

class PlaybackEditorWindow : public IWindow
{
public:
	PlaybackEditorWindow(const std::string& windowTitle, bool windowClosable,
		ImGuiWindowFlags windowFlags = 0)
		: IWindow(windowTitle, windowClosable, windowFlags), m_origWindowTitle(windowTitle), playback_manager()
	{
	}

	~PlaybackEditorWindow() override = default;
	std::string interpret_move(char move);

	PlaybackManager playback_manager;


protected:
	void Draw() override;
	void DrawEditLinePopup();

private:


	const std::string m_origWindowTitle;
};
