#include "VolumeMenu.h"
#include "HelmsleyVolume.h"
#include "Utils.h"
#include "cvrMenu/MenuManager.h"
#include "cvrConfig/ConfigManager.h"


using namespace cvr;

void VolumeMenu::init()
{
	scale = new MenuRangeValueCompact("Scale", 0.1, 100.0, 1.0, true);
	scale->setCallback(this);
	_scene->addMenuItem(scale);

	sampleDistance = new MenuRangeValueCompact("SampleDistance", .0001, 0.01, .00150f, true);
	sampleDistance->setCallback(this);
	_scene->addMenuItem(sampleDistance);
	
	adaptiveQuality = new MenuCheckbox("Adaptive Quality", false);
	adaptiveQuality->setCallback(this);
	_scene->addMenuItem(adaptiveQuality);
	
}

void VolumeMenu::menuCallback(cvr::MenuItem * item)
{
	if (item == sampleDistance)
	{
		_volume->_StepSize->set(sampleDistance->getValue());
	}
	else if (item == scale)
	{
		_scene->setScale(scale->getValue());
	}
	else if (item == adaptiveQuality)
	{
		_volume->getDrawable()->getOrCreateStateSet()->setDefine("VR_ADAPTIVE_QUALITY", adaptiveQuality->getValue());
	}
}

VolumeMenu::~VolumeMenu() {
	if (_scene)
	{
		delete _scene;
	}
}

NewVolumeMenu::~NewVolumeMenu()
{
	delete _toolMenu;

	_menu->setActive(false, false);
	MenuManager::instance()->removeMenuSystem(_menu);
	delete _menu;

	if (_maskMenu)
	{
		_maskMenu->setActive(false, false);
		MenuManager::instance()->removeMenuSystem(_maskMenu);
		delete _maskMenu;
	}
	if (_tentMenu)
	{
		_tentMenu->setActive(false, false);
		MenuManager::instance()->removeMenuSystem(_tentMenu);
		delete _tentMenu;
	}

	if (_container)
	{
		delete _container;
	}
	if (_maskContainer)
	{
		delete _maskContainer;
	}
	if (_tentWindowContainer)
	{
		delete _tentWindowContainer;
	}

	if(_cp->_parent == nullptr){
		delete _cp;
	}
	
	delete _so;

	delete _volume;

}

void NewVolumeMenu::init()
{
#pragma region VolumeOptions
	_so = new SceneObject("VolumeMenu", false, false, false, false, false);
	PluginHelper::registerSceneObject(_so, "HelmsleyVolume");
	_so->attachToScene();
#ifdef WITH_OPENVR
	_so->getRoot()->addUpdateCallback(new FollowSceneObjectLerp(_scene, 0.2f, _so));
	_so->getRoot()->addUpdateCallback(new PointAtHeadLerp(0.2f, _so));
#endif

	osg::Vec3 volumePos = ConfigManager::getVec3("Plugin.HelmsleyVolume.Orientation.Volume.Position", osg::Vec3(0, 750, 1000));
	_so->setPosition(volumePos);

	_menu = new UIPopup();
	
	UIQuadElement* bknd = new UIQuadElement(UI_BACKGROUND_COLOR);
	_menu->addChild(bknd);
	_menu->setPosition(osg::Vec3(1000, 450, 1300));//ConfigManager::getVec3("Plugin.HelmsleyVolume.Orientation.OptionsMenu.Position", osg::Vec3(500, -500, 1350)) - volumePos);

	_menu->getRootElement()->setAbsoluteSize(ConfigManager::getVec3("Plugin.HelmsleyVolume.Orientation.OptionsMenu.Scale", osg::Vec3(1000, 1, 600)));
	


	ColorPicker* cp = new ColorPicker();
	_cp = cp;
	_tentWindowOnly = new TentWindowOnly();
	 _tentWindow = new TentWindow(_tentWindowOnly);
	_menu->addChild(_tentWindow);
	_tentWindow->setPercentSize(osg::Vec3(1, 1, .75));
	_tentWindow->setPercentPos(osg::Vec3(0, 0, -1));
	_tentWindow->setVolume(_volume);
	


	UIList* list = new UIList(UIList::TOP_TO_BOTTOM, UIList::CONTINUE);
	list->setPercentPos(osg::Vec3(0, 0, -0.2));
	list->setPercentSize(osg::Vec3(1, 1, 0.8));
	list->setAbsoluteSpacing(10);
	bknd->addChild(list);

	

	UIText* label = new UIText("Volume Options", 50.0f, osgText::TextBase::CENTER_CENTER);
	label->setPercentSize(osg::Vec3(1, 1, 0.2));
	bknd->addChild(label);


	UIList* fliplist = new UIList(UIList::LEFT_TO_RIGHT, UIList::CUT);

	_horizontalflip = new CallbackButton();
	_horizontalflip->setCallback(this);
	label = new UIText("Flip Sagittal", 30.0f, osgText::TextBase::CENTER_CENTER);
	label->setColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
	_horizontalflip->addChild(label);

	_verticalflip = new CallbackButton();
	_verticalflip->setCallback(this);
	label = new UIText("Flip Axial", 30.0f, osgText::TextBase::CENTER_CENTER);
	label->setColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
	_verticalflip->addChild(label);

	_depthflip = new CallbackButton();
	_depthflip->setCallback(this);
	label = new UIText("Flip Coronal", 30.0f, osgText::TextBase::CENTER_CENTER);
	label->setColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
	_depthflip->addChild(label);

	fliplist->addChild(_horizontalflip);
	fliplist->addChild(_verticalflip);
	fliplist->addChild(_depthflip);
	list->addChild(fliplist);

	label = new UIText("Density", 30.0f, osgText::TextBase::LEFT_CENTER);
	label->setPercentPos(osg::Vec3(0.1, 0, 0));

	UIList* valueList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CUT);
	label = new UIText("Contrast", 40.0f, osgText::TextBase::LEFT_CENTER);
	label->setPercentPos(osg::Vec3(0.1, 0, 0));
	valueList->addChild(label);
	_contrastValueLabel = new UIText("Low 0.00 / High 1.00", 35.0f, osgText::TextBase::RIGHT_CENTER);
	_contrastValueLabel->setPercentPos(osg::Vec3(-0.1, 0, 0));
	valueList->addChild(_contrastValueLabel);
	list->addChild(valueList);

	_contrastBottom = new CallbackSlider();
	_contrastBottom->setPercentPos(osg::Vec3(0.025, 0, 0.05));
	_contrastBottom->setPercentSize(osg::Vec3(0.95, 1, 0.9));
	_contrastBottom->handle->setAbsoluteSize(osg::Vec3(20, 0, 0));
	_contrastBottom->handle->setAbsolutePos(osg::Vec3(-10, -0.2f, 0));
	_contrastBottom->handle->setPercentSize(osg::Vec3(0, 1, 1));
	_contrastBottom->setCallback(this);
	_contrastBottom->setPercent(0);

	_contrastTop = new CallbackSlider();
	_contrastTop->setPercentPos(osg::Vec3(0.025, 0, 0.05));
	_contrastTop->setPercentSize(osg::Vec3(0.95, 1, 0.9));
	_contrastTop->handle->setAbsoluteSize(osg::Vec3(20, 0, 0));
	_contrastTop->handle->setAbsolutePos(osg::Vec3(-10, -0.2f, 0));
	_contrastTop->handle->setPercentSize(osg::Vec3(0, 1, 1));
	_contrastTop->setCallback(this);
	_contrastTop->setPercent(1);

	_brightness = new CallbackSlider();
	_brightness->setPercentPos(osg::Vec3(0.025, 0, 0.05));
	_brightness->setPercentSize(osg::Vec3(0.95, 1, 0.9));
	_brightness->handle->setAbsoluteSize(osg::Vec3(20, 0, 0));
	_brightness->handle->setAbsolutePos(osg::Vec3(-10, -0.2f, 0));
	_brightness->handle->setPercentSize(osg::Vec3(0, 1, 1));
	_brightness->setCallback(this);
	_brightness->setPercent(.5f);

	list->addChild(_contrastBottom);
	list->addChild(_contrastTop);


	valueList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CUT);
	label = new UIText("Brightness", 40.0f, osgText::TextBase::LEFT_CENTER);
	label->setPercentPos(osg::Vec3(0.1, 0, 0));
	valueList->addChild(label);
	_brightValueLabel = new UIText("+0.00", 35.0f, osgText::TextBase::RIGHT_CENTER);
	_brightValueLabel->setPercentPos(osg::Vec3(-0.1, 0, 0));
	valueList->addChild(_brightValueLabel);
	list->addChild(valueList);
	list->addChild(_brightness);


	

	UIList* list2 = new UIList(UIList::LEFT_TO_RIGHT, UIList::CUT);
	
	
	_transferFunctionRadial = new CallbackRadial();
	_transferFunctionRadial->allowNoneSelected(false);
	_blacknwhite = new UIRadialButton(_transferFunctionRadial);
	_rainbow = new UIRadialButton(_transferFunctionRadial);
	_custom = new UIRadialButton(_transferFunctionRadial);


	_transferFunctionRadial->setCurrent(0);
	_transferFunctionRadial->setCallback(this);

	_colorDisplay = new ShaderQuad();
	
	std::string vert = HelmsleyVolume::loadShaderFile("transferFunction.vert");
	std::string frag = HelmsleyVolume::loadShaderFile("transferFunction.frag");
	osg::Program* p = new osg::Program;
	p->setName("TransferFunction");
	p->addShader(new osg::Shader(osg::Shader::VERTEX, vert));
	p->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag));
	_colorDisplay->setProgram(p);
	_colorDisplay->addUniform(_volume->_computeUniforms["ContrastBottom"]);
	_colorDisplay->addUniform(_volume->_computeUniforms["ContrastTop"]);
	_colorDisplay->addUniform(_volume->_computeUniforms["Brightness"]);
	_colorDisplay->addUniform(_volume->_computeUniforms["leftColor"]);
	_colorDisplay->addUniform(_volume->_computeUniforms["rightColor"]);
	_colorDisplay->setPercentSize(osg::Vec3(1.0, 1.0, 0.5));
	_colorDisplay->getGeode()->getOrCreateStateSet()->setRenderBinDetails(3, "RenderBin");

	_opacityDisplay = new ShaderQuad();
	_opacityDisplay->getGeode()->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	frag = HelmsleyVolume::loadShaderFile("transferFunctionOpacity.frag");
	p = new osg::Program;
	p->setName("TransferFunctionOpacity");
	p->addShader(new osg::Shader(osg::Shader::VERTEX, vert));
	p->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag));
	_opacityDisplay->setProgram(p);
	_opacityDisplay->addUniform(_volume->_computeUniforms["OpacityCenter"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["OpacityWidth"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["OpacityTopWidth"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["OpacityMult"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["Lowest"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["ContrastBottom"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["ContrastTop"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["Brightness"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["TriangleCount"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["leftColor"]);
	_opacityDisplay->addUniform(_volume->_computeUniforms["rightColor"]);
	_opacityDisplay->setPercentSize(osg::Vec3(1.0, 1.0, 0.5));
	_opacityDisplay->setPercentPos(osg::Vec3(0.0, 0.0, 1.0));
	_opacityDisplay->getGeode()->getOrCreateStateSet()->setRenderBinDetails(3, "RenderBin");

	_opacityColorDisplay = new ShaderQuad();
	_opacityColorDisplay->getGeode()->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	frag = HelmsleyVolume::loadShaderFile("transferFunctionColorOpacity.frag");
	p = new osg::Program;
	p->setName("TransferFunctionColorOpacity");
	p->addShader(new osg::Shader(osg::Shader::VERTEX, vert));
	p->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag));
	_opacityColorDisplay->setProgram(p);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["OpacityCenter"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["OpacityWidth"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["OpacityTopWidth"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["OpacityMult"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["Lowest"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["ContrastBottom"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["ContrastTop"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["Brightness"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["TriangleCount"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["leftColor"]);
	_opacityColorDisplay->addUniform(_volume->_computeUniforms["rightColor"]);
	_opacityColorDisplay->setPercentSize(osg::Vec3(1.0, 1.0, 0.5));

	_opacityColorDisplay->setPercentPos(osg::Vec3(0.0, 0.0, 0.5));
	_opacityColorDisplay->getGeode()->getOrCreateStateSet()->setRenderBinDetails(3, "RenderBin");
	
	
	
	UIText* bnw = new UIText("Grayscale", 40.0f, osgText::TextBase::CENTER_CENTER);
	bnw->setColor(osg::Vec4(0.8, 1, 0.8, 1));
	UIText* rnbw = new UIText("Rainbow", 40.0f, osgText::TextBase::CENTER_CENTER);
	rnbw->setColor(osg::Vec4(1, 0.8, 0.8, 1));
	UIText* cstm = new UIText("Custom", 40.0f, osgText::TextBase::CENTER_CENTER);
	cstm->setColor(osg::Vec4(1, 0.8, 0.8, 1));

	

	_blacknwhite->addChild(bnw);
	_rainbow->addChild(rnbw);
	_custom->addChild(cstm);


	list2->addChild(_blacknwhite);
	list2->addChild(_rainbow);
	list2->addChild(_custom);

	list->addChild(list2);
	list->setAbsoluteSpacing(0.0);
	_tentMenu = new UIPopup();
	
	_tentWindowOnly->setPercentSize(osg::Vec3(1, 1, 3.0));
	_tentWindowOnly->setPercentPos(osg::Vec3(0, 0, 1.50));
	UIList* gradientList = new UIList(UIList::TOP_TO_BOTTOM, UIList::CONTINUE);
	gradientList->addChild(_colorDisplay);
	gradientList->addChild(_opacityColorDisplay);
	gradientList->addChild(_opacityDisplay);
	gradientList->addChild(_tentWindowOnly);
	
	UIQuadElement* gradientBknd = new UIQuadElement(UI_BACKGROUND_COLOR);
	gradientBknd->setPercentSize(osg::Vec3(1, 1, .4));

	_colorSliderIndex = 0;
	_colSliderList[_colorSliderIndex] = new ColorSlider(_cp, osg::Vec4(1.0, 0.0, 0.0, 1.0));
	_colSliderList[_colorSliderIndex]->setPercentPos(osg::Vec3(0.0, -1.0, 0.0));
	_colSliderList[_colorSliderIndex]->setAbsoluteSize(osg::Vec3(50.0, 50.0, 50.0));
	_colSliderList[_colorSliderIndex]->setPercentSize(osg::Vec3(0.0, 0.0, 0.0));
	_colSliderList[_colorSliderIndex]->setCallback(this);

	_colorSliderIndex = 1;
	_colSliderList[_colorSliderIndex] = new ColorSlider(_cp, osg::Vec4(1.0, 1.0, 1.0, 1.0));
	_colSliderList[_colorSliderIndex]->setPercentPos(osg::Vec3(1.0, -1.0, 0.0));
	_colSliderList[_colorSliderIndex]->setAbsoluteSize(osg::Vec3(50.0, 50.0, 50.0));
	_colSliderList[_colorSliderIndex]->setPercentSize(osg::Vec3(0.0, 0.0, 0.0));
	_colSliderList[_colorSliderIndex]->setCallback(this);
	_colorSliderIndex = 0;

	_colSliderList[0]->getGeode()->setNodeMask(0);
	_colSliderList[1]->getGeode()->setNodeMask(0);

	_cp->setPercentPos(osg::Vec3(-0.35, 0.0, 0.0));
	_cp->setPercentSize(osg::Vec3(0.3, 0.0, 1.0));
	_cp->setCallback(this);
	


	_tentMenu->addChild(_colSliderList[0]);
	_tentMenu->addChild(_colSliderList[1]);
	_tentMenu->addChild(gradientBknd);
	_tentMenu->addChild(gradientList);

	
	
	

	if (!_movable)
	{
		_menu->setActive(true, true);
	}
	else {
		_menu->setActive(true, false);
		_container = new SceneObject("VolumeMenu", false, true, false, false, false);
		_so->addChild(_container);
		_container->setShowBounds(true);
		_container->addChild(_menu->getRoot());
		_menu->getRootElement()->updateElement(osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 0));
		_container->dirtyBounds();
	}

#pragma endregion
	//>===============================MASKS AND REGIONS==============================<//
	_toolMenu = new ToolMenu(0, true, _so);
	_maskMenu = new UIPopup();
	UIQuadElement* regionHeaderBknd = new UIQuadElement(UI_BACKGROUND_COLOR);
	_presetBknd = new UIQuadElement(UI_BACKGROUND_COLOR);
	_presetBknd->setBorderSize(.01);
	_presetPopup = new UIPopup;

	UIText* regionLabel = new UIText("Regions", 50.0f, osgText::TextBase::CENTER_CENTER);
	regionLabel->setPercentSize(osg::Vec3(1, 1, 0.2));
	_addTriangle = new CallbackButton();
	_addTriangle->setCallback(this);
	_addPreset = new CallbackButton();
	_addPreset->setCallback(this);
	_loadPreset = new CallbackButton();
	_loadPreset->setCallback(this);

	label = new UIText(" Add Region ", 50.0f, osgText::TextBase::LEFT_TOP);
	label->setColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
	_addTriangle->addChild(label);
	UIText* addPresetLabel = new UIText(" Save Preset ", 50.0f, osgText::TextBase::CENTER_TOP);
	addPresetLabel->setColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
	_addPreset->addChild(addPresetLabel);
	UIText* loadPresetLabel = new UIText(" Load Preset ", 50.0f, osgText::TextBase::RIGHT_TOP);
	loadPresetLabel->setColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
	_loadPreset->addChild(loadPresetLabel);
	
	regionHeaderBknd->addChild(regionLabel);
	regionHeaderBknd->setPercentSize(osg::Vec3(1.62,1.0,0.7));
	regionHeaderBknd->setBorderSize(.01);
	
	UIText* presetsLabel = new UIText("Presets", 50.0f, osgText::TextBase::CENTER_CENTER);
	presetsLabel->setPercentSize(osg::Vec3(1, 0, 0.2));
	presetsLabel->setPercentPos(osg::Vec3(0, -.1, -.05));

	
	_presetBknd->setPercentSize(osg::Vec3(.3, 0.0, .3));
	_presetBknd->setBorderSize(.01);
	_presetBknd->addChild(presetsLabel);
	_presetPopup->addChild(_presetBknd);

	_presetUIList = addPresets(_presetBknd);
	if (_presetUIList == nullptr) {
		_presetUIList = new UIList(UIList::TOP_TO_BOTTOM, UIList::CONTINUE);
		_presetUIList->setPercentPos(osg::Vec3(0, 0, -.25));
		_presetUIList->setPercentSize(osg::Vec3(1.0, 1.0, .75));
		_presetBknd->addChild(_presetUIList);
	}

	_triangleList = new UIList(UIList::TOP_TO_BOTTOM, UIList::CONTINUE);
	_triangleList->setPercentPos(osg::Vec3(0.0, 0.0, -.25));
	_triangleList->setPercentSize(osg::Vec3(1.0, 1.0, .75));
	
	_triangleIndex = 0;
	_triangleCallbacks[_triangleIndex] = new CallbackButton();
	_triangleCallbacks[_triangleIndex]->setCallback(this);
	_triangleCallbacks[_triangleIndex]->setPercentPos(osg::Vec3(0.20, 0.0, 0.0));

	label = new UIText("Triangle 0", 50.0f, osgText::TextBase::LEFT_TOP); 
	label->setColor(osg::Vec4(triangleColors[_triangleIndex], 1.0));
	label->setPercentPos(osg::Vec3(-.05, -10.0, 0.0));
	label->getTextObject()->setPosition(osg::Vec3(0.0, -5, 0.0));
	_triangleCallbacks[_triangleIndex]->addChild(label);

	label->setPercentSize(osg::Vec3(.25, 0.0, .5));
	VisibilityToggle* vT = new VisibilityToggle("");
	vT->getChild(0)->setPercentSize(osg::Vec3(1, 1, 1));
	vT->getChild(0)->setPercentPos(osg::Vec3(0, 0, 0));
	vT->setCallback(this);
	vT->setPercentPos(osg::Vec3(-0.20, 0.0, 0.1));
	vT->setPercentSize(osg::Vec3(0.185, 0.0, 0.5));
	vT->toggle();


	ShaderQuad* sq = new ShaderQuad();
	frag = HelmsleyVolume::loadShaderFile("shaderQuadPreview.frag");
	vert = HelmsleyVolume::loadShaderFile("transferFunction.vert");
	p = new osg::Program;
	p->setName("trianglePreview");
	p->addShader(new osg::Shader(osg::Shader::VERTEX, vert));
	p->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag));
	sq->setProgram(p);
	sq->setPercentSize(osg::Vec3(0.5, 1.0, 0.4));
	sq->setPercentPos(osg::Vec3(.27, 0.0, 0.1));
	sq->addUniform(_tentWindow->_tWOnly->_tents->at(0)->_centerUniform);
	sq->addUniform(_tentWindow->_tWOnly->_tents->at(0)->_widthUniform);
	sq->addUniform(_volume->_computeUniforms["ContrastBottom"]);
	sq->addUniform(_volume->_computeUniforms["ContrastTop"]);
	sq->addUniform(_volume->_computeUniforms["Brightness"]);

	_triangleCallbacks[_triangleIndex]->addChild(vT);
	_triangleCallbacks[_triangleIndex]->addChild(sq);


	label->setAbsoluteSize(osg::Vec3(0.0f, 0.0f, 50.0));
	UIList* regionTopList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CONTINUE);
	regionTopList->addChild(_addTriangle);
	regionTopList->addChild(_addPreset);
	regionTopList->addChild(_loadPreset);
	_triangleList->addChild(regionTopList);
	_triangleList->addChild(_triangleCallbacks[_triangleIndex]);
	_triangleList->setMaxSize(label->getAbsoluteSize().z()*3);

	regionHeaderBknd->addChild(_triangleList);
	_maskMenu->addChild(regionHeaderBknd);
	
	osg::Quat rot;
	rot.makeRotate(0.707, 0, 0, 1);
	_maskMenu->setRotation(rot);
	_maskMenu->setPosition(osg::Vec3(-2000, -1000, 1300));
	_maskMenu->getRootElement()->setAbsoluteSize(osg::Vec3(500, 1, 800));


	
	_tentMenu->setPosition(osg::Vec3(-1200, 675, 1880));
	_tentMenu->getRootElement()->setAbsoluteSize(osg::Vec3(3000, 1, 600));
	



	/*if (_volume->hasMask())
	{
	*/	
		_maskBknd = new UIQuadElement(UI_BACKGROUND_COLOR);
		_maskBknd->setPercentPos((osg::Vec3(0, 0, -.7)));
		_maskBknd->setPercentSize((osg::Vec3(1.62, 1, 1)));
		_maskBknd->setBorderSize(.01);
		_maskMenu->addChild(_maskBknd);

		label = new UIText("Organs", 50.0f, osgText::TextBase::CENTER_CENTER);
		label->setPercentSize(osg::Vec3(1, 1, 0.2));
		_maskBknd->addChild(label);

		_mainMaskList = new UIList(UIList::TOP_TO_BOTTOM, UIList::CONTINUE);
		_mainMaskList->setPercentPos(osg::Vec3(0, 0, -.2));
		_mainMaskList->setPercentSize(osg::Vec3(1, 1, .8));
		_maskBknd->addChild(_mainMaskList);
		
		_bodyColCallback = new CallbackButton();
		_bodyColCallback->setCallback(this);
		_colonColCallback = new CallbackButton();
		_colonColCallback->setCallback(this);
		_kidneyColCallback = new CallbackButton();
		_kidneyColCallback->setCallback(this);
		_bladderColCallback = new CallbackButton();
		_bladderColCallback->setCallback(this);
		_spleenColCallback = new CallbackButton();
		_spleenColCallback->setCallback(this);
		_aortaColCallback = new CallbackButton();
		_aortaColCallback->setCallback(this);
		_illeumColCallback = new CallbackButton();
		_illeumColCallback->setCallback(this);
		
		UIQuadElement* bodyColorbutton = new UIQuadElement(osg::Vec4(1, 0, 0, 0));
		bodyColorbutton->setPercentPos(osg::Vec3(0.6, 0.0, 0.0));
		bodyColorbutton->setPercentSize(osg::Vec3(.25, 1.0, 0.8));
		bodyColorbutton->setTransparent(true);

		_colonColorButton = new UIQuadElement(osg::Vec4(0.450, 0.090, 1, 1));
		_colonColorButton->setBorderSize(.05);
		_colonCol = osg::Vec3(0, 1, 1);
		_colonColorButton->addChild(_colonColCallback);
		_colonColorButton->setPercentPos(osg::Vec3(0.6, 0.0, 0.0));
		_colonColorButton->setPercentSize(osg::Vec3(.25, 1.0, 0.8));

		_kidneyColorButton = new UIQuadElement(osg::Vec4(0, 0.278, 1, 1));
		_kidneyColorButton->setBorderSize(.05);
		_kidneyCol = osg::Vec3(.33, 1, 1);
		_kidneyColorButton->addChild(_kidneyColCallback);
		_kidneyColorButton->setPercentPos(osg::Vec3(0.6, 0.0, 0.0));
		_kidneyColorButton->setPercentSize(osg::Vec3(.25, 1.0, 0.8));
		
	
		_bladderColorButton = new UIQuadElement(osg::Vec4(0.992, 0.968, 0.843, 1));
		_bladderColorButton->setBorderSize(.05);
		_bladderCol = osg::Vec3(.66, 1, 1);
		_bladderColorButton->addChild(_bladderColCallback);
		_bladderColorButton->setPercentPos(osg::Vec3(0.6, 0.0, 0.0));
		_bladderColorButton->setPercentSize(osg::Vec3(.25, 1.0, 0.8));
		
		_spleenColorButton = new UIQuadElement(osg::Vec4(1, 0.874, 0.109, 1));
		_spleenColorButton->setBorderSize(.05);
		_spleenCol = osg::Vec3(.5, 1, 1);
		_spleenColorButton->addChild(_spleenColCallback);
		_spleenColorButton->setPercentPos(osg::Vec3(0.6, 0.0, 0.0));
		_spleenColorButton->setPercentSize(osg::Vec3(.25, 1.0, 0.8));

		_aortaColorButton = new UIQuadElement(osg::Vec4(1, 0, 0, 1));
		_aortaColorButton->setBorderSize(.05);
		_aortaCol = osg::Vec3(0.984, 0.109, 0.372);
		_aortaColorButton->addChild(_aortaColCallback);
		_aortaColorButton->setPercentPos(osg::Vec3(0.6, 0.0, 0.0));
		_aortaColorButton->setPercentSize(osg::Vec3(.25, 1.0, 0.8));

		_illeumColorButton = new UIQuadElement(osg::Vec4(0.968, 0.780, 1, 1));
		_illeumColorButton->setBorderSize(.05);
		_illeumCol = osg::Vec3(.5, 1, 1);
		_illeumColorButton->addChild(_illeumColCallback);
		_illeumColorButton->setPercentPos(osg::Vec3(0.6, 0.0, 0.0));
		_illeumColorButton->setPercentSize(osg::Vec3(.25, 1.0, 0.8));


		_organs = new VisibilityToggle("Body");
		_organs->toggle();
		_organs->setCallback(this);

		_colon = new VisibilityToggle("Colon");
		_colon->setCallback(this);

		_kidney = new VisibilityToggle("Kidney");
		_kidney->setCallback(this);

		_bladder = new VisibilityToggle("Bladder");
		_bladder->setCallback(this);

		_spleen = new VisibilityToggle("Spleen");
		_spleen->setCallback(this);

		_illeum = new VisibilityToggle("Illeum");
		_illeum->setCallback(this);

		_aorta = new VisibilityToggle("Aorta");
		_aorta->setCallback(this);

		UIList* bodyList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CONTINUE);
		bodyList->addChild(_organs);
		bodyList->addChild(bodyColorbutton);
		bodyList->setPercentPos(osg::Vec3(.02, 0.0, 0.0));
		UIList* colonList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CONTINUE);
		colonList->addChild(_colon);
		colonList->addChild(_colonColorButton);
		colonList->setPercentPos(osg::Vec3(.02, 0.0, 0.0));
		UIList* kidneyList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CONTINUE);
		kidneyList->addChild(_kidney);
		kidneyList->addChild(_kidneyColorButton);
		kidneyList->setPercentPos(osg::Vec3(.02, 0.0, 0.0));
		UIList* bladderList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CONTINUE);
		bladderList->addChild(_bladder);
		bladderList->addChild(_bladderColorButton);
		bladderList->setPercentPos(osg::Vec3(.02, 0.0, 0.0));
		UIList* spleenList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CONTINUE);
		spleenList->addChild(_spleen);
		spleenList->addChild(_spleenColorButton);
		spleenList->setPercentPos(osg::Vec3(.02, 0.0, 0.0));
		UIList* aortaList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CONTINUE);
		aortaList->addChild(_aorta);
		aortaList->addChild(_aortaColorButton);
		aortaList->setPercentPos(osg::Vec3(.02, 0.0, 0.0));
		UIList* illeumList = new UIList(UIList::LEFT_TO_RIGHT, UIList::CONTINUE);
		illeumList->addChild(_illeum);
		illeumList->addChild(_illeumColorButton);
		illeumList->setPercentPos(osg::Vec3(.02, 0.0, 0.0));
	
		_mainMaskList->addChild(bodyList);
		_mainMaskList->addChild(colonList);
		_mainMaskList->addChild(kidneyList);
		_mainMaskList->addChild(bladderList);
		_mainMaskList->addChild(spleenList);
		_mainMaskList->addChild(aortaList);
		_mainMaskList->addChild(illeumList);
		
	//}
	if (!_movable)
	{
		_maskMenu->setActive(true, true);
		_tentMenu->setActive(true, true);
	}
	else {
		_maskMenu->setActive(true, false);
		_tentMenu->setActive(true, false);
		_maskContainer = new SceneObject("MaskMenu", false, true, false, false, false);
		_tentWindowContainer = new SceneObject("TentWindow", false, true, false, false, false);
		_so->addChild(_maskContainer);
		_so->addChild(_tentWindowContainer);

		_maskContainer->setShowBounds(false);
		_maskContainer->addChild(_maskMenu->getRoot());

		_tentWindowContainer->setShowBounds(false);
		_tentWindowContainer->addChild(_tentMenu->getRoot());

		_maskMenu->getRootElement()->updateElement(osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 0));
		_tentMenu->getRootElement()->updateElement(osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 0));
		_maskContainer->dirtyBounds();
	}

	if (!_volume->hasMask()) {
		_maskMenu->getRootElement()->getGroup()->removeChild(_maskBknd->getGroup());
		_maskBknd->_parent = nullptr;
		_maskBknd->setActive(false);
	}
}

UIList* NewVolumeMenu::addPresets(UIQuadElement* bknd) {
	std::vector<std::string> presetFilePaths = FileSelector::getPresets();
	UIList* presetUIList;
	if (!presetFilePaths.empty()) {
		presetUIList = new UIList(UIList::TOP_TO_BOTTOM, UIList::CONTINUE);
		presetUIList->setPercentPos(osg::Vec3(0, 0, -.25));
		presetUIList->setPercentSize(osg::Vec3(1.0, 1.0, .75));
	}
	else {
		return nullptr;
	}
	for (int i = 0; i < presetFilePaths.size(); i++) {
		CallbackButton* presetbutton = new CallbackButton();
		presetbutton->setCallback(this);
		int presetNameStart = strrchr(presetFilePaths[i].c_str(), '\\') - presetFilePaths[i].c_str() + 1;
		int presetNameLength = strrchr(presetFilePaths[i].c_str(), '.') - presetFilePaths[i].c_str() - presetNameStart;
		UIText* presetText = new UIText(presetFilePaths[i].substr(presetNameStart, presetNameLength), 40.0f, osgText::TextBase::CENTER_CENTER);
		presetbutton->addChild(presetText);
		presetUIList->addChild(presetbutton);
		_presetCallbacks.push_back(presetbutton);
	}
	bknd->addChild(presetUIList);
	return presetUIList;
}

void NewVolumeMenu::uiCallback(UICallbackCaller * item)
{	
	if (item == _horizontalflip)
	{
		osg::Matrix m;
		m.makeScale(osg::Vec3(-1, 1, 1));
		_volume->_transform->postMult(m);
		_volume->flipCull();
	}
	else if (item == _verticalflip)
	{
		osg::Matrix m;
		m.makeScale(osg::Vec3(1, 1, -1));
		_volume->_transform->postMult(m);
		_volume->flipCull();
	}
	else if (item == _depthflip)
	{
		osg::Matrix m;
		m.makeScale(osg::Vec3(1, -1, 1));
		_volume->_transform->postMult(m);
		_volume->flipCull();
	}
	else if (item == _organs)
	{
		_volume->getCompute()->getOrCreateStateSet()->setDefine("ORGANS_ONLY", !_organs->isOn());
		_volume->setDirtyAll();
	}
	else if (item == _colon)
	{
		_volume->getCompute()->getOrCreateStateSet()->setDefine("COLON", _colon->isOn());
		_volume->setDirtyAll();
	}
	else if (item == _kidney)
	{
		_volume->getCompute()->getOrCreateStateSet()->setDefine("KIDNEY", _kidney->isOn());
		_volume->setDirtyAll();
	}
	else if (item == _bladder)
	{
		_volume->getCompute()->getOrCreateStateSet()->setDefine("BLADDER", _bladder->isOn());
		_volume->setDirtyAll();
	}
	else if (item == _spleen)
	{
		_volume->getCompute()->getOrCreateStateSet()->setDefine("SPLEEN", _spleen->isOn());
		_volume->setDirtyAll();
	}
	else if (item == _illeum)
	{
		_volume->getCompute()->getOrCreateStateSet()->setDefine("ILLEUM", _illeum->isOn());
		_volume->setDirtyAll();
	}
	else if (item == _aorta)
	{
		_volume->getCompute()->getOrCreateStateSet()->setDefine("AORTA", _aorta->isOn());
		_volume->setDirtyAll();
	}
	else if (item == _addTriangle)
	{
		if (_triangleIndex < 5) {
			_triangleIndex++;
			addRegion();
		}
	}
	
	else if (item == _contrastBottom)
	{
		if (_contrastBottom->getPercent() >= _contrastTop->getPercent())
		{
			_contrastBottom->setPercent(_contrastTop->getPercent() - 0.001f);
		}
		_volume->_computeUniforms["ContrastBottom"]->set(_contrastBottom->getAdjustedValue());
		_colSliderList[0]->setPercentPos(osg::Vec3(_contrastBottom->getAdjustedValue(), -1.0, 0.0));
		
		
		std::string low = std::to_string(_contrastBottom->getAdjustedValue()).substr(0, 4);
		std::string high = std::to_string(_contrastTop->getAdjustedValue()).substr(0, 4);
		_contrastValueLabel->setText("Low " + low + " / " + "High " + high);
		_volume->setDirtyAll();
	}
	else if (item == _contrastTop)
	{
		if (_contrastBottom->getPercent() >= _contrastTop->getPercent())
		{
			_contrastTop->setPercent(_contrastBottom->getPercent() + 0.001f);
		}
		_volume->_computeUniforms["ContrastTop"]->set(_contrastTop->getAdjustedValue());
		_colSliderList[1]->setPercentPos(osg::Vec3(_contrastTop->getAdjustedValue(), -1.0, 0.0));

		std::string low = std::to_string(_contrastBottom->getAdjustedValue()).substr(0, 4);
		std::string high = std::to_string(_contrastTop->getAdjustedValue()).substr(0, 4);
		_contrastValueLabel->setText("Low " + low + " / " + "High " + high);
		_volume->setDirtyAll();
	}
	else if (item == _brightness)
	{
		_volume->_computeUniforms["Brightness"]->set(_brightness->getAdjustedValue());
		float realValue = _brightness->getAdjustedValue() - .5;
		std::string value;
		if (realValue >= 0.0) {
			value = "+" + std::to_string(realValue).substr(0, 4);
		}
		else {
			value = std::to_string(realValue).substr(0, 5);
		}
		
		
		_brightValueLabel->setText(value);
		_volume->setDirtyAll();
	}
	else if (item == _transferFunctionRadial)
	{
		useTransferFunction(_transferFunctionRadial->getCurrent());
	}
	else if (item == _addPreset) {
		savePreset();
	}
	else if (item == _loadPreset) {
		_maskMenu->addChild(_presetPopup->getRootElement());
		_presetPopup->getRootElement()->setPercentPos(osg::Vec3(1.62, 0.0, 0.0));
	}
	else if (checkTriangleCallbacks(item)) {
		return;
	}
	else if (checkTriangleVisCallbacks(item)) {
		return;
	}
	else if (checkPresetCallbacks(item)) {
		return;
	}
	else if (item == _cp) {
		osg::Vec3 col = _cp->returnColor();

		
		_colSliderList[_colorSliderIndex]->setColor(osg::Vec4(col, 1.0));
		if(_colorSliderIndex == 0)
			_volume->_computeUniforms["leftColor"]->set(col);
		else
			_volume->_computeUniforms["rightColor"]->set(col);
		_volume->setDirtyAll();
	}
	else if (checkColorSliderCallbacks(item)) {
		osg::Vec4 col4 = _colSliderList[_colorSliderIndex]->getColor();
		osg::Vec3 col; col.x() = col4.x(); col.y() = col4.y(); col.z() = col4.z();
		_cp->setCPColor(col);
		return;
	}
	
}
void NewVolumeMenu::useTransferFunction(int tfID) {
	if (tfID == 0)
	{
		_transferFunction = "vec3(ra.r);";
		((UIText*)_blacknwhite->getChild(0))->setColor(UI_ACTIVE_COLOR);
		((UIText*)_rainbow->getChild(0))->setColor(UI_INACTIVE_COLOR);
		((UIText*)_custom->getChild(0))->setColor(UI_INACTIVE_COLOR);
		_colSliderList[0]->getGeode()->setNodeMask(0);
		_colSliderList[1]->getGeode()->setNodeMask(0);
		_tentMenu->getRootElement()->getGroup()->removeChild(_cp->getGroup());
		_cp->_parent = nullptr;
	}
	else if (tfID == 1)
	{
		_transferFunction = "hsv2rgb(vec3(ra.r * 0.8, 1, 1));";
		((UIText*)_blacknwhite->getChild(0))->setColor(UI_INACTIVE_COLOR);
		((UIText*)_rainbow->getChild(0))->setColor(UI_ACTIVE_COLOR);
		((UIText*)_custom->getChild(0))->setColor(UI_INACTIVE_COLOR);
		_colSliderList[0]->getGeode()->setNodeMask(0);
		_colSliderList[1]->getGeode()->setNodeMask(0);
		_tentMenu->getRootElement()->getGroup()->removeChild(_cp->getGroup());
		_cp->_parent = nullptr;
	}
	else if (tfID == 2)
	{
		_transferFunction = "custom(vec3(ra.r));";;
		((UIText*)_blacknwhite->getChild(0))->setColor(UI_INACTIVE_COLOR);
		((UIText*)_rainbow->getChild(0))->setColor(UI_INACTIVE_COLOR);
		((UIText*)_custom->getChild(0))->setColor(UI_ACTIVE_COLOR);
		_colSliderList[0]->getGeode()->setNodeMask(0xffffffff);
		_colSliderList[1]->getGeode()->setNodeMask(0xffffffff);
		_tentMenu->addChild(_cp);
	}
	_volume->getCompute()->getOrCreateStateSet()->setDefine("COLOR_FUNCTION", _transferFunction, osg::StateAttribute::ON);
	_colorDisplay->setShaderDefine("COLOR_FUNCTION", _transferFunction, osg::StateAttribute::ON);
	_opacityDisplay->setShaderDefine("COLOR_FUNCTION", _transferFunction, osg::StateAttribute::ON);
	_opacityColorDisplay->setShaderDefine("COLOR_FUNCTION", _transferFunction, osg::StateAttribute::ON);
	upDatePreviewDefines(_transferFunction);
	_volume->setDirtyAll();

	osg::Vec3 pos = _container->getPosition();
	std::cout << "x " << pos.x() << "y " << pos.y() << "z " << pos.z() << std::endl;
}

void NewVolumeMenu::upDatePreviewDefines(std::string tf) {
	for (int i = 0; i < _triangleIndex+1; i++) {
		ShaderQuad* sq = (ShaderQuad*)_triangleCallbacks[i]->getChild(2);
		sq->setShaderDefine("COLOR_FUNCTION", tf, osg::StateAttribute::ON);
	}
}

bool NewVolumeMenu::checkTriangleCallbacks(UICallbackCaller* item) {
	bool found = false;
	int i = 0;
	while (i <	_triangleIndex+1) {
		if (item == _triangleCallbacks[i]) {
			found = true;
			_tentWindow->setTent(i);
			break;
		}
		i++;
	}
	return found;
}

bool NewVolumeMenu::checkTriangleVisCallbacks(UICallbackCaller* item) {
	bool found = false;
	int i = 0;
	while (i < _triangleIndex + 1) {
		if (item == (VisibilityToggle*)_triangleCallbacks[i]->getChild(1)) {
			found = true;
			_tentWindow->toggleTent(i);
			break;
		}
		i++;
	}
	return found;
}

bool NewVolumeMenu::checkPresetCallbacks(UICallbackCaller* item) {
	bool found = false;
	for (int i = 0; i < _presetCallbacks.size(); i++) {
		if (item == _presetCallbacks[i]) {
			usePreset(((UIText*)_presetCallbacks[i]->getChild(0))->getText());
			_maskMenu->getRootElement()->getGroup()->removeChild(_presetPopup->getRootElement()->getGroup());
			_presetPopup->getRootElement()->_parent = nullptr;
			found = true;
			break;
		}
	}
	return found;
}

bool NewVolumeMenu::checkColorSliderCallbacks(UICallbackCaller* item) {
	bool found = false;
	for (int i = 0; i < _colSliderList.size(); i++) {
		if (item == _colSliderList[i]) {
			_colorSliderIndex = i;
			found = true;
			break;
		}
	}
	return found;
}

//#include <sys/types.h>
//#include <sys/stat.h>

void NewVolumeMenu::usePreset(std::string filename) {
	_tentWindow->clearTents();
	clearRegionPreviews();
	_triangleIndex = -1;
	std::string currPath = cvr::ConfigManager::getEntry("Plugin.HelmsleyVolume.PresetsFolder", "C:/", false);
	std::string presetPath = currPath + "\\" + filename + ".yml";
	YAML::Node config = YAML::LoadFile(presetPath);

	std::string datasetPath = config["series name"].as<std::string>();
	FileSelector* fs = HelmsleyVolume::instance()->getFileSelector();
	fs->loadVolumeOnly(datasetPath);
	_tentWindow->setVolume(_volume);
	_tentWindowOnly->setVolume(_volume);
	//////////////////opacities//////////////////
	for (int i = 0; i < config["tent count"].as<int>(); i++) {
		_triangleIndex++;
		char presetIndex = '0' + i;
		std::string presetName = "tent ";
		presetName+=presetIndex;
		addRegion();
		float center = config[presetName]["opacity"][0].as<float>();
		float bottomWidth = config[presetName]["opacity"][1].as<float>();
		float topWidth = config[presetName]["opacity"][2].as<float>();
		float height = config[presetName]["opacity"][3].as<float>();
		float lowest = config[presetName]["opacity"][4].as<float>();
		_tentWindow->fillTentDetails(_triangleIndex, center, bottomWidth, topWidth, height, lowest);	
	}

	/////////////////tf & contrast////////////
	int tfid = -1;
	if (config["color scheme"].as<std::string>() == "grayscale") {
		tfid = 0;
	}
	else if (config["color scheme"].as<std::string>() == "color") {
		tfid = 1;
	}
	_transferFunctionRadial->setCurrent(tfid);
	useTransferFunction(tfid);
	float contrastLow = config["contrast"][0].as<float>();
	float contrastHigh = config["contrast"][1].as<float>();
	float brightness = config["contrast"][2].as<float>();
	setContrastValues(contrastLow, contrastHigh, brightness);

	///////////////Masks////////////////////
	if (_volume->hasMask()) {
		_maskMenu->addChild(_maskBknd);
		_maskBknd->setActive(true);

		bool val = config["mask"][0].as<int>();

		if (_organs->isOn() != val)
			_organs->toggle();
		_volume->getCompute()->getOrCreateStateSet()->setDefine("ORGANS_ONLY", !val);
		val = config["mask"][1].as<int>();
		if (_colon->isOn() != val)
			_colon->toggle();
		_volume->getCompute()->getOrCreateStateSet()->setDefine("COLON", val);
		val = config["mask"][2].as<int>();
		if (_kidney->isOn() != val)
			_kidney->toggle();
		_volume->getCompute()->getOrCreateStateSet()->setDefine("KIDNEY", val);
		val = config["mask"][3].as<int>();
		if (_bladder->isOn() != val)
			_bladder->toggle();
		_volume->getCompute()->getOrCreateStateSet()->setDefine("BLADDER", val);
		val = config["mask"][4].as<int>();
		if (_spleen->isOn() != val)
			_spleen->toggle();
		_volume->getCompute()->getOrCreateStateSet()->setDefine("SPLEEN", val);
		val = config["mask"][5].as<int>();
		if (_illeum->isOn() != val)
			_illeum->toggle();
		_volume->getCompute()->getOrCreateStateSet()->setDefine("ILLEUM", val);
		val = config["mask"][6].as<int>();
		if (_aorta->isOn() != val)
			_aorta->toggle();
		_volume->getCompute()->getOrCreateStateSet()->setDefine("AORTA", val);

		_volume->setDirtyAll();
	}
	else {
		_maskMenu->getRootElement()->getGroup()->removeChild(_maskBknd->getGroup());
		_maskBknd->_parent = nullptr;
		_maskBknd->setActive(false);
	}

	////////////Cutting Plane/////////////
	ToolToggle* cuttingPlaneTool = _toolMenu->getCuttingPlaneTool();
	try {
		if (config["cutting plane position"].as<std::string>() != "NA") {
			cuttingPlaneTool->getIcon()->setColor(osg::Vec4(0.0, 0.0, 0.0, 1));
		}
	}
	catch(...){
		osg::Vec3 cutPpos;
		cutPpos.x() = config["cutting plane position"][0].as<float>();
		cutPpos.y() = config["cutting plane position"][1].as<float>();
		cutPpos.z() = config["cutting plane position"][2].as<float>();

		osg::Quat cutPQuat;
		cutPQuat.x() = config["cutting plane rotation"][0].as<float>();
		cutPQuat.y() = config["cutting plane rotation"][1].as<float>();
		cutPQuat.z() = config["cutting plane rotation"][2].as<float>();
		cutPQuat.w() = config["cutting plane rotation"][3].as<float>();

		
		if (!cuttingPlaneTool->isOn())
			cuttingPlaneTool->toggle();
		else
			CuttingPlane* cutP = HelmsleyVolume::instance()->HelmsleyVolume::createCuttingPlane(0);
		cuttingPlaneTool->getIcon()->setColor(osg::Vec4(0.1, 0.4, 0.1, 1));
		//CuttingPlane* cutP = HelmsleyVolume::instance()->HelmsleyVolume::createCuttingPlane(0);
		CuttingPlane* cutP = HelmsleyVolume::instance()->HelmsleyVolume::getCuttingPlanes()[0];
		cutP->setPosition(cutPpos);
		cutP->setRotation(cutPQuat);
		
	}
}

void NewVolumeMenu::clearRegionPreviews() {
	for(int i = 0; i < _triangleIndex+1; i++) {
		_triangleList->removeChild(_triangleCallbacks[i]);
	}
}

Tent* NewVolumeMenu::addRegion() {
	_triangleCallbacks[_triangleIndex] = new CallbackButton();
	_triangleCallbacks[_triangleIndex]->setCallback(this);
	_triangleCallbacks[_triangleIndex]->setPercentPos(osg::Vec3(0.20, 0.0, 0.0));

	osg::Vec3 color = triangleColors[_triangleIndex];
	std::string name = "Triangle " + std::to_string(_triangleIndex);
	UIText* label = new UIText(name, 50.0f, osgText::TextBase::LEFT_TOP);
	label->setColor(osg::Vec4(color, 1.0));
	VisibilityToggle* vT = new VisibilityToggle("");
	vT->getChild(0)->setPercentSize(osg::Vec3(1, 1, 1));
	vT->getChild(0)->setPercentPos(osg::Vec3(0, 0, 0));
	vT->setCallback(this);
	vT->setPercentPos(osg::Vec3(-0.20, 0.0, 0.1));
	vT->setPercentSize(osg::Vec3(0.185, 0.0, 0.5));
	vT->toggle();

	Tent* tent = _tentWindow->addTent(_triangleIndex, color);

	ShaderQuad* sq = new ShaderQuad();
	std::string frag = HelmsleyVolume::loadShaderFile("shaderQuadPreview.frag");
	std::string vert = HelmsleyVolume::loadShaderFile("transferFunction.vert");
	osg::Program* p = new osg::Program;
	p->setName("trianglePreview");
	p->addShader(new osg::Shader(osg::Shader::VERTEX, vert));
	p->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag));
	sq->setProgram(p);
	sq->setPercentSize(osg::Vec3(0.5, 1.0, 0.4));
	sq->setPercentPos(osg::Vec3(.27, 0.0, 0.1));
	sq->addUniform(tent->_centerUniform);
	sq->addUniform(tent->_widthUniform);
	sq->setShaderDefine("COLOR_FUNCTION", _transferFunction, osg::StateAttribute::ON);

	sq->addUniform(_volume->_computeUniforms["ContrastBottom"]);
	sq->addUniform(_volume->_computeUniforms["ContrastTop"]);
	sq->addUniform(_volume->_computeUniforms["Brightness"]);



	_triangleList->addChild(_triangleCallbacks[_triangleIndex]);



	_triangleCallbacks[_triangleIndex]->addChild(label);
	_triangleCallbacks[_triangleIndex]->addChild(vT);
	_triangleCallbacks[_triangleIndex]->addChild(sq);

	return tent;
}

void NewVolumeMenu::setContrastValues(float contrastLow, float contrastHigh, float brightness) {
	_volume->_computeUniforms["ContrastBottom"]->set(contrastLow);
	_volume->_computeUniforms["ContrastTop"]->set(contrastHigh);
	_volume->_computeUniforms["Brightness"]->set(brightness);

	float realValue = brightness - .5;
	std::string value;
	if (realValue >= 0.0) {
		value = "+" + std::to_string(realValue).substr(0, 4);
	}
	else {
		value = std::to_string(realValue).substr(0, 5);
	}
	_brightValueLabel->setText(value);

	std::string low = std::to_string(contrastLow).substr(0, 4);
	std::string high = std::to_string(contrastHigh).substr(0, 4);
	_contrastValueLabel->setText("Low " + low + " / " + "High " + high);
	_volume->setDirtyAll();
}

void NewVolumeMenu::savePreset(){
	std::vector<float> opacityData;//0=Center, 1=BottomWidth, 2=TopWidth, 3= Height, 4 = Lowest
	std::vector<float> contrastData = { _contrastBottom->getAdjustedValue(), _contrastTop->getAdjustedValue(), _brightness->getAdjustedValue()};
	std::string tf;
	if (_transferFunctionRadial->getCurrent() == 0) {
		tf = "grayscale";
	}
	else {
		tf = "color";
	}
	YAML::Emitter out;
	std::string name = "Preset";
	char nextPresetIndex = '0' + _presetCallbacks.size();
	name += nextPresetIndex;

	out << YAML::BeginMap;
	out << YAML::Key << "Name";
	out << YAML::Value << name;
	out << YAML::Key << "contrast";
	out << YAML::Value << YAML::Flow << contrastData;
	out << YAML::Key << "color scheme";
	out << YAML::Value << tf;
	out << YAML::Key << "tent count";
	out << YAML::Value << _triangleIndex + 1;

	for (int i = 0; i < _triangleIndex + 1; i++) {
		opacityData = _tentWindow->getPresetData(i);
		std::string tentString = "tent ";
		char tentIndex = '0' + i;
		std::string tentStringIndex = tentString += tentIndex;
		out << YAML::Key << tentStringIndex;
		out << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "opacity";
		out << YAML::Value << YAML::Flow << opacityData;
		out << YAML::EndMap;
	}
	///////////////////Cutting Plane///////////////////////
	std::vector<CuttingPlane*> cPs = HelmsleyVolume::instance()->getCuttingPlanes();
	if (!cPs.empty()) {
		osg::Vec3 cPPos = cPs[0]->getPosition();
		std::vector<float> fPos;
		fPos.push_back(cPPos.x());
		fPos.push_back(cPPos.y());
		fPos.push_back(cPPos.z());
		out << YAML::Key << "cutting plane position";
		out << YAML::Value << YAML::Flow << fPos;


		osg::Quat cPRot = cPs[0]->getRotation();
		std::vector<float> fQuat;
		fQuat.push_back(cPRot.x());
		fQuat.push_back(cPRot.y());
		fQuat.push_back(cPRot.z());
		fQuat.push_back(cPRot.w());

		out << YAML::Key << "cutting plane rotation";
		out << YAML::Value << YAML::Flow << fQuat;
	}
	else {
		out << YAML::Key << "cutting plane position";
		out << YAML::Value << "NA";
	}
	///////////////////Masks////////////////////////
	if (_volume->hasMask()) {
		out << YAML::Key << "mask";
		std::vector<unsigned char> maskValues;
		_organs->isOn() ? maskValues.push_back('1') : maskValues.push_back('0');
		_colon->isOn() ? maskValues.push_back('1') : maskValues.push_back('0');
		_kidney->isOn() ? maskValues.push_back('1') : maskValues.push_back('0');
		_bladder->isOn() ? maskValues.push_back('1') : maskValues.push_back('0');
		_spleen->isOn() ? maskValues.push_back('1') : maskValues.push_back('0');
		_illeum->isOn() ? maskValues.push_back('1') : maskValues.push_back('0');
		_aorta->isOn() ? maskValues.push_back('1') : maskValues.push_back('0');
		out << YAML::Value << YAML::Flow << maskValues;
	}


	///////////////////Dataset//////////////////////
	out << YAML::Key << "series name";
	FileSelector* fs = HelmsleyVolume::instance()->getFileSelector();
	std::cout << "currpath " << fs->getCurrPath() << std::endl;
	std::string datasetPath = fs->getCurrPath();
	out << YAML::Value << datasetPath;

	out << YAML::EndMap;

	std::string currPath = cvr::ConfigManager::getEntry("Plugin.HelmsleyVolume.PresetsFolder", "C:/", false);
	std::string presetPath = currPath + "\\" + name + ".yml";
	std::ofstream fout(presetPath);
	fout << out.c_str();

	CallbackButton* presetbutton = new CallbackButton();
	presetbutton->setCallback(this);
	UIText* presetText = new UIText(name, 40.0f, osgText::TextBase::CENTER_CENTER);
	presetbutton->addChild(presetText);
	_presetUIList->addChild(presetbutton);
	_presetCallbacks.push_back(presetbutton);


}

ToolMenu::ToolMenu(int index, bool movable, cvr::SceneObject* parent)
{
	_movable = movable;
	_index = index;

	_menu = new UIPopup();
	if (parent)
	{
		osg::Vec3 volumePos = ConfigManager::getVec3("Plugin.HelmsleyVolume.Orientation.Volume.Position", osg::Vec3(0, 750, 500));
		_menu->setPosition(osg::Vec3(-150, 900, 1500) - volumePos);
	}
	else
	{
		_menu->setPosition(ConfigManager::getVec3("Plugin.HelmsleyVolume.Orientation.ToolMenu.Position", osg::Vec3(-150, 500, 600)));
	}
	_menu->getRootElement()->setAbsoluteSize(ConfigManager::getVec3("Plugin.HelmsleyVolume.Orientation.ToolMenu.Scale", osg::Vec3(300, 1, 100)));

	UIList* list = new UIList(UIList::LEFT_TO_RIGHT, UIList::CUT);
	list->setAbsoluteSpacing(5);
	_menu->addChild(list);

	std::string dir = ConfigManager::getEntry("Plugin.HelmsleyVolume.ImageDir");

	_cuttingPlane = new ToolToggle(dir + "slice.png");
	_cuttingPlane->setColor(UI_BLACK_COLOR);
	_cuttingPlane->setCallback(this);
	list->addChild(_cuttingPlane);

	_measuringTool = new ToolToggle(dir + "ruler.png");
	_measuringTool->setColor(UI_BLACK_COLOR);
	_measuringTool->setCallback(this);
	list->addChild(_measuringTool);

	_screenshotTool = new ToolToggle(dir + "browser.png");
	_screenshotTool->setColor(UI_BLACK_COLOR);
	_screenshotTool->setCallback(this);
	list->addChild(_screenshotTool);

	_centerLIneTool = new ToolToggle(dir + "line.png");
	_centerLIneTool->setCallback(this);
	list->addChild(_centerLIneTool);
	

	if (!_movable && !parent)
	{
		_menu->setActive(true, true);
	}
	else {
		_menu->setActive(true, false);
		_container = new SceneObject("VolumeMenu", false, _movable, false, false, false);
		if (parent)
		{
			parent->addChild(_container);
		}
		else
		{
			PluginHelper::registerSceneObject(_container, "VolumeMenu");
			_container->attachToScene();
		}
		_container->addChild(_menu->getRoot());
		_menu->getRootElement()->updateElement(osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 0));
		_container->dirtyBounds();
	}
}


ToolMenu::~ToolMenu()
{
	_menu->setActive(false, false);
	MenuManager::instance()->removeMenuSystem(_menu);
	delete _menu;

	if (_container)
	{
		_container->detachFromScene();
		delete _container;
	}
}

void ToolMenu::uiCallback(UICallbackCaller* item)
{
	if (item == _screenshotTool)
	{
		osg::Matrix mat = PluginHelper::getHandMat(_screenshotTool->getLastHand());
		osg::Vec4d position = osg::Vec4(0, 300, 0, 1) * mat;
		osg::Vec3f pos = osg::Vec3(position.x(), position.y(), position.z());

		osg::Quat q = osg::Quat();
		osg::Quat q2 = osg::Quat();
		osg::Vec3 v = osg::Vec3();
		osg::Vec3 v2 = osg::Vec3();
		mat.decompose(v, q, v2, q2);

		HelmsleyVolume::instance()->toggleScreenshotTool(_screenshotTool->isOn());
		HelmsleyVolume::instance()->getScreenshotTool()->setRotation(q);
		HelmsleyVolume::instance()->getScreenshotTool()->setPosition(pos);


		if (_screenshotTool->isOn())
		{
			_screenshotTool->getIcon()->setColor(osg::Vec4(0.1, 0.4, 0.1, 1));
		}
		else 
		{
			_screenshotTool->getIcon()->setColor(osg::Vec4(0, 0, 0, 1));
		}
	}
	
	else if (item == _cuttingPlane)
	{
		if (_cuttingPlane->isOn())
		{
			HelmsleyVolume::instance()->createCuttingPlane(0);
			_cuttingPlane->getIcon()->setColor(osg::Vec4(0.1, 0.4, 0.1, 1));
		}
		else
		{
			HelmsleyVolume::instance()->removeCuttingPlane(0);
			_cuttingPlane->getIcon()->setColor(osg::Vec4(0, 0, 0, 1));
		}
	}
	else if (item == _measuringTool)
	{
		if (_measuringTool->isOn())
		{
			HelmsleyVolume::instance()->setTool(HelmsleyVolume::MEASUREMENT_TOOL);
			HelmsleyVolume::instance()->activateMeasurementTool(_index);
			_measuringTool->getIcon()->setColor(osg::Vec4(0.1, 0.4, 0.1, 1));
		}
		else
		{
			HelmsleyVolume::instance()->setTool(HelmsleyVolume::NONE);
			HelmsleyVolume::instance()->deactivateMeasurementTool(_index);
			
			_measuringTool->getIcon()->setColor(osg::Vec4(0, 0, 0, 1));
		}
	}
	else if (item == _centerLIneTool)
	{
		osg::Matrix mat = PluginHelper::getHandMat(_centerLIneTool->getLastHand());
		osg::Vec4d position = osg::Vec4(0, 300, 0, 1) * mat;
		osg::Vec3f pos = osg::Vec3(position.x(), position.y(), position.z());

		osg::Quat q = osg::Quat();
		osg::Quat q2 = osg::Quat();
		osg::Vec3 v = osg::Vec3();
		osg::Vec3 v2 = osg::Vec3();
		mat.decompose(v, q, v2, q2);

		HelmsleyVolume::instance()->toggleCenterlineTool(_centerLIneTool->isOn());
		HelmsleyVolume::instance()->getCenterlineTool()->setRotation(q);
		HelmsleyVolume::instance()->getCenterlineTool()->setPosition(pos);
		if (_centerLIneTool->isOn())
		{
			_centerLIneTool->getIcon()->setColor(osg::Vec4(0.1, 0.4, 0.1, 1));
			HelmsleyVolume::instance()->toggleCenterLine(true);
			HelmsleyVolume::instance()->activateClippingPath();

		}
		else
		{
			_centerLIneTool->getIcon()->setColor(osg::Vec4(0, 0, 0, 1));
			HelmsleyVolume::instance()->toggleCenterLine(false);
		}
	}
}