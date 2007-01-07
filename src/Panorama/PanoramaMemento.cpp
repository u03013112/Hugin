// -*- c-basic-offset: 4 -*-

/** @file PanoramaMemento.cpp
 *
 *  @brief implementation of PanoramaMemento Class
 *
 *  @author Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id$
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



#include <config.h>
#include <jhead/jhead.h>

#include <common/utils.h>
#include <common/stl_utils.h>
#include <common/math.h>

#include <PT/PanoramaMemento.h>
#include <PT/Panorama.h>
//#include <PT/SpaceTransform.h>


#include <vigra/impex.hxx>

#include <PT/PanoToolsInterface.h>

using namespace PT;
using namespace std;
using namespace utils;
using namespace vigra;

PanoramaMemento::~PanoramaMemento()
{

}

void PT::fillVariableMap(VariableMap & vars)
{

    vars.insert(pair<const char*, Variable>("y",Variable("y",0)));
    vars.insert(pair<const char*, Variable>("r",Variable("r",0)));
    vars.insert(pair<const char*, Variable>("p",Variable("p",0)));

    // Lens variables
    vars.insert(pair<const char*, Variable>("v",Variable("v",51)));
    vars.insert(pair<const char*, Variable>("a",Variable("a",0.0)));
    vars.insert(pair<const char*, Variable>("b",Variable("b",0.0)));
    vars.insert(pair<const char*, Variable>("c",Variable("c",0.0)));
    vars.insert(pair<const char*, Variable>("d",Variable("d",0)));
    vars.insert(pair<const char*, Variable>("e",Variable("e",0)));
    vars.insert(pair<const char*, Variable>("g",Variable("g",0)));
    vars.insert(pair<const char*, Variable>("t",Variable("t",0)));

    vars.insert(pair<const char*, Variable>("Va",Variable("Va",0)));
    vars.insert(pair<const char*, Variable>("Vb",Variable("Vb",0)));
    vars.insert(pair<const char*, Variable>("Vc",Variable("Vc",0)));
    vars.insert(pair<const char*, Variable>("Vd",Variable("Vd",0)));
    vars.insert(pair<const char*, Variable>("Vx",Variable("Vx",0)));
    vars.insert(pair<const char*, Variable>("Vy",Variable("Vy",0)));

    // color and brightness correction variables.
    vars.insert(pair<const char*, Variable>("K0a",Variable("K0a",1.0)));
    vars.insert(pair<const char*, Variable>("K0b",Variable("K0b",0)));
    vars.insert(pair<const char*, Variable>("K1a",Variable("K1a",1.0)));
    vars.insert(pair<const char*, Variable>("K1b",Variable("K1b",0)));
    vars.insert(pair<const char*, Variable>("K2a",Variable("K2a",1.0)));
    vars.insert(pair<const char*, Variable>("K2b",Variable("K2b",0)));

};

void PT::fillLensVarMap(LensVarMap & variables)
{
    variables.insert(pair<const char*, LensVariable>("v",LensVariable("v", 51, true)));
    variables.insert(pair<const char*, LensVariable>("a",LensVariable("a", 0.0, true )));
    variables.insert(pair<const char*, LensVariable>("b",LensVariable("b", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("c",LensVariable("c", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("d",LensVariable("d", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("e",LensVariable("e", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("g",LensVariable("g", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("t",LensVariable("t", 0.0, true)));

    // vignetting correction variables
    variables.insert(pair<const char*, LensVariable>("Va",LensVariable("Va", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("Vb",LensVariable("Vb", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("Vc",LensVariable("Vc", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("Vd",LensVariable("Vd", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("Vx",LensVariable("Vx", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("Vy",LensVariable("Vy", 0.0, true)));

    // color and brightness correction variables.
    variables.insert(pair<const char*, LensVariable>("K0a",LensVariable("K0a", 1.0, false)));
    variables.insert(pair<const char*, LensVariable>("K0b",LensVariable("K0b", 0.0, false)));
    variables.insert(pair<const char*, LensVariable>("K1a",LensVariable("K1a", 1.0, false)));
    variables.insert(pair<const char*, LensVariable>("K1b",LensVariable("K1b", 0.0, false)));
    variables.insert(pair<const char*, LensVariable>("K2a",LensVariable("K2a", 1.0, false)));
    variables.insert(pair<const char*, LensVariable>("K2b",LensVariable("K2b", 0.0, false)));

}

void PT::printVariableMap(ostream & o, const VariableMap & vars)
{
    for ( VariableMap::const_iterator it = vars.begin(); it != vars.end();++it)
    {
        it->second.print(o);
        o << " ";
    }
}

ostream & Variable::print(ostream & o) const
{
    return o << name << value;
}

std::ostream & LensVariable::printLink(std::ostream & o,
                                       unsigned int linkImage) const
{
    return o << name << "=" << linkImage;
}


//=========================================================================
//=========================================================================


Lens::Lens()
    : m_hasExif(false), m_projectionFormat(RECTILINEAR),
      m_imageSize(0,0), m_sensorSize(36.0,24.0)
{
    fillLensVarMap(variables);
}

char *PT::Lens::variableNames[] = { "v", "a", "b", "c", "d", "e", "g", "t",
                                    "Va", "Vb", "Vc", "Vd", "Vx", "Vy",
                                    "K0a", "K0b", "K1a", "K1b", "K2a", "K2b", 0};



void Lens::update(const Lens & l)
{
    m_projectionFormat = l.m_projectionFormat;
    m_sensorSize = l.m_sensorSize;
    m_imageSize = l.m_imageSize;
    variables = l.variables;
}

double Lens::getHFOV() const
{
    return const_map_get(this->variables,"v").getValue();
}

void Lens::setHFOV(double d)
{
    map_get(variables,"v").setValue(d);
}

double Lens::getFocalLength() const
{

    double HFOV = const_map_get(variables,"v").getValue();
#if 0
    if (isLandscape()) {
        ssize = m_sensorSize;
    } else {
        ssize.y = m_sensorSize.x;
        ssize.x = m_sensorSize.y;
    }
#endif

    switch (m_projectionFormat)
    {
        case RECTILINEAR:
            return (m_sensorSize.x/2.0) / tan(HFOV/180.0*M_PI/2);
            break;
        case CIRCULAR_FISHEYE:
        case FULL_FRAME_FISHEYE:
            // same projection equation for both fisheye types,
            // assume equal area projection.
            return m_sensorSize.x / (HFOV/180*M_PI);
            break;
        default:
            // TODO: add formulas for other projections
            DEBUG_WARN("Focal length calculations only supported with rectilinear and fisheye images");
            return 0;
    }
}

void Lens::setFocalLength(double fl)
{
#if 0
    if (isLandscape()) {
        ssize = m_sensorSize;
    } else {
        ssize.y = m_sensorSize.x;
        ssize.x = m_sensorSize.y;
    }
#endif

    double hfov=map_get(variables, "v").getValue();
    switch (m_projectionFormat) {
        case RECTILINEAR:
            hfov = 2*atan((m_sensorSize.x/2.0)/fl)  * 180.0/M_PI;
            break;
        case CIRCULAR_FISHEYE:
        case FULL_FRAME_FISHEYE:
            hfov = m_sensorSize.x / fl * 180/M_PI;
        default:
            // TODO: add formulas for other projections
            DEBUG_WARN("Focal length calculations only supported with rectilinear and fisheye images");
    }
    map_get(variables, "v").setValue(hfov);
}


void Lens::setCropFactor(double factor)
{
    // calculate diagonal on our sensor
    double d = sqrt(36.0*36.0 + 24.0*24.0) / factor;

    double r = (double)m_imageSize.x / m_imageSize.y;

    // calculate the sensor width and height that fit the ratio
    // the ratio is determined by the size of our image.
    m_sensorSize.x = d / sqrt(1 + 1/(r*r));
    m_sensorSize.y = m_sensorSize.x / r;
}

double Lens::getCropFactor() const
{
    double d2 = m_sensorSize.x*m_sensorSize.x + m_sensorSize.y*m_sensorSize.y;
    return sqrt(36.0*36+24*24) / sqrt(d2);
}

void Lens::setSensorSize(const FDiff2D & size) {
    m_sensorSize = size;
}


bool Lens::initFromFile(const std::string & filename, double &cropFactor, double & roll)
{
    std::string ext = utils::getExtension(filename);
    std::transform(ext.begin(), ext.end(), ext.begin(), (int(*)(int)) toupper);

    roll = 0;
    int width;
    int height;
    try {
        vigra::ImageImportInfo info(filename.c_str());
        width = info.width();
        height = info.height();
    } catch(vigra::PreconditionViolation & ) {
        return false;
    }
    setImageSize(vigra::Size2D(width, height));

    if (ext != "JPG" && ext != "JPEG") {
        return false;
    }

    ImageInfo_t exif;
    ResetJpgfile();
    // Start with an empty image information structure.

    memset(&exif, 0, sizeof(exif));
    exif.FlashUsed = -1;
    exif.MeteringMode = -1;

    if (!ReadJpegFile(exif,filename.c_str(), READ_EXIF)){
        DEBUG_DEBUG("Could not read jpg info");
        return false;
    }

#ifdef DEBUG
    ShowImageInfo(exif);
#endif

    DEBUG_DEBUG("exif dimensions: " << exif.ExifImageWidth << "x" << exif.ExifImageWidth);

    switch (exif.Orientation) {
        case 3:  // rotate 180
            roll = 180;
            break;
        case 6: // rotate 90
            roll = 90;
            break;
        case 8: // rotate 270
            roll = 270;
            break;
        default:
            break;
    }
    // image has been modified without adjusting exif tags
    // assume user has rotated to upright pose
    if (exif.ExifImageWidth && exif.ExifImageLength) {
        double ratioExif = exif.ExifImageWidth / (double)exif.ExifImageLength;
        double ratioImage = width/(double)height;
        if (abs( ratioExif - ratioImage) > 0.1) {
            roll = 0;
        }
    }

    // calc sensor dimensions if not set and 35mm focal length is available
    FDiff2D sensorSize;
    double focalLength = 0;


    if (exif.FocalLength > 0 && exif.CCDHeight > 0 && exif.CCDWidth > 0) {
        // read sensor size directly.
        sensorSize.x = exif.CCDWidth;
        sensorSize.y = exif.CCDHeight;
        if (strcmp(exif.CameraModel, "Canon EOS 20D") == 0) {
            // special case for buggy 20D camera
            sensorSize.x = 22.5;
            sensorSize.y = 15;
        }
		//
		// check if sensor size ratio and image size fit together
		double rsensor = (double)sensorSize.x / sensorSize.y;
		double rimg = (double) width / height;
		if ( (rsensor > 1 && rimg < 1) || (rsensor < 1 && rimg > 1) ) {
			// image and sensor ratio do not match
			// swap sensor sizes
			float t;
			t = sensorSize.y;
			sensorSize.y = sensorSize.x;
			sensorSize.x = t;
		}
        cropFactor = sqrt(36.0*36.0+24.0*24)/sqrt(sensorSize.x*sensorSize.x + sensorSize.y*sensorSize.y);
        focalLength = exif.FocalLength;
    } else if (exif.FocalLength35mm > 0 && exif.FocalLength > 0) {
        cropFactor = exif.FocalLength35mm / exif.FocalLength;
        focalLength = exif.FocalLength;
    } else if (exif.FocalLength35mm > 0 && cropFactor <= 0) {
        // do not ask for crop factor, even if we will store an invalid sensor size.
        // currenty the sensor size (just the ratio) is not used anywhere.
        cropFactor = 1;
        focalLength = exif.FocalLength35mm;
    } else if (exif.FocalLength > 0 || exif.FocalLength35mm > 0 ) {
        // no complete specification found.. ask the user for sensor/chip size, or crop factor
        if (cropFactor > 0) {
            // crop factor was provided by user
        } else {
            // need to redo, this time with crop
            cropFactor = -1;
            return false;
        }
        if (exif.FocalLength > 0 ) {
            focalLength = exif.FocalLength;
        } else if (exif.FocalLength35mm) {
            focalLength = exif.FocalLength35mm * cropFactor;
        }
    }

    if (sensorSize.x > 0) {
        setSensorSize(sensorSize);
    } else if (cropFactor > 0) {
        setCropFactor(cropFactor);
    } else {
        return false;
    }

    if (focalLength > 0) {
        setFocalLength(focalLength);
    } else {
        return false;
    }

    return true;
}

//=========================================================================
//=========================================================================

#if 0
ControlPoint::ControlPoint(Panorama & pano, const QDomNode & node)
{
    setFromXML(node,pano);
}
#endif


void ControlPoint::mirror()
{
    unsigned int ti;
    double td;
    ti =image1Nr; image1Nr = image2Nr, image2Nr = ti;
    td = x1; x1 = x2 ; x2 = td;
    td = y1; y1 = y2 ; y2 = td;
}

std::string ControlPoint::modeNames[] = { "x_y", "x", "y" };


const std::string & ControlPoint::getModeName(OptimizeMode mode) const
{
    return modeNames[mode];
}

//=========================================================================
//=========================================================================

const std::string & PanoramaOptions::getFormatName(FileFormat f)
{
    assert((int)f <= (int)HDR_m);
    return fileformatNames[(int) f];
}

const std::string & PanoramaOptions::getOutputExtension()
{
    assert((int)outputFormat <= (int)HDR_m);
    return fileformatExt[(int) outputFormat];
}

PanoramaOptions::FileFormat PanoramaOptions::getFormatFromName(const std::string & name)
{
    int max = (int) HDR_m;
    int i;
    for (i=0; i<max; i++) {
        if (name == fileformatNames[i]) {
            break;
        }
    }
    if (i == max) {
        DEBUG_ERROR("could not parse format " << name );
        return TIFF;
    }
    return (FileFormat) i;
}


void PanoramaOptions::printScriptLine(std::ostream & o) const
{
    o << "p f" << m_projectionFormat << " w" << getWidth()<< " h" << getHeight()
            << " v" << getHFOV() << " ";

    switch (colorCorrection) {
    case NONE:
        break;
    case BRIGHTNESS_COLOR:
        o << " k" << colorReferenceImage;
        break;
    case BRIGHTNESS:
        o << " b" << colorReferenceImage;
        break;
    case COLOR:
        o << " d" << colorReferenceImage;
        break;
    }

    o << " n\"" << getFormatName(outputFormat);
    if ( outputFormat == JPEG ) {
        o << " q" << quality;
    } else if ( outputFormat == TIFF ||
                outputFormat == TIFF_m ||
                outputFormat == TIFF_mask ||
                outputFormat == TIFF_multilayer ||
                outputFormat == TIFF_multilayer_mask)
    {
        o << " c:" << tiffCompression;
        if (tiff_saveROI) {
            o << " r:CROP";
        }
    }
    o << "\"";
    o << std::endl;

    // misc options
    o << "m g" << gamma << " i" << interpolator;
    switch (remapAcceleration) {
    case NO_SPEEDUP:
        break;
    case MAX_SPEEDUP:
        o << " f0";
        break;
    case MEDIUM_SPEEDUP:
        o << " f1";
    }
    o << " m" << huberSigma;
    
    o << std::endl;
}

bool PanoramaOptions::fovCalcSupported(ProjectionFormat f) const
{
    return ( f == RECTILINEAR
            || f == CYLINDRICAL
            || f == EQUIRECTANGULAR
            || f == MERCATOR
            || f == SINUSOIDAL );
}

void PanoramaOptions::setProjection(ProjectionFormat f)
{
    // only try to keep the FOV if calculations are implemented for
    // both projections
    if (fovCalcSupported(m_projectionFormat) && fovCalcSupported(f)) 
    {
        // keep old fov
        // correct image width and height
        PanoramaOptions copy(*this);
        copy.m_projectionFormat = f;
        double hfov = std::min(getHFOV(), copy.getMaxHFOV());
        double vfov = std::min(getVFOV(), copy.getMaxVFOV());
        setHFOV(hfov, false);
        setVFOV(vfov);
        m_projectionFormat = f;
        setHFOV(hfov, false);
        setVFOV(vfov);
    } else {
        m_projectionFormat = f;
        double hfov = std::min(getHFOV(), getMaxHFOV());
        setHFOV(hfov, false);
    }
}

void PanoramaOptions::setWidth(unsigned int w, bool keepView)
{
    if (m_projectionFormat == EQUIRECTANGULAR || m_projectionFormat == SINUSOIDAL) {
        if (w%2 == 1) {
            w = w+1;
        }
    }
    double scale = w / (double) m_size.x;
    if (keepView) {
        m_size.y = roundi(m_size.y*scale);
        if (fovCalcSupported(m_projectionFormat)) {
            if (getVFOV() > getMaxVFOV()) {
                setVFOV(getMaxVFOV());
	    }
        }
    }
    m_size.x = w;
    // reset roi
    m_roi=vigra::Rect2D(m_size);
    DEBUG_DEBUG(" HFOV: " << m_hfov << " size: " << m_size << " roi: " << m_roi << "  => vfov: " << getVFOV());
}

void PanoramaOptions::setHFOV(double h, bool keepView)
{
    if (keepView && !fovCalcSupported(m_projectionFormat)) {
        DEBUG_NOTICE("Ignoring keepView");
        keepView = false;
    }

    if (h <= 0) {
        h = 1;
    }
    double vfov;
    if (keepView) {
        vfov = getVFOV();
    }
    m_hfov = std::min(h, getMaxHFOV());
    if (keepView) {
        setVFOV(std::min(vfov, getMaxVFOV()));
    }
}

void PanoramaOptions::setHeight(unsigned int h) 
{
    if (h == 0) {
        h = 1;
    }
    m_size.y = h;
    // reset roi
    m_roi=vigra::Rect2D(m_size);

    DEBUG_DEBUG(" HFOV: " << m_hfov << " size: " << m_size << " roi:" << m_roi << "  => vfov: " << getVFOV());
}

void PanoramaOptions::setVFOV(double VFOV)
{
    if (! fovCalcSupported(m_projectionFormat)) {
        return;
    }

    if (VFOV <= 0) {
        VFOV = 1;
    }
    // TODO: create transform from equirect to target projection and
    // set additional
    PTools::Transform transf;
    SrcPanoImage src;
    src.setProjection(SrcPanoImage::EQUIRECTANGULAR);
    src.setHFOV(360);
    src.setSize(vigra::Size2D(360,180));
    transf.createInvTransform(src, *this);

    FDiff2D pmiddle;

    if (VFOV>180 && getMaxVFOV() > 180) {
        // we have crossed the pole
        transf.transform(pmiddle, FDiff2D(180, 180-VFOV/2 - 0.01));
    } else {
        transf.transform(pmiddle, FDiff2D(0, VFOV/2));
    }
    m_size.y = abs(roundi(2*pmiddle.y));

    // reset roi
    m_roi=vigra::Rect2D(m_size);
    DEBUG_DEBUG(" HFOV: " << m_hfov << " size: " << m_size << " roi: " << m_roi << "  => vfov: " << VFOV);

}

double PanoramaOptions::getVFOV() const
{
    // calcuale VFOV based on current panorama
    PTools::Transform transf;
    SrcPanoImage src;
    src.setProjection(SrcPanoImage::EQUIRECTANGULAR);
    src.setHFOV(360);
    src.setSize(vigra::Size2D(360,180));
    transf.createTransform(src, *this);

    FDiff2D pmiddle;
    FDiff2D pcorner;
    transf.transform(pmiddle, FDiff2D(0, m_size.y/2.0));
//    transf.transform(pcorner, FDiff2D(m_size.x/2.0, m_size.y/2.0));
    double VFOV;
    if (pmiddle.x > 90 ||pmiddle.y < -90) {
        // the pole has been crossed
        VFOV = 2*(180-pmiddle.y);
    } else {
        VFOV = 2*pmiddle.y;
    }
    //double VFOV = 2.0*std::max(pcorner.y, pmiddle.y);

    /*
    double VFOV;
    switch (m_projectionFormat) {
        case PanoramaOptions::RECTILINEAR:
            VFOV = 2.0 * atan( (double)m_height * tan(DEG_TO_RAD(m_hfov)/2.0) / m_width);
            VFOV = RAD_TO_DEG(VFOV);
            break;
        case PanoramaOptions::CYLINDRICAL:
        {
            // equations: w = f * v (f: focal length, in pixel)
            double f = m_width / DEG_TO_RAD(m_hfov);
            VFOV = 2*atan(m_height/(2.0*f));
            VFOV = RAD_TO_DEG(VFOV);
            break;
        }
        case PanoramaOptions::EQUIRECTANGULAR:
            // FIXME: This is wrong!
        case TRANSVERSE_MERCATOR:
        case MERCATOR:
            VFOV = m_hfov * m_height / m_width;
            break;
        case PanoramaOptions::FULL_FRAME_FISHEYE:
            VFOV = m_hfov * m_height / m_width;
            break;
    }
    */
    DEBUG_DEBUG(" HFOV: " << m_hfov << " size: " << m_size << " roi: " << m_roi << "  => vfov: " << VFOV);
    return VFOV;
}

double PanoramaOptions::getMaxHFOV() const
{
    switch (m_projectionFormat)
    {
        case RECTILINEAR:
        case TRANSVERSE_MERCATOR:
            return 175;
        case STEREOGRAPHIC:
            return 355;
        case CYLINDRICAL:
        case EQUIRECTANGULAR:
        case FULL_FRAME_FISHEYE:
        case MERCATOR:
        case SINUSOIDAL:
        case LAMBERT:
        case LAMBERT_AZIMUTHAL:
        default:
            return 360;
    }
}

double PanoramaOptions::getMaxVFOV() const
{
    switch (m_projectionFormat)
    {
        case RECTILINEAR:
        case CYLINDRICAL:
        case MERCATOR:
            return 175;
        default:
        case EQUIRECTANGULAR:
        case LAMBERT:
        case SINUSOIDAL:
            return 180;
        case STEREOGRAPHIC:
            return 355;
        case FULL_FRAME_FISHEYE:
        case TRANSVERSE_MERCATOR:
        case LAMBERT_AZIMUTHAL:
            return 360;
    }
}

DestPanoImage PanoramaOptions::getDestImage() const
{
    Size2D size(getWidth(), getHeight());
    return DestPanoImage((DestPanoImage::Projection) getProjection(),
                          getHFOV(), size );
}

const string PanoramaOptions::fileformatNames[] =
{
    "JPEG",
    "PNG",
    "TIFF",
    "TIFF_m",
    "TIFF_mask",
    "TIFF_multilayer",
    "TIFF_multilayer_mask",
    "PICT",
    "PSD",
    "PSD_m",
    "PSD_mask",
    "PAN",
    "IVR",
    "IVR_java",
    "VRML",
    "QTVR",
    "HDR",
    "HDR_m"
};


const string PanoramaOptions::fileformatExt[] =
{
    "jpg",
    "png",
    "tif",
    "tif",
    "tif",
    "tif",
    "tif",
    "pict",
    "psd",
    "psd",
    "psd",
    "pan",
    "ivr",
    "IVR_java",
    "wrl",
    "mov",
    "hdr",
    "hdr"
};

struct ImgInfo
{
public:
    ImgInfo()
    {
        init();
    }

    ImgInfo(const string & line)
    {
        init();
        this->parse(line);
    }

    void init()
    {
        blend_radius = 0;
        width = -1;
        height = -1;
        f = -2;
        vigcorrMode = 0;
        for (char ** v = varnames; *v != 0; v++) {
            vars[*v] = 0;
            links[*v] = -2;
        }
	autoCenterCrop = true;
	cropFactor = 1;
    }

    void parse(const string & line)
    {
        double * val = defaultValues;
        for (char ** v = varnames; *v; v++, val++) {
            vars[*v] = *val;
            links[*v] = -1;
            std::string name;
            name = *v;
            getPTDoubleParam(vars[*v], links[*v], line, name);
        }

        getIntParam(blend_radius, line, "u");

        // read lens type and hfov
        getIntParam(f, line, "f");

        getPTStringParam(filename,line,"n");
        getIntParam(width, line, "w");
        getIntParam(height, line, "h");

        getIntParam(vigcorrMode, line, "Vm");
        getPTStringParam(flatfieldname,line,"Vf");

        string crop_str;
        if ( getPTParam(crop_str, line, "C") ) {
            int left, right, top, bottom;
            int n = sscanf(crop_str.c_str(), "%d,%d,%d,%d", &left, &right, &top, &bottom);
            if (n == 4) {
                crop = vigra::Rect2D(left, top, right, bottom);
            } else {
                DEBUG_WARN("Could not parse crop string: " << crop_str);
            }
        }
        if ( getPTParam(crop_str, line, "S") ) {
            int left, right, top, bottom;
            int n = sscanf(crop_str.c_str(), "%d,%d,%d,%d", &left, &right, &top, &bottom);
            if (n == 4) {
                crop = vigra::Rect2D(left, top, right, bottom);
            } else {
                DEBUG_WARN("Could not parse crop string: " << crop_str);
            }
        }
    }

    static char *varnames[];
    static double defaultValues[];
    std::string filename;
    std::string flatfieldname;
    std::map<std::string, double> vars;
    std::map<std::string, int> links;
    int f;
    int blend_radius;
    int width, height;
    int vigcorrMode;
    vigra::Rect2D crop;
    bool autoCenterCrop;
    double cropFactor;
};

// cannot use Lens::variableNames here, because r,p,v need to be included
char * ImgInfo::varnames[] = {"v","a","b","c","d","e","g","t","r","p","y",
                              "Va", "Vb", "Vc", "Vd", "Vx", "Vy", 
                              "K0a", "K0b", "K1a", "K1b", "K2a", "K2b", 0};
double ImgInfo::defaultValues[] = {51.0,  0.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0,  0.0, 0.0, 0.0,
                                      1.0, 0.0, 0.0, 0.0,   0.0, 0.0, 
                                      1.0, 0.0,  1.0, 0.0,  1.0, 0.0};

bool PanoramaMemento::loadPTScript(std::istream &i, const std::string &prefix)
{
    DEBUG_TRACE("");
#ifdef __unix__
    // set numeric locale to C, for correct number output
    char * old_locale = setlocale(LC_NUMERIC,NULL);
    setlocale(LC_NUMERIC,"C");
#endif
    PTParseState state;
    string line;

    // vector with the different information lines about images
    vector<ImgInfo> oImgInfo;
    vector<ImgInfo> iImgInfo;
    // strange comment informations.
    vector<ImgInfo> cImgInfo;
    // hugin additional information
    vector<ImgInfo> huginImgInfo;

    // indicate lines that should be skipped for whatever reason
    bool skipNextLine = false;

    bool PTGUIScriptFile = false;
    int PTGUIScriptVersion = 0;
    // PTGui lens line detected
    int ctrlPointsImgNrOffset = 0;
    bool PTGUILensLine = false;

    bool PTGUILensLoaded = false;
    ImgInfo PTGUILens;

    bool firstOptVecParse = true;
    unsigned int lineNr = 0;
    while (i.good()) {
        std::getline(i, line);
        lineNr++;
        DEBUG_DEBUG(lineNr << ": " << line);
        if (skipNextLine) {
            skipNextLine = false;
            continue;
        }
        // check for a known line
        switch(line[0]) {
        case 'p':
        {
            DEBUG_DEBUG("p line: " << line);
            string format;
            int i;
            getIntParam(i,line,"f");
            options.setProjection( (PanoramaOptions::ProjectionFormat) i );
            unsigned int w;
            getIntParam(w, line, "w");
            options.setWidth(w);
            double v;
            getDoubleParam(v, line, "v");
            options.setHFOV(v);
            int height;
            getIntParam(height, line, "h");
            options.setHeight(height);

            // this is fragile.. hope nobody adds additional whitespace
            // and other arguments than q...
            // n"JPEG q80"
            getPTStringParam(format,line,"n");
            int t = format.find(' ');
 
            options.outputFormat = options.getFormatFromName(format.substr(0,t));

            // parse output format options.
            switch (options.outputFormat)
            {
            case PanoramaOptions::JPEG:
                {
                    // "parse" jpg quality
                    int q;
                    if (getIntParam(q, format, "q") ) {
                        options.quality = (int) q;
                    }
                }
                break;
            case PanoramaOptions::TIFF_m:
                {
                    int coordImgs = 0;
                    getIntParam(coordImgs, format, "p");
                    if (coordImgs)
                        options.saveCoordImgs = true;
                }
            case PanoramaOptions::TIFF:
            case PanoramaOptions::TIFF_mask:
            case PanoramaOptions::TIFF_multilayer:
            case PanoramaOptions::TIFF_multilayer_mask:
                {
                    // parse tiff compression mode
                    std::string comp;
                    if (getPTStringParamColon(comp, format, "c")) {
                        if (comp == "NONE" || comp == "LZW" ||
                            comp == "DEFLATE") 
                        {
                            options.tiffCompression = comp;
                        } else {
                            DEBUG_WARN("No valid tiff compression found");
                        }
                    }
                    // read tiff roi
                    if (getPTStringParamColon(comp, format, "r")) {
                        if (comp == "CROP") {
                            options.tiff_saveROI = true;
                        } else {
                            options.tiff_saveROI = false;
                        }
                    }
                }
                break;
            default:
                break;
            }

            int cRefImg = 0;
            if (getIntParam(cRefImg, line,"k")) {
                options.colorCorrection = PanoramaOptions::BRIGHTNESS_COLOR;
            } else if (getIntParam(cRefImg, line,"b")) {
                options.colorCorrection = PanoramaOptions::BRIGHTNESS;
            } else if (getIntParam(cRefImg, line,"d")) {
                options.colorCorrection = PanoramaOptions::COLOR;
            } else {
                options.colorCorrection = PanoramaOptions::NONE;
            }
            options.colorReferenceImage=cRefImg;
            break;

        }
        case 'm':
        {
            DEBUG_DEBUG("m line: " << line);
            // parse misc options
            int i;
            getIntParam(i,line,"i");
            options.interpolator = (vigra_ext::Interpolator) i;
            getDoubleParam(options.gamma,line,"g");

            if (getIntParam(i,line,"f")) {
                switch(i) {
                case 0:
                    options.remapAcceleration = PanoramaOptions::MAX_SPEEDUP;
                    break;
                case 1:
                    options.remapAcceleration = PanoramaOptions::MEDIUM_SPEEDUP;
                    break;
                default:
                    options.remapAcceleration = PanoramaOptions::NO_SPEEDUP;
                    break;
                }
            } else {
                options.remapAcceleration = PanoramaOptions::NO_SPEEDUP;
            }

            break;
        }
        case 'v':
        {
            DEBUG_DEBUG("v line: " << line);
            if (!PTGUIScriptFile) {
                if (firstOptVecParse) {
                    int nImg = max(iImgInfo.size(), oImgInfo.size());
                    DEBUG_DEBUG("nImg: " << nImg);
                    optvec = OptimizeVector(nImg);
                    firstOptVecParse = false;
                }
                std::stringstream optstream;
                optstream << line.substr(1);
                string var;
                while (!(optstream >> std::ws).eof()) {
                    optstream >> var;
                    if (var.length() == 1) {
                        // special case for PTGUI
                        var += "0";
                    }
                    unsigned int nr = utils::lexical_cast<unsigned int>(var.substr(1));
                    DEBUG_ASSERT(nr < optvec.size());
                    optvec[nr].insert(var.substr(0,1));
                    DEBUG_DEBUG("parsing opt: >" << var << "< : var:" << var[0] << " image:" << nr);
                }
            }
            break;
        }
        case 'c':
        {
            DEBUG_DEBUG("c line: " << line);
            int t;
            // read control points
            ControlPoint point;
            getIntParam(point.image1Nr, line, "n");
            point.image1Nr += ctrlPointsImgNrOffset;
            getIntParam(point.image2Nr, line, "N");
            point.image2Nr += ctrlPointsImgNrOffset;
            getDoubleParam(point.x1, line, "x");
            getDoubleParam(point.x2, line, "X");
            getDoubleParam(point.y1, line, "y");
            getDoubleParam(point.y2, line, "Y");
            if (!getIntParam(t, line, "t") ){
                t = 0;
            }

            point.mode = (ControlPoint::OptimizeMode) t;
            ctrlPoints.push_back(point);
            state = P_CP;
            break;
        }

        // handle the complicated part.. the image & lens settings.
        // treat i and o lines the same.. however, o lines have priority
        // over i lines.(i lines often do not contain link information!)
        case 'i':
        {
            if (PTGUILensLine) {
                PTGUILensLine = false;
                PTGUILensLoaded = true;
                PTGUILens.parse(line);
            } else {
                iImgInfo.push_back(ImgInfo(line));
            }
            break;
        }
        case 'o':
        {
            if (PTGUILensLine) {
                PTGUILensLine = false;
                PTGUILensLoaded = true;
                PTGUILens.parse(line);
            } else {
                oImgInfo.push_back(ImgInfo(line));
            }
            break;
        }

        case '#':
        {
            // parse special comments...
            if (line.substr(0,20) == string("# ptGui project file")) {
                PTGUIScriptFile = true;
            }

            if (PTGUIScriptFile) {
                // parse special PTGUI stuff.
                if (sscanf(line.c_str(), "#-fileversion %d", &PTGUIScriptVersion) > 0) {
                    DEBUG_DEBUG("Detected PTGUI script version: " << PTGUIScriptVersion);
                    switch (PTGUIScriptVersion) {
                        case 0:
                            break;
                        case 1:
                            break;
                        case 2:
                            break;
                        case 3:
                            break;
                        case 4:
                            break;
                        case 5:
                            break;
                        case 6:
                            break;
                        case 7:
                            break;
                        default:
                            ctrlPointsImgNrOffset = -1;
                            // latest known version is 8
                            break;
                    }
                }
                if (line.substr(0,12) == "#-dummyimage") {
                    PTGUILensLine = true;
                }
            }

            if (line.substr(0,8) == "#-hugin ") {
		// read hugin image line
                ImgInfo info;
                info.autoCenterCrop = (line.find("autoCenterCrop=1") != std::string::npos);
                size_t pos = line.find("cropFactor=");
                if (pos > 0 && pos < line.length()) {
                    double cropFactor=1;
                    const char * s = line.c_str() + pos;
                    sscanf(s,"cropFactor=%lf", & cropFactor);
                    std::cerr << "crop factor: " << s << " parsed: " << cropFactor << std::endl;
                    info.cropFactor = cropFactor;
                }
                huginImgInfo.push_back(info);
	    }
	    
            // PTGui and PTAssember project files:
            // #-imgfile 960 1280 "D:\data\bruno\074-098\087.jpg"
            if (line.substr(0,10) == "#-imgfile ") {

                // arghhh. I like string processing without regexps.
                int b = line.find_first_not_of(" ",9);
                int e = line.find_first_of(" ",b);
                DEBUG_DEBUG(" width:" << line.substr(b,e-b)<<":")
                int nextWidth = utils::lexical_cast<int,string>(line.substr(b,e-b));
                DEBUG_DEBUG("next width " << nextWidth);
                b = line.find_first_not_of(" ",e);
                e = line.find_first_of(" ",b);
                DEBUG_DEBUG(" height:" << line.substr(b,e-b)<<":")
                int nextHeight = utils::lexical_cast<int, string>(line.substr(b,e-b));
                DEBUG_DEBUG("next height " << nextHeight);
                b = line.find_first_not_of(" \"",e);
                e = line.find_first_of("\"",b);
                string nextFilename = line.substr(b,e-b);
                DEBUG_DEBUG("next filename " << nextFilename);

                ImgInfo info;
                info.width  = nextWidth;
                info.height = nextHeight;
                info.filename = nextFilename;
                cImgInfo.push_back(info);
            }


            // parse our special options
            if (line.substr(0,14) == "#hugin_options") {
                DEBUG_DEBUG("parsing special line");
                getIntParam(options.optimizeReferenceImage, line, "r");
				int val;
				if (getIntParam(val,line,"e")) {
					switch(val) {
					case 0:
						options.blendMode = PanoramaOptions::NO_BLEND;
						break;
                    case 1:
                        options.blendMode = PanoramaOptions::PTBLENDER_BLEND;
                        break;
					case 2:
						options.blendMode = PanoramaOptions::ENBLEND_BLEND;
						break;
					case 3:
						options.blendMode = PanoramaOptions::SMARTBLEND_BLEND;
						break;
					default:
						options.blendMode = PanoramaOptions::ENBLEND_BLEND;
						break;
					}
				} else {
                    options.blendMode = PanoramaOptions::ENBLEND_BLEND;
				}
            }
            break;
        }

        } // case
    }

    // assemble images & lenses from the information read before..

    // handle PTGUI special case
    if (PTGUILensLoaded) {
        // create lens with dummy info
        Lens l;
        for (char **v = Lens::variableNames; *v != 0; v++) {
            map_get(l.variables, *v).setValue(PTGUILens.vars[*v]);
        }
        l.setImageSize(vigra::Size2D(PTGUILens.width, PTGUILens.height));
        l.setCropFactor(1);
        l.setProjection((Lens::LensProjectionFormat) PTGUILens.f);
        lenses.push_back(l);
    }

/*
    // ugly hack to load PTGui script files
    if (ptGUIDummyImage) {
        DEBUG_DEBUG("loading default PTGUI line: " << line);
            Lens l;
            // skip ptgui's dummy image
            // load parameters into default lens...
            for (LensVarMap::iterator it = l.variables.begin();
             it != l.variables.end();
             ++it)
            {
                DEBUG_DEBUG("reading default lens variable " << it->first);
                int link;
                bool ok = readVar(it->second, link, line);
                DEBUG_ASSERT(ok);
                DEBUG_ASSERT(link == -1);
            }
            lenses.push_back(l);

            ptGUIDummyImage = false;
            break;
        }
*/

    // merge image info from the 3 different lines...
    // i lines are the main reference.

    int nImgs = iImgInfo.size();
    int nOLines = oImgInfo.size();
    int nCLines = cImgInfo.size();

    if (nImgs < nOLines) {
        // no, or less i lines found. scrap i lines.
        DEBUG_DEBUG("throwing away " << nImgs << " i lines");
        iImgInfo = oImgInfo;
        nImgs = nOLines;
    }
    if (nOLines < nImgs) {
        oImgInfo = iImgInfo;
    }

    // merge o lines and i lines into i lines.
    for (int i=0; i < nImgs; i++) {

        // move parameters from o lines -> i (only if it isn't given in the
        // i lines. or it is linked on the o lines)

        // ordinary variables
        for (char ** v = ImgInfo::varnames; *v ; v++) {

            if (iImgInfo[i].links[*v] == -2 && oImgInfo[i].links[*v] != -2 || iImgInfo[i].links[*v] == -1 && oImgInfo[i].links[*v] >=0) {
                DEBUG_DEBUG(*v << ": o -> i");
                iImgInfo[i].vars[*v] = oImgInfo[i].vars[*v];
                iImgInfo[i].links[*v] = oImgInfo[i].links[*v];
            }
        }

        if (iImgInfo[i].filename == "" && oImgInfo[i].filename != "") {
            DEBUG_DEBUG("filename: o -> i");
            iImgInfo[i].filename = oImgInfo[i].filename;
        }

        if (iImgInfo[i].crop.isEmpty() && !oImgInfo[i].crop.isEmpty()) {
            DEBUG_DEBUG("crop: o -> i");
            iImgInfo[i].crop = oImgInfo[i].crop;
        }

        if (iImgInfo[i].width <= 0 && oImgInfo[i].width > 0) {
            DEBUG_DEBUG("width: o -> i");
            iImgInfo[i].width = oImgInfo[i].width;
        }

        if (iImgInfo[i].height <= 0 && oImgInfo[i].height > 0) {
            DEBUG_DEBUG("height: o -> i");
            iImgInfo[i].height = oImgInfo[i].height;
        }

        if (iImgInfo[i].f < 0 && oImgInfo[i].f > 0) {
            DEBUG_DEBUG("f: o -> i");
            iImgInfo[i].f = oImgInfo[i].f;
        }

        if (nCLines == nImgs) {
            // img file & size in clines
            if (cImgInfo[i].filename != "" && cImgInfo[i].width > 0) {
                DEBUG_DEBUG("filename, width, height: c -> i");
                iImgInfo[i].filename = cImgInfo[i].filename;
                iImgInfo[i].width = cImgInfo[i].width;
                iImgInfo[i].height = cImgInfo[i].height;
            }
        }
        if (huginImgInfo.size() == nImgs) {
            iImgInfo[i].cropFactor = huginImgInfo[i].cropFactor;
            iImgInfo[i].autoCenterCrop = huginImgInfo[i].autoCenterCrop;
        }
    }

    // create image and lens.
    for (int i=0; i < nImgs; i++) {

        DEBUG_DEBUG("i line: " << i);
        // read the variables & decide if to create a new lens or not
        VariableMap vars;
        int link = -2;
        fillVariableMap(vars);

        for (char ** v = ImgInfo::varnames; *v != 0; v++) {
            std::string name(*v);
            double val = iImgInfo[i].vars[*v];
            map_get(vars,name).setValue(val);
            DEBUG_ASSERT(link <0  || iImgInfo[i].links[*v] < 0|| link == iImgInfo[i].links[*v]);
            if (iImgInfo[i].links[*v] >= 0) {
                link = iImgInfo[i].links[*v];
            }
        }

        int width = iImgInfo[i].width;
        int height = iImgInfo[i].height;

        string file = iImgInfo[i].filename;
        // add prefix if only a relative path.
#ifdef WIN32
        bool absPath = (file[1]==':' && file[2]=='\\');
#else
        bool absPath = file[0] == '/';
#endif
        if (!absPath) {
            file.insert(0, prefix);
        }
        DEBUG_DEBUG("filename: " << file);

        Lens l;

        l.setImageSize(vigra::Size2D(iImgInfo[i].width, iImgInfo[i].height));
        l.setCropFactor(iImgInfo[i].cropFactor);

        int anchorImage = -1;
        int lensNr = -1;
        for (LensVarMap::iterator it = l.variables.begin();
            it != l.variables.end();
            ++it)
        {
            std::string varname = it->first;
            // default to unlinked variables, overwrite later, if found in script
            (*it).second.setLinked(false);

            DEBUG_DEBUG("reading variable " << varname << " link:" << link );
            if (link >=0 && iImgInfo[i].links[varname]>= 0) {
                // linked variable

                if (PTGUILensLoaded && link == 0) {
                    anchorImage = link;
                    // set value from lens variable
                    lensNr = 0;
                } else if ((int) images.size() <= link && (!PTGUILensLoaded)) {
                    DEBUG_ERROR("variables must be linked to an image with a lower number" << endl
                                << "number links: " << link << " images: " << images.size() << endl
                                << "error on line " << lineNr << ":" << endl
                                << line);
#ifdef __unix__
                    // reset locale
                    setlocale(LC_NUMERIC,old_locale);
#endif
                    return false;
                } else {
                    DEBUG_DEBUG("anchored to image " << link);
                    anchorImage = link;
                    // existing lens
                    lensNr = images[anchorImage].getLensNr();
                    DEBUG_DEBUG("using lens nr " << lensNr);
                }
                DEBUG_ASSERT(lensNr >= 0);
                // get variable value of the link target
                double val = map_get(lenses[lensNr].variables, varname).getValue();
                map_get(vars, varname).setValue(val);
                map_get(lenses[lensNr].variables, varname).setLinked(true);
                it->second.setValue(val);
            } else {
                DEBUG_DEBUG("image " << i << " not linked, link: " << link);
                // not linked
                // copy value to lens variable.
                double val = map_get(vars,varname).getValue();
                it->second.setValue(val);
            }
        }
        variables.push_back(vars);

        DEBUG_DEBUG("lensNr after scanning " << lensNr);
        //l.projectionFormat = (Lens::LensProjectionFormat) iImgInfo[i].f;
        l.setProjection((Lens::LensProjectionFormat) iImgInfo[i].f);

        if (lensNr != -1) {
    //                lensNr = images[anchorImage].getLensNr();
            if (l.getProjection() != lenses[lensNr].getProjection()) {
                DEBUG_ERROR("cannot link images with different projections");
    #ifdef __unix__
                // reset locale
                setlocale(LC_NUMERIC,old_locale);
    #endif
                return false;
            }
        }

        if (lensNr == -1) {
            // no links -> create a new lens
            // create a new lens.
            lenses.push_back(l);
            lensNr = lenses.size()-1;
        } else {
            // check if the lens uses landscape as well..
            if (lenses[(unsigned int) lensNr].isLandscape() != l.isLandscape()) {
                DEBUG_ERROR("Landscape and portrait images can't share a lens" << endl
                            << "error on script line " << lineNr << ":" << line);
            }
            // check if the ratio is equal
        }

        DEBUG_ASSERT(lensNr >= 0);
        DEBUG_DEBUG("adding image with lens " << lensNr);
        images.push_back(PanoImage(file,width, height, (unsigned int) lensNr));

        ImageOptions opts = images.back().getOptions();
        opts.featherWidth = (unsigned int) iImgInfo[i].blend_radius;
        if (!iImgInfo[i].crop.isEmpty()) {
            opts.docrop = true;
            opts.cropRect = iImgInfo[i].crop;
        }
        opts.m_vigCorrMode = iImgInfo[i].vigcorrMode;
        opts.m_flatfield = iImgInfo[i].flatfieldname;
        opts.autoCenterCrop = iImgInfo[i].autoCenterCrop;
        images.back().setOptions(opts);
    }

    // if we haven't found a v line in the project file
    if (optvec.size() != images.size()) {
        optvec = OptimizeVector(images.size());
    }
    return true;
}
