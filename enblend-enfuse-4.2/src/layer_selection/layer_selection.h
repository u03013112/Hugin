/*
 * Copyright (C) 2010-2016 Dr. Christoph L. Spiel
 *
 * This file is part of Enblend.
 *
 * Enblend is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Enblend is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Enblend; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef LAYER_SELECTION_H_
#define LAYER_SELECTION_H_


#include <map>
#include <memory>
#include <string>
#include <vector>

#include <vigra/imageinfo.hxx>

#include "info.h"


namespace selector
{
    typedef std::vector<unsigned> layer_ordered_list_t;
    struct Abstract; // forward declaration for class in "selector.h"
}


class LayerSelectionHost
{
    typedef std::vector<unsigned> layer_tally_t;
    typedef std::map<std::string, layer_tally_t> file_tally_t;

public:
    LayerSelectionHost();
    LayerSelectionHost(const LayerSelectionHost& a_selection);
    LayerSelectionHost& operator=(const LayerSelectionHost& a_selection);
    virtual ~LayerSelectionHost();

    std::string name() const;
    std::string description() const;

    selector::Abstract* get_selector() const;
    void set_selector(selector::Abstract* a_selector);

    template <class const_iterator>
    void retrieve_image_information(const_iterator begin, const_iterator end)
    {
        delete info_;
        info_ = new ImageListInformation;

        delete tally_;
        tally_ = new file_tally_t;

        for (const_iterator image = begin; image != end; ++image)
        {
            ImageInfo image_info((*image)->filename());
            vigra::ImageImportInfo file_info((*image)->filename().c_str());

            for (int layer = 0; layer < file_info.numImages(); ++layer)
            {
                std::unique_ptr<vigra::ImageImportInfo> layer_info(new vigra::ImageImportInfo(file_info));
                layer_info->setImageIndex(layer);

                image_info.append(LayerInfo(layer_info->width(), layer_info->height(),
                                            layer_info->isColor(), layer_info->pixelType(),
                                            layer_info->getPosition(),
                                            layer_info->getXResolution(), layer_info->getYResolution()));
            }

            info_->append(image_info);
            tally_->insert(file_tally_t::value_type((*image)->filename(),
                                                    layer_tally_t(file_info.numImages())));
        }
    }

    virtual bool accept(const std::string& a_filename, unsigned a_layer_index);
    virtual selector::layer_ordered_list_t viable_layers(const std::string& a_filename);

    // query tally
    // std::vector<std::string> unused_files() const;
    // std::pair<std::string, unsigned> multiply_used_layers() const;

private:
    selector::Abstract* selector_;
    ImageListInformation* info_;

    file_tally_t* tally_;
};


#endif // LAYER_SELECTION_H_


// Local Variables:
// mode: c++
// End:
