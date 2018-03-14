/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
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

#ifndef __OPENSPACE_MODULE_MULTIRESVOLUME___LOCALTFBRICKSELECTOR___H__
#define __OPENSPACE_MODULE_MULTIRESVOLUME___LOCALTFBRICKSELECTOR___H__

#include <modules/multiresvolume/rendering/tspbrickselector.h>

#include <vector>

namespace openspace {

class LocalErrorHistogramManager;

class LocalTfBrickSelector : public TSPBrickSelector {
public:
    struct Error {
        float spatial;
        float temporal;
        
        void set(float a, TSP::NodeType nodeType) {
            switch (nodeType) {
            case TSP::NodeType::SPATIAL:
                spatial = a;
                break;
            case TSP::NodeType::TEMPORAL:
                temporal = a;
                break;
            default:
                return;
            }
        }

        float get(TSP::NodeType nodeType) {
            switch (nodeType) {
            case TSP::NodeType::SPATIAL:
                return spatial;
            case TSP::NodeType::TEMPORAL:
                return temporal;
            default:
                return -1.;
            }
        }
    };

    LocalTfBrickSelector(std::shared_ptr<TSP> tsp, LocalErrorHistogramManager* hm, TransferFunction* tf, int memoryBudget, int streamingBudget);
    ~LocalTfBrickSelector();

    virtual bool initialize();

    void selectBricks(int timestep, std::vector<int>& bricks);
    bool calculateBrickErrors();

private:
    LocalErrorHistogramManager* _histogramManager;
    std::vector<Error> _brickErrors;

    float spatialSplitPoints(unsigned int brickIndex);
    float temporalSplitPoints(unsigned int brickIndex);
    float splitPoints(unsigned int brickIndex, BrickSelection::SplitType& splitType);
};

} // namespace openspace

#endif // __OPENSPACE_MODULE_MULTIRESVOLUME___LOCALTFBRICKSELECTOR___H__
