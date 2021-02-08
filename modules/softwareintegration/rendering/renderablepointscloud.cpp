/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2021                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <modules/softwareintegration/rendering/renderablepointscloud.h>

#include <openspace/documentation/verifier.h>
#include <openspace/engine/globals.h>
#include <openspace/rendering/renderengine.h>
#include <openspace/util/distanceconstants.h>
#include <openspace/util/updatestructures.h>
#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/opengl/openglstatecache.h>
#include <ghoul/opengl/programobject.h>

#include <fstream>

namespace {
    constexpr const char* _loggerCat = "PointsCloud";

    constexpr int8_t CurrentCacheVersion = 1;

    constexpr openspace::properties::Property::PropertyInfo ColorInfo = {
        "Color",
        "Color",
        "The color of the points."
    };

    constexpr openspace::properties::Property::PropertyInfo SizeInfo = {
        "Size",
        "Size",
        "The size of the points."
    };

    constexpr openspace::properties::Property::PropertyInfo ToggleVisibilityInfo = {
        "ToggleVisibility",
        "Toggle Visibility",
        "Enables/Disables the drawing of points."
    };

    constexpr openspace::properties::Property::PropertyInfo DataInfo = {
        "Data",
        "Data",
        "Data to use for the positions of the points, given in Parsec."
    };
} // namespace

namespace openspace {

documentation::Documentation RenderablePointsCloud::Documentation() {
    using namespace documentation;
    return {
        "RenderablePointsCloud",
        "softwareintegration_renderable_pointscloud",
        {
            {
                ColorInfo.identifier,
                new DoubleVector3Verifier,
                Optional::Yes,
                ColorInfo.description
            },
            {
                SizeInfo.identifier,
                new DoubleVerifier,
                Optional::Yes,
                SizeInfo.description
            },
            {
                ToggleVisibilityInfo.identifier,
                new BoolVerifier,
                Optional::Yes,
                ToggleVisibilityInfo.description
            },
            {
                DataInfo.identifier,
                new Vector3ListVerifier<double>,
                Optional::No,
                DataInfo.description
            }
        }
    };
}

RenderablePointsCloud::RenderablePointsCloud(const ghoul::Dictionary& dictionary)
    : Renderable(dictionary)
    , _color(ColorInfo, glm::vec3(0.5f), glm::vec3(0.f), glm::vec3(1.f))
    , _size(SizeInfo, 1.f, 0.f, 150.f)
    , _isVisible(ToggleVisibilityInfo, true)
{
    documentation::testSpecificationAndThrow(
        Documentation(),
        dictionary,
        "RenderablePointsCloud"
    );

    if (dictionary.hasKey(ColorInfo.identifier)) {
        _color = dictionary.value<glm::dvec3>(ColorInfo.identifier);
    }
    _color.setViewOption(properties::Property::ViewOptions::Color);
    addProperty(_color);

    ghoul::Dictionary d = dictionary.value<ghoul::Dictionary>(DataInfo.identifier);
    for (int i = 0; i < static_cast<int>(d.size()); ++i) {
        const std::string key = std::to_string(i + 1);
        _pointData.push_back(d.value<glm::dvec3>(key));
    }

    addProperty(_opacity);

    if (dictionary.hasKey(SizeInfo.identifier)) {
        _size = static_cast<float>(dictionary.value<double>(SizeInfo.identifier));
    }
    addProperty(_size);

    if (dictionary.hasKey(ToggleVisibilityInfo.identifier)) {
        _isVisible = dictionary.value<bool>(ToggleVisibilityInfo.identifier);
    }
    addProperty(_isVisible);
}

bool RenderablePointsCloud::isReady() const {
    return _shaderProgram && (!_fullData.empty());
}

void RenderablePointsCloud::initialize() {
    loadData();
}

void RenderablePointsCloud::initializeGL() {
    _shaderProgram = global::renderEngine->buildRenderProgram(
        "PointsCloud",
        absPath("${MODULE_SOFTWAREINTEGRATION}/shaders/point_vs.glsl"),
        absPath("${MODULE_SOFTWAREINTEGRATION}/shaders/point_fs.glsl")
    );
}

void RenderablePointsCloud::deinitializeGL() {
    glDeleteVertexArrays(1, &_vertexArrayObjectID);
    _vertexArrayObjectID = 0;

    glDeleteBuffers(1, &_vertexBufferObjectID);
    _vertexBufferObjectID = 0;

    if (_shaderProgram) {
        global::renderEngine->removeRenderProgram(_shaderProgram.get());
        _shaderProgram = nullptr;
    }
}

void RenderablePointsCloud::render(const RenderData& data, RendererTasks&) {
    if (_fullData.empty()) {
        return;
    }

    if (!_isVisible) {
        return;
    }

    _shaderProgram->activate();

    glm::dmat4 modelTransform =
        glm::translate(glm::dmat4(1.0), data.modelTransform.translation) * // Translation
        glm::dmat4(data.modelTransform.rotation) *  // Spice rotation
        glm::scale(glm::dmat4(1.0), glm::dvec3(data.modelTransform.scale));

    glm::dmat4 modelViewTransform = data.camera.combinedViewMatrix() * modelTransform;

    _shaderProgram->setUniform("modelViewTransform", modelViewTransform);
    _shaderProgram->setUniform(
        "MVPTransform",
        glm::dmat4(data.camera.projectionMatrix()) * modelViewTransform
    );

    _shaderProgram->setUniform("color", _color);
    _shaderProgram->setUniform("opacity", _opacity);
    _shaderProgram->setUniform("size", _size);

    // Changes GL state:
    glEnablei(GL_BLEND, 0);
    glDepthMask(false);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE); // Enable gl_PointSize in vertex

    glBindVertexArray(_vertexArrayObjectID);
    const GLsizei nPoints = static_cast<GLsizei>(_fullData.size() / _nValuesPerPoint);
    glDrawArrays(GL_POINTS, 0, nPoints);

    glBindVertexArray(0);
    _shaderProgram->deactivate();

    // Restores GL State
    global::renderEngine->openglStateCache().resetBlendState();
    global::renderEngine->openglStateCache().resetDepthState();
}

void RenderablePointsCloud::update(const UpdateData&) {
    if (!_isDirty) {
        return;
    }

    LDEBUG("Regenerating data");

    createDataSlice();

    int size = static_cast<int>(_slicedData.size());

    if (_vertexArrayObjectID == 0) {
        glGenVertexArrays(1, &_vertexArrayObjectID);
        LDEBUG(fmt::format("Generating Vertex Array id '{}'", _vertexArrayObjectID));
    }
    if (_vertexBufferObjectID == 0) {
        glGenBuffers(1, &_vertexBufferObjectID);
        LDEBUG(fmt::format("Generating Vertex Buffer Object id '{}'", _vertexBufferObjectID));
    }

    glBindVertexArray(_vertexArrayObjectID);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferObjectID);
    glBufferData(
        GL_ARRAY_BUFFER,
        size * sizeof(float),
        _slicedData.data(),
        GL_STATIC_DRAW
    );

    GLint positionAttribute = _shaderProgram->attributeLocation("in_position");

    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribPointer(
        positionAttribute,
        4,
        GL_FLOAT,
        GL_FALSE,
        0,
        nullptr
    );

    glBindVertexArray(0);

    _isDirty = false;
}

void RenderablePointsCloud::createDataSlice() {
    _slicedData.clear();
    _slicedData.reserve(4 * (_fullData.size() / _nValuesPerPoint));

    auto addPosition = [&](const glm::vec4& pos) {
        for (int j = 0; j < 4; ++j) {
            _slicedData.push_back(pos[j]);
        }
    };

    for (size_t i = 0; i < _fullData.size(); i += _nValuesPerPoint) {
        glm::dvec4 transformedPos = glm::dvec4(
            _fullData[i + 0],
            _fullData[i + 1],
            _fullData[i + 2],
            1.0
        );
        // W-normalization
        transformedPos /= transformedPos.w;
        transformedPos *= openspace::distanceconstants::Parsec;
        addPosition(transformedPos);
    }
}

void RenderablePointsCloud::loadData() {
    if (_pointData.empty()) {
        LWARNING("No point data found");
        return;
    }

    _nValuesPerPoint = 3;

    int dataSize = _pointData.size();
    std::vector<float> values(_nValuesPerPoint);

    for (int i = 0; i < dataSize; i++) {
        for (int j = 0; j < _nValuesPerPoint; j++) {
            values.push_back(_pointData[i][j]);
        }
    }

    // @TODO: potentially combine with additional data about the points, such as
    // velocity and luminosity

    _fullData.insert(_fullData.end(), values.begin(), values.end());
    _isDirty = true;
}

} // namespace openspace
