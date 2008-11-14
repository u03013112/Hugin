// -*- c-basic-offset: 4 -*-
/** @file PreviewIdentifyTool.cpp
 *
 *  @author James Legg
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "panoinc_WX.h"
#include "panoinc.h"

#include "PreviewIdentifyTool.h"
#include <config.h>

#include "base_wx/platform.h"
#include "GLPreviewFrame.h"
#include "MainFrame.h"

#include <wx/platform.h>
#ifdef __WXMAC__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <algorithm>
#include <vector>

// the size of the rectangular texture. Must be a power of two, and at least 8.
#define rect_ts 64
// the number of times larger the circular texture is, must be a power of 2, and
// at least 1. Making better improves the appearance of the circle.
#define circle_ts_multiple 4
#define circle_ts (rect_ts * circle_ts_multiple)
#define circle_middle ((float) (circle_ts - 1) / 2.0)
#define circle_border_outer (circle_middle - 0.5 * (float) circle_ts_multiple)
#define circle_border_inner (circle_middle - 2.5 * (float) circle_ts_multiple)
#define circle_border_peak (circle_middle - 1.5 * (float) circle_ts_multiple)

PreviewIdentifyTool::PreviewIdentifyTool(PreviewToolHelper *helper,
                                         GLPreviewFrame *owner)
    : PreviewTool(helper)
{
    preview_frame = owner;
    // make the textures. We have a circle border and a square one.
    // the textures are white with a the alpha chanel forming a border.
    glGenTextures(1, (GLuint*) &rectangle_border_tex);
    glGenTextures(1, (GLuint*) &circle_border_tex);
    // we only want to specify alpha, but using just alpha in opengl attaches 0
    // for the luminosity. I tried biasing the red green and blue values to get
    // them to 1.0, but it didn't work under OS X for some reason. Instead we
    // use a luminance alpha pair, and use 1.0 for luminance all the time.
    {
        // In the rectangle texture, the middle is 1/4 opaque, the outer pixels
        // are completely transparent, and one pixel in from the edges is
        // a completly opaque line.
        unsigned char rect_tex_data[rect_ts][rect_ts][2];
        // make everything white
        for (unsigned int x = 0; x < rect_ts; x++)
        {
            for (unsigned int y = 0; y < rect_ts; y++)
            {
                rect_tex_data[x][y][0] = 255;
            }
        }
        // now set the middle of the mask semi transparent
        for (unsigned int x = 2; x < rect_ts - 2; x++)
        {
            for (unsigned int y = 2; y < rect_ts - 2; y++)
            {
                rect_tex_data[x][y][1] = 63;
            }
        }
        // make an opaque border
        for (unsigned int d = 1; d < rect_ts - 1; d++)
        {
            rect_tex_data[d][1][1] = 255;
            rect_tex_data[d][rect_ts - 2][1] = 255;
            rect_tex_data[1][d][1] = 255;
            rect_tex_data[rect_ts - 2][d][1] = 255;
        }
        // make a transparent border around that
        for (unsigned int d = 0; d < rect_ts; d++)
        {
            rect_tex_data[d][0][1] = 0;
            rect_tex_data[d][rect_ts - 1][1] = 0;
            rect_tex_data[0][d][1] = 0;
            rect_tex_data[rect_ts - 1][d][1] = 0;
        }
        glBindTexture(GL_TEXTURE_2D, rectangle_border_tex);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE_ALPHA, rect_ts, rect_ts,
                          GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, rect_tex_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
    }
    {
        // the circular one should look similar to the rectangle one, but we
        // enlarge it so that the circle apears less blocky. We don't want to
        // make it equally sharper however, so we make it a bit fuzzier by
        // blending.
        unsigned char circle_tex_data[circle_ts][circle_ts][2];
        for (unsigned int x = 0; x < circle_ts; x++)
        {
            for (unsigned int y = 0; y < circle_ts; y++)
            {
                float x_offs = (float) x - circle_middle,
                      y_offs = (float) y - circle_middle,
                      radius = sqrt(x_offs * x_offs + y_offs * y_offs),
                      intensity;
                if (radius < circle_border_inner)
                {
                    intensity = 63.0;
                } else if (radius < circle_border_peak)
                {
                    intensity = (radius - circle_border_inner) /
                          (float) circle_ts_multiple * 255.0 * 3.0 / 4.0 + 64.0;
                } else if (radius < circle_border_outer)
                {
                    intensity = (radius - circle_border_peak) /
                                    (float) circle_ts_multiple * -255.0 + 256.0;
                } else
                {
                    intensity = 0.0;
                }
                circle_tex_data[x][y][0] = 255;
                circle_tex_data[x][y][1] = (unsigned char) intensity;
            }
        }
        glBindTexture(GL_TEXTURE_2D, circle_border_tex);
        gluBuild2DMipmaps(GL_TEXTURE_2D,
                          GL_LUMINANCE_ALPHA, circle_ts, circle_ts,
                          GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, circle_tex_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
    }
}
PreviewIdentifyTool::~PreviewIdentifyTool()
{
    // free the textures
    glDeleteTextures(1, (GLuint*) &rectangle_border_tex);
    glDeleteTextures(1, (GLuint*) &circle_border_tex);
}

void PreviewIdentifyTool::Activate()
{
    // register notifications
    helper->NotifyMe(PreviewToolHelper::MOUSE_MOVE, this);
    helper->NotifyMe(PreviewToolHelper::DRAW_OVER_IMAGES, this);
    helper->NotifyMe(PreviewToolHelper::IMAGES_UNDER_MOUSE_CHANGE, this);
    helper->NotifyMe(PreviewToolHelper::MOUSE_PRESS, this);
    
    // clear up
    // Assume that there are no images under the mouse when the tool is
    // activated. This should be fine if the user clicks the button to activate
    // the tool.
    image_set.clear();
    mouse_is_over_button = false;
    /* TODO if it becomes possible to activate the tool by a keyboard shortcut
     * or something, call ImagesUnderMouseChangedEvent() to make sure we display
     * indicators for images currently under the cursor. */
     
     helper->SetStatusMessage(_("Move the mouse over the images or image buttons to identify them."));
}

void PreviewIdentifyTool::ImagesUnderMouseChangedEvent()
{
    // If we are currently showing indicators for some of the images, we want
    // to work out which ones are not in the new set, so we can set their
    // buttons back to the system colour.
    std::set<unsigned int> new_image_set = helper->GetImageNumbersUnderMouse();
    std::vector<unsigned int>::iterator end;
    {
        std::vector<unsigned int> difference(image_set.size());
        end = difference.begin();
        end = std::set_difference (image_set.begin(), image_set.end(),
                                   new_image_set.begin(), new_image_set.end(),
                                   difference.begin());
        DEBUG_ASSERT(end >= difference.begin() && end <= difference.end());
        if (difference.begin() != end)
        {
            std::vector<unsigned int>::iterator iterator;
            for (iterator = difference.begin(); iterator != end; iterator++)
            {
                DEBUG_ASSERT(*iterator < helper->GetPanoramaPtr()->getNrOfImages());
                // reset this button to its default system colour.
                preview_frame->SetImageButtonColour(*iterator, 0, 0, 0);
                // remove the notification
                helper->DoNotNotifyMeBeforeDrawing(*iterator, this);
            }
        }
    }
    // now request to be notified when drawing the new ones.
    {
        std::vector<unsigned int> difference(new_image_set.size());
        end = difference.begin();
        end = std::set_difference (new_image_set.begin(), new_image_set.end(),
                                   image_set.begin(), image_set.end(),
                                   difference.begin());
        DEBUG_ASSERT(end >= difference.begin() && end <= difference.end());
        if (difference.begin() != end)
        {
            std::vector<unsigned int>::iterator iterator;
            for (iterator = difference.begin(); iterator != end; iterator++)
            {
                DEBUG_ASSERT(*iterator < helper->GetPanoramaPtr()->getNrOfImages());
                // get notification of when this is about to be drawn.
                helper->NotifyMeBeforeDrawing(*iterator, this);
            }
        }
    }
    // remember the new set.
    image_set.swap(new_image_set);
    
    // Redraw with new indicators. Since the indicators aren't part of the
    // panorama and none of it is likely to change, we have to persuade the
    // viewstate that a redraw is required.
    helper->GetViewStatePtr()->ForceRequireRedraw();
    helper->GetViewStatePtr()->Redraw();
    
    // if there is exactly two images, tell the user they can click to edit CPs.
    if (image_set.size() == 2)
    {
         helper->SetStatusMessage(_("Click to create or edit control points here."));
    } else {
         helper->SetStatusMessage(_("Move the mouse over the images or image buttons to identify them."));
    }
}

void PreviewIdentifyTool::AfterDrawImagesEvent()
{
    // we draw the partly transparent identification boxes over the top of the
    // entire stack of images in image_set so that the extents of images in the
    // background are clearly marked.
    unsigned int num_images = image_set.size();
    // draw the actual images
    // the preview draws them in reverse order, so the lowest numbered appears
    // on top. We will folow this convention to avoid confusion.
    std::set<unsigned int>::reverse_iterator it;
    for (it = image_set.rbegin(); it != image_set.rend(); it++)
    {
        DEBUG_ASSERT(*it < helper->GetPanoramaPtr()->getNrOfImages());
        helper->GetViewStatePtr()->GetTextureManager()->
                DrawImage(*it,
                         helper->GetViewStatePtr()->GetMeshDisplayList(*it));
    }
    // now draw the identification boxes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_TEXTURE);
    unsigned int image_counter = 0;
    for (it = image_set.rbegin(); it != image_set.rend(); it++)
    {
        // we want to shift the texture so it lines up with the cropped region.
        glPushMatrix();
        HuginBase::SrcPanoImage *src = helper->GetViewStatePtr()->
                                                               GetSrcImage(*it);
        int width = src->getSize().width(), height = src->getSize().height();
        vigra::Rect2D crop_region = src->getCropRect();
        // pick a texture depending on crop mode and move it to the cropped area
        switch (src->getCropMode())
        {
            case HuginBase::SrcPanoImage::CROP_CIRCLE:
                glBindTexture(GL_TEXTURE_2D, circle_border_tex);
                // change the crop region to a square around the circle.
                if (crop_region.width() < crop_region.height())
                {
                    // too tall, move top and bottom.
                    int diff = (crop_region.width() - crop_region.height()) / 2;
                    // diff is negative, so we will shrink the border in the y
                    // direction.
                    crop_region.addBorder(0, diff);
                } else if (crop_region.width() > crop_region.height())
                {
                    // too wide, move left and right
                    int diff = (crop_region.height() - crop_region.width()) / 2;
                    crop_region.addBorder(diff, 0);
                }
                {
                    float diameter = (float) crop_region.width();
                    glScalef((float) width / diameter,
                             (float) height / diameter, 1.0);
                    glTranslatef(-(float) crop_region.left() / (float) width,
                                 -(float) crop_region.top() / (float) height,
                                 0.0);
                }
                break;
            case HuginBase::SrcPanoImage::CROP_RECTANGLE:
                // get the biggest rectangle contained by both the image 
                // and the cropped area.
                crop_region &= vigra::Rect2D(src->getSize());
                glBindTexture(GL_TEXTURE_2D, rectangle_border_tex);
                glScalef((float) width / (float) crop_region.width(),
                         (float) height / (float) crop_region.height(),
                         1.0);
                glTranslatef(-(float) crop_region.left() / (float) width,
                             -(float) crop_region.top() / (float) height,
                             0.0);
                break;
            case HuginBase::SrcPanoImage::NO_CROP:
                glBindTexture(GL_TEXTURE_2D, rectangle_border_tex);
                break;
        }
        // draw the image in this texture
        unsigned char r,g,b;
        HighlightColour(image_counter, num_images, r, g, b);
        image_counter++;
        glColor3ub(r,g,b);
        glCallList(helper->GetViewStatePtr()->GetMeshDisplayList(*it));
        glPopMatrix();
        // tell the preview frame to update the button to show the same colour.
        preview_frame->SetImageButtonColour(*it, r, g, b);
    }
    // set stuff back how we found it.
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_BLEND);
    glColor3ub(255, 255, 255);
}

bool PreviewIdentifyTool::BeforeDrawImageEvent(unsigned int image)
{
    // Delay drawing of images, so we can show them on top of the others.
    DEBUG_ASSERT(image < helper->GetPanoramaPtr()->getNrOfImages());
    if (image_set.count(image)) return false;
    return true;
}

void PreviewIdentifyTool::ShowImageNumber(unsigned int image)
{
    DEBUG_ASSERT(image < helper->GetPanoramaPtr()->getNrOfImages());
    // Add this image to the set of images drawn.
    if (!image_set.count(image))
    {
        // it is not already in the set. Add it now
        image_set.insert(image);
        // we want notification of when it is drawn so we can delay drawing.
        helper->NotifyMeBeforeDrawing(image, this);
        //  now we want a redraw.
        helper->GetViewStatePtr()->ForceRequireRedraw();
        helper->GetViewStatePtr()->Redraw();
    }
    mouse_over_image = image;
    mouse_is_over_button = true;
}

void PreviewIdentifyTool::StopShowingImages()
{
    if (mouse_is_over_button)
    {
        // set the colour to the image the user just moused off to the default.
        preview_frame->SetImageButtonColour(mouse_over_image, 0, 0, 0);
        helper->DoNotNotifyMeBeforeDrawing(mouse_over_image, this);
        image_set.erase(mouse_over_image);
        // now redraw without the indicator.
        helper->GetViewStatePtr()->ForceRequireRedraw();
        helper->GetViewStatePtr()->Redraw();
        mouse_is_over_button = false;
    }    
}

// generate a colour given how many colours we need and an index.
void PreviewIdentifyTool::HighlightColour(unsigned int index,
                                          unsigned int count,
                                          unsigned char &red,
                                          unsigned char &green,
                                          unsigned char &blue)
{
    DEBUG_ASSERT(index < count && index >= 0);
    // the first one is red, the rest are evenly spaced throughout the spectrum 
    float hue = ((float) index / (float) count) * 6.0;
    if (hue < 1.0)
    {
        // red to yellow
        red = 255;
        green = (unsigned char) (hue * 255.0);
        blue = 0;
    } else if (hue < 2.0) {
        // yellow to green
        red = (unsigned char) ((-hue + 2.0) * 255.0);
        green = 255;
        blue = 0;
    } else if (hue < 3.0) {
        // green to cyan
        red = 0;
        green = 255;
        blue = (unsigned char) ((hue - 2.0) * 255.0);
    } else if (hue < 4.0) {
        // cyan to blue
        red = 0;
        green = (unsigned char) ((-hue + 4.0) * 255.0);
        blue = 255;
    } else if (hue < 5.0) {
        // blue to magenta
        red = (unsigned char) ((hue - 4.0) * 255.0);
        green = 0;
        blue = 255;
    } else {
        // magenta to red
        red = 255;
        green = 0;
        blue = (unsigned char) ((-hue + 6.0) * 255.0);
    }
}

void PreviewIdentifyTool::MouseButtonEvent(wxMouseEvent & e)
{
    if (   e.GetButton() == wxMOUSE_BTN_LEFT
        && image_set.size() == 2)
    {
        // when there are only two images with indicators shown, show the
        // control point editor with those images when left clicked.
        MainFrame::Get()->ShowCtrlPointEditor(*(image_set.begin()),
                                              *(++image_set.begin()));
        MainFrame::Get()->Raise();
    }
}
