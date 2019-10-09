#ifndef HELMSLEY_VOLUME_H
#define HELMSLEY_VOLUME_H

#include <cvrKernel/CVRPlugin.h>
#include <cvrKernel/FileHandler.h>
#include <cvrKernel/SceneObject.h>
#include <cvrKernel/SceneManager.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/ComController.h>

#include <cvrConfig/ConfigManager.h>

#include <cvrMenu/SubMenu.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/MenuRadial.h>
#include <cvrMenu/MenuRangeValue.h>
#include <cvrMenu/MenuRangeValueCompact.h>
#include <cvrMenu/PopupMenu.h>
#include <cvrMenu/MenuSystem.h>
#include <cvrMenu/MenuCheckbox.h>
#include <cvrMenu/MenuList.h>

#include <osg/PositionAttitudeTransform>

#include "VolumeGroup.h"
#include "VolumeMenu.h"
#include "MeasurementTool.h"
#include "ScreenshotTool.h"
#include "FileSelector.h"

#include <string>

class HelmsleyVolume : public cvr::MenuCallback, public cvr::CVRPlugin
{
    public:
		enum ToolState {
			NONE,
			CUTTING_PLANE,
			MEASUREMENT_TOOL,
			POINTER
		};

        HelmsleyVolume();
        virtual ~HelmsleyVolume();

		static HelmsleyVolume* instance(){ return _instance; }

        bool init();
        void preFrame();
		void postFrame();

		bool processEvent(cvr::InteractionEvent* e);
		void menuCallback(cvr::MenuItem* menuItem);
		void createList(cvr::SubMenu* , std::string configbase);

		void loadVolume(std::string path, std::string maskpath = "");
		void removeVolume(int index);

		void setTool(ToolState tool) { _tool = tool; }

		std::vector<cvr::SceneObject*> getSceneObjects() { return _sceneObjects; }

		static std::string loadShaderFile(std::string filename);

    protected:
		struct MeasurementInfo {
			osg::Vec3 start;
			osg::Vec3 end;
		};

		cvr::SubMenu * _vMenu;
		cvr::MenuButton * _vButton;
		osg::MatrixTransform* cuttingPlane;
		osg::ref_ptr<MeasurementTool> measurementTool;
		ScreenshotTool* screenshotTool;
		FileSelector* fileSelector;

		std::vector<osg::ref_ptr<VolumeGroup> > _volumes;
		std::vector<cvr::SceneObject*> _sceneObjects;
		std::vector<VolumeMenu*> _volumeMenus;
		std::vector<cvr::MenuButton*> _removeButtons;

		std::map<cvr::MenuItem*, std::string> _buttonMap;
		cvr::MenuRadial * _radial;
		cvr::PopupMenu * _selectionMenu;

		osg::Matrix _selectionMatrix;

		float _cuttingPlaneDistance;

		int _interactButton = 0;
		int _radialButton = 3;
		int _radialXVal = 0;
		int _radialYVal = 1;

		bool _radialShown = false;
		float _radialX = 0;
		float _radialY = 0;

		ToolState _tool = NONE;

		static HelmsleyVolume* _instance;
};

#endif
