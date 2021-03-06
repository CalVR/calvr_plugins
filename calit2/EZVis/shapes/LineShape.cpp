#include "LineShape.h"

#include <osg/Geometry>
#include <osg/Material>

#include <string>
#include <vector>

LineShape::LineShape(std::string command, std::string name) 
{
    // check for changed values
    createParameter("pos1", new Vec3Type());
    createParameter("pos2", new Vec3Type());
    createParameter("color", new Vec4Type());
    createParameter("width", new FloatType());

    _type = SimpleShape::LINE;

    BasicShape::setName(name);
    
    _vertices = new osg::Vec3Array(2);
    _colors = new osg::Vec4Array(2);
    _width = new osg::LineWidth(); // default width is 1
   
    setPosition(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(1.0, 0.0, 0.0));
    setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0),osg::Vec4(0.0, 1.0, 0.0, 1.0));
    update(command);

    setVertexArray(_vertices); 
    setColorArray(_colors); 
    setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

    osg::StateSet* state = getOrCreateStateSet();
    osg::Material* mat = new osg::Material();
    //state->setMode(GL_BLEND, osg::StateAttribute::ON);
    state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    state->setAttribute(_width);
}

LineShape::~LineShape()
{
}

void LineShape::setWidth(float width)
{
    _width->setWidth(width);
}

void LineShape::setPosition(osg::Vec3 p0, osg::Vec3 p1)
{
    (*_vertices)[0].set(p0[0], p0[1], p0[2]);    
    (*_vertices)[1].set(p1[0], p1[1], p1[2]);    
}

void LineShape::setColor(osg::Vec4 c0, osg::Vec4 c1)
{
    (*_colors)[0].set(c0[0], c0[1], c0[2], c0[3]);    
    (*_colors)[1].set(c1[0], c1[1], c1[2], c1[3]);    

    if( (c0[3] != 1.0) || (c1[3] != 1.0))
        getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    else
        getOrCreateStateSet()->setRenderingHint(osg::StateSet::DEFAULT_BIN);


}

void LineShape::update(std::string command)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _dirty = true;

    // check for changed values
    setParameter(command, "pos1");
    setParameter(command, "pos2");
    setParameter(command, "color");
    setParameter(command, "width");
}

void LineShape::update()
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if( !_dirty )
        return;

    float width = _width->getWidth();
    osg::Vec3 p1((*_vertices)[0]);
    osg::Vec4 c1((*_colors)[0]);

    // get values out
    p1 = getParameter("pos1")->asVec3Type()->getValue();
    c1 = getParameter("color")->asVec4Type()->getValue();
    
	osg::Vec3 p2((*_vertices)[1]);
    osg::Vec4 c2((*_colors)[1]);
    
    p2 = getParameter("pos2")->asVec3Type()->getValue();
    c2 = getParameter("color")->asVec4Type()->getValue();

    width = getParameter("width")->asFloatType()->getValue();

    setPosition(p1, p2);
    setColor(c1, c2);
    setWidth(width);

	_colors->dirty();
	_vertices->dirty();
    dirtyBound();
    
	// reset flag
    _dirty = false;
}

