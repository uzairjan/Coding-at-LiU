/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Created between 2005 and 2012 by The Voreen Team                   *
 * as listed in CREDITS.TXT <http://www.voreen.org>                   *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#ifndef VRN_VOLUME3XCOMBINER_H
#define VRN_VOLUME3XCOMBINER_H

#include "voreen/core/processors/processor.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

class Volume3xCombiner : public Processor {
public:
    Volume3xCombiner();
    std::string getClassName() const   { return "Volume3xCombiner";      }
    std::string getCategory() const    { return "Volume"               ; }
    CodeState getCodeState() const     { return CODE_STATE_EXPERIMENTAL; }

    Processor* create() const          { return new Volume3xCombiner;    }

protected:
    void process();

	VolumePort _inportX;
	VolumePort _inportY;
	VolumePort _inportZ;
	VolumePort _outport;
};

} // namespace

#endif // VRN_VOLUME3XCOMBINER_H
