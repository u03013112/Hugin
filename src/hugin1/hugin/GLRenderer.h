// -*- c-basic-offset: 4 -*-
/** @file GLRenderer.h
 *
 *  @author James Legg
 *
 *  This is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* The renderer handles drawing the preview. It is used by a GLViewer, which is
 * a wxWidget. The work of generating textures to represent the image is done by
 * a TextureManager, and the remappings are made in display lists by a
 * MeshManager. The GLViewer gives us instances of those objects to use.
 */

#ifndef _GLRENDERER_H
#define _GLRENDERER_H


// TODO needed?
//#include <vector>
/* something messed up... temporary fix :-( */
#include "hugin_utils/utils.h"
#define DEBUG_HEADER ""
#include <base_wx/ImageCache.h>
#include <vigra_ext/ROIImage.h>
#include <vigra/diff2d.hxx>
#include <utility>

class PreviewToolHelper;

class GLRenderer
{
public:
    /** ctor.
     */
    GLRenderer(PT::Panorama * pano, TextureManager *tex_man,
               MeshManager *mesh_man, ViewState *veiw_state,
               PreviewToolHelper *tool_helper);

    /** dtor.
     */
    virtual ~GLRenderer();
    // resize the viewport, because the window's dimensions have changed.
    // returns the number of screen pixels until the start of the panorma,
    // both horizontally and vertically.
    vigra::Diff2D Resize(int width, int height);
    // void panoramaChanged(PT::Panorama &pano);
    // void panoramaImagesChanged(PT::Panorama &pano, const PT::UIntSet & imgNr);
    void Redraw();
    void SetBackground(unsigned char red, unsigned char green, unsigned char blue);
    float width_o, height_o;
private:
    PT::Panorama  * m_pano;
    TextureManager * m_tex_man;
    MeshManager * m_mesh_man;
    ViewState * m_view_state;
    PreviewToolHelper *m_tool_helper;
    int width, height;
};

#endif
